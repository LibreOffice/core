FROM gitpod/workspace-full

RUN sudo sh -c "echo deb-src http://archive.ubuntu.com/ubuntu/ disco main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://archive.ubuntu.com/ubuntu/ disco-updates main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ disco-security main restricted >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ disco-security universe >> /etc/apt/sources.list" \
 && sudo sh -c "echo deb-src http://security.ubuntu.com/ubuntu/ disco-security multiverse >> /etc/apt/sources.list" \
 && sudo apt-get update \
 && sudo apt-get install -y \
    build-essential git libkrb5-dev graphviz nasm \
 && sudo apt-get build-dep -y libreoffice \
 && sudo rm -rf /var/lib/apt/lists/*

