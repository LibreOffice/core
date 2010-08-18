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


#.IF "$(GUI)"=="UNX" || "$(GUIBASE)"=="WIN"
#LANG_GUI=WIN
#.ELSE
LANG_GUI=$(GUI)
#.ENDIF

###################################################
#
# entries for charsets

# variable name equals $(solarlang)$(LANG_GUI)
#
###################################################

#deut$(LANG_GUI)*=-CHARSET_microsoft-cp1252

###################################################
#
# set defaults
#
###################################################

#.IF "$(solarlang)" == ""
#solarlang=deut
#.ENDIF

RSC_SRS_CHARSET=-CHARSET_DONTKNOW
default$(LANG_GUI)*=-CHARSET_UTF8

