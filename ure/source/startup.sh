#!/bin/sh
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: startup.sh,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:13:09 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
jpath=`"${epath}/javaldx" $my_envargs`
if [ -n "${jpath}" ]; then
    LD_LIBRARY_PATH=${jpath}${LD_LIBRARY_PATH+:${LD_LIBRARY_PATH}}
    export LD_LIBRARY_PATH
fi

exec "$0.bin" "$@"
