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

PRJNAME=icc
TARGET=icc

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=SampleICC-1.3.2
TARFILE_MD5=fdb27bfe2dbe2e7b57ae194d9bf36bab
PATCH_FILES= \
    $(TARFILE_NAME).patch \
    $(TARFILE_NAME)-fmtargs.patch

CONVERTFILES= \
    IccProfLib$/IccTagProfSeqId.h \
    IccProfLib$/IccTagProfSeqId.cpp \
    Contrib$/ICC_utils$/Stubs.h \
    Contrib$/ICC_utils$/Vetters.cpp

CONFIGURE_ACTION= $(GNUCOPY) -r $(BACK_PATH)..$/source$/create_sRGB_profile Contrib$/CmdLine && unzip -o $(BACK_PATH)..$/makefiles.zip
BUILD_ACTION=dmake &&  cd Contrib$/CmdLine$/create_sRGB_profile && $(AUGMENT_LIBRARY_PATH) .$/create_sRGB_profile

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


