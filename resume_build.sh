#!/bin/bash
cd /cygdrive/c/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork
export PATH="/usr/bin:/cygdrive/c/Program Files/Git/bin:/cygdrive/c/nasm:$PATH"
echo "Restarting make..."
make >> build.log 2>&1
