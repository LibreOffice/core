#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# Pass -env arguments on to javaldx; needs to be fixed:
my_envargs=
for my_arg in "$@" ; do
  case ${my_arg} in
    -env:*) my_envargs="${my_envargs} ${my_arg}" ;;
  esac
done

# Extend the LD_LIBRARY_PATH for Java:
epath=`dirname "$0"`
if [ -x "${epath}/javaldx" ] ; then
    jpath=`"${epath}/javaldx" $my_envargs`
    if [ -n "${jpath}" ]; then
        LD_LIBRARY_PATH=${jpath}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
        export LD_LIBRARY_PATH
    fi
fi

exec "$0.bin" "$@"
