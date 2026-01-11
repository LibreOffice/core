#!/bin/bash
export PATH="/cygdrive/c/Program Files/LLVM/bin:/cygdrive/c/Program Files/Git/mingw64/bin:/cygdrive/c/Program Files/Git/bin:/usr/bin:/bin:$PATH"
cd /cygdrive/c/Users/philh/dev/officelabs-suite/officelabs-master/libreoffice-fork
exec make "$@"
