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

PRJ=.

PRJNAME=afms
TARGET=afms

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.IF "$(L10N_framework)"==""
# --- Files --------------------------------------------------------

TARFILE_NAME=Adobe-Core35_AFMs-314
TARFILE_MD5=1756c4fa6c616ae15973c104cd8cb256
TARFILE_ROOTDIR=Adobe-Core35_AFMs-314

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk

ZIP1DIR         = $(MISC)$/build$/$(TARFILE_NAME)
ZIP1TARGET      = fontunxafm
ZIP1LIST        = *.afm -x "*Helvetica-Narrow*"

.ENDIF # L10N_framework
.INCLUDE : target.mk

.INCLUDE : tg_ext.mk
.IF "$(L10N_framework)"==""
.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN):	$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)

.ENDIF          # "$(ZIP1TARGETN)"!=""

.ENDIF # L10N_framework
