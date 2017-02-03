#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# enable file locking
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

cd_or_exit() {
    if ! cd "$1"; then
        echo "Can't cd to $1"
        exit 1
    fi
}

# resolve installation directory
sd_cwd=$(pwd)
sd_res="$0"
while [ -h "$sd_res" ] ; do
    sd_dirname=$(dirname "$sd_res")
    cd_or_exit "$sd_dirname"
    sd_basename=$(basename "$sd_res")
    sd_res=$(ls -l "$sd_basename" | sed "s/.*$sd_basename -> //g")
done
sd_dirname=$(dirname "$sd_res")
cd_or_exit "$sd_dirname"
sd_prog=$(pwd)
cd_or_exit "$sd_cwd"

# this is a temporary hack until we can live with the default search paths
case "$(uname -s)" in
NetBSD|OpenBSD|FreeBSD|DragonFly)
    LD_LIBRARY_PATH="$sd_prog${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    JAVA_HOME=$(javaPathHelper -h libreoffice-java 2> /dev/null)
    export LD_LIBRARY_PATH
    if [ -n "${JAVA_HOME}" ]; then
        export JAVA_HOME
    fi
    ;;
AIX)
    LIBPATH="$sd_prog${LIBPATH:+:${LIBPATH}}"
    export LIBPATH
    ;;
esac

for arg in "$@"
do
  case "$arg" in
       #collect all bootstrap variables specified on the command line
       #so that they can be passed as arguments to javaldx later on
       -env:*) BOOTSTRAPVARS=$BOOTSTRAPVARS" ""$arg";;

       # make sure shared extensions will be readable by all users
       --shared) umask 0022;;
  esac
done

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "${sd_prog}/javaldx" ] ; then
    my_path=$("${sd_prog}/javaldx" "$BOOTSTRAPVARS" \
        "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc")
    if [ -n "$my_path" ] ; then
        sd_platform=$(uname -s)
        case "$sd_platform" in
          AIX)
            LIBPATH="$my_path${LIBPATH:+:$LIBPATH}"
            export LIBPATH
            ;;
          *)
            LD_LIBRARY_PATH="$my_path${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
            export LD_LIBRARY_PATH
            ;;
        esac
    fi
fi

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

# execute binary
exec "$sd_prog/unopkg.bin" "$@" \
    "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"
