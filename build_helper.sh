#!/bin/bash
cd /cygdrive/c/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork
export PATH="/usr/bin:/cygdrive/c/Program Files/Git/bin:/cygdrive/c/nasm:$PATH"
echo "Starting autogen..."
./autogen.sh > build.log 2>&1
if [ $? -ne 0 ]; then
    echo "Autogen failed" >> build.log
    exit 1
fi
echo "Starting make..."
make >> build.log 2>&1
