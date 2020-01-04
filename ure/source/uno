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

# Pass -env arguments on to javaldx; needs to be fixed:
my_envargs=
for my_arg in "$@" ; do
  case ${my_arg} in
    -env:*) my_envargs="${my_envargs} ${my_arg}" ;;
  esac
done

# Extend the LD_LIBRARY_PATH for Java:
epath=$(dirname "$0")
if [ -x "${epath}/javaldx" ] ; then
    jpath=$("${epath}/javaldx" $my_envargs)
    if [ -n "${jpath}" ]; then
        sd_platform=$(uname -s)
        case $sd_platform in
          AIX)
            LIBPATH=${jpath}${LIBPATH:+:${LIBPATH}}
            export LIBPATH
            ;;
          *)
            LD_LIBRARY_PATH=${jpath}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
            export LD_LIBRARY_PATH
            ;;
        esac
    fi
fi

exec "$0.bin" "$@"
