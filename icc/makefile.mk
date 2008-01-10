#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: pl $ $Date: 2008-01-10 19:42:35 $
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

PRJ=.

PRJNAME=icc
TARGET=icc

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=SampleICC-1.3.2
PATCH_FILE_NAME=$(TARFILE_NAME).patch
BUILD_ACTION=$(GNUMAKE) && cd Contrib/CmdLine/create_sRGB_profile && ./create_sRGB_profile
CONVERTFILES= \
    IccProfLib/IccTagProfSeqId.h \
    IccProfLib/IccTagProfSeqId.cpp \
    Contrib/ICC_utils/Stubs.h \
    Contrib/ICC_utils/Vetters.cpp

.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"=="4nt"
CONFIGURE_ACTION= $(GNUCOPY) -r $(BACK_PATH)../source/create_sRGB_profile Contrib/CmdLine && unzip $(BACK_PATH)..$/makefiles.zip
BUILD_ACTION=dmake &&  cd Contrib/CmdLine/create_sRGB_profile && ./create_sRGB_profile
.ELSE
CONFIGURE_ACTION= $(COPY) -r $(BACK_PATH)../source/create_sRGB_profile Contrib/CmdLine && CC="" CXX="" ./configure
.ENDIF
.ELSE
CONFIGURE_ACTION= $(COPY) -r $(BACK_PATH)../source/create_sRGB_profile Contrib/CmdLine && ./configure
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


