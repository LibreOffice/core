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

PRJ=..
PRJPCH=

PRJNAME=udkapi
TARGET=udkapi

#use_starjar=true


# --- Settings -----------------------------------------------------

.INCLUDE :  makefile.pmk

# ------------------------------------------------------------------

UNOIDLDBFILES= \
    $(UCR)$/css.db \
    $(UCR)$/cssutil.db \
    $(UCR)$/cssbeans.db \
    $(UCR)$/cssbridge.db \
    $(UCR)$/cssboleautomation.db \
    $(UCR)$/cssconnection.db\
    $(UCR)$/csscontainer.db \
    $(UCR)$/cssio.db \
    $(UCR)$/cssjava.db \
    $(UCR)$/csslang.db \
    $(UCR)$/csssec.db \
    $(UCR)$/cssloader.db \
    $(UCR)$/cssreflection.db \
    $(UCR)$/cssregistry.db \
    $(UCR)$/cssscript.db \
    $(UCR)$/csstest.db \
    $(UCR)$/cssperftest.db \
    $(UCR)$/cssbridgetest.db \
    $(UCR)$/cssuno.db \
    $(UCR)$/cssulog.db \
    $(UCR)$/csscorba.db \
    $(UCR)$/cssiop.db \
    $(UCR)$/cssiiop.db \
    $(UCR)$/cssgiop.db \
    $(UCR)$/csstask.db \
    $(UCR)$/cssuri.db

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


