FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    build-essential \
    g++ \
    python3 \
    python3-pip \
    python3-dev \
    python3-setuptools \
    openjdk-17-jdk \
    autoconf \
    automake \
    libtool \
    pkg-config \
    libnss3-dev \
    libnspr4-dev \
    libxml2-utils \
    gperf \
    xsltproc \
    uuid-runtime \
    bison \
    flex \
    zip \
    libxrender-dev \
    libxt-dev \
    libcups2-dev \
    libxrandr-dev \
    libglu1-mesa-dev \
    libcairo2-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    libgif-dev \
    libexpat1-dev \
    libssl-dev \
    libkrb5-dev \
    libldap2-dev \
    libdbus-1-dev \
    libharfbuzz-dev \
    libcurl4-openssl-dev \
    libxslt1-dev \
    ninja-build \
    meson \
    wget \
    curl
RUN pip3 install --no-cache-dir setuptools lxml meson ninja --break-system-packages



#RUN dpkg-query -W -f='${Package}\t${Version}\n' \
#    git build-essential g++ python3 python3-pip python3-dev python3-setuptools openjdk-17-jdk autoconf automake libtool pkg-config libnss3-dev libnspr4-dev libxml2-utils gperf xsltproc uuid-runtime bison flex zip libxrender-dev libxt-dev libcups2-dev libxrandr-dev libglu1-mesa-dev libcairo2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libpng-dev libjpeg-dev libtiff-dev libgif-dev libexpat1-dev libssl-dev libkrb5-dev libldap2-dev libdbus-1-dev libharfbuzz-dev libcurl4-openssl-dev libxslt1-dev ninja-build meson wget curl 2>/dev/null | column -t

COPY ./libreoffice-core /APP/libreoffice-core
WORKDIR /APP/libreoffice-core
RUN ./autogen.sh --disable-gtk3 --disable-gui --disable-dbus --disable-cairo-canvas --without-help --without-java --without-myspell-dicts --without-doxygen --without-junit --enable-release-build --disable-debug --prefix=/tmp/build/
RUN make -j8
RUN make install
RUN ls -laht /tmp/build/
RUN ls -laht /APP
ENTRYPOINT [ "mv", "/tmp/build/", "/build/" ]
#ENTRYPOINT [ "tail", "-f", "/dev/null" ]