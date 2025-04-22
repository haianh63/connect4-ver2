FROM python:3.10-slim

WORKDIR /app

# Cài đặt các công cụ cần thiết để biên dịch file C
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Cấu hình môi trường
ENV PYTHONUNBUFFERED=1

# Copy file requirements.txt và cài đặt dependencies
COPY requirements.txt .
RUN pip install --upgrade pip
RUN pip install -r requirements.txt

# Copy toàn bộ mã nguồn
COPY . .

# Build file connectfour.so từ file connectfour.c
RUN g++ -shared -fPIC -o connectfour.so connectfour.cpp

# Mở cổng do Render cung cấp
EXPOSE $PORT

# Khởi chạy ứng dụng bằng uvicorn
CMD uvicorn app:app --host 0.0.0.0 --port $PORT