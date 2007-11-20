#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: ihi $ $Date: 2007-11-20 19:46:20 $
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

PRJ := ..$/..$/..
PRJNAME := sal

TARGET := qa_rtl_strings

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

SHL1TARGET := $(TARGET)
SHL1OBJS := \
    $(SLO)$/test_oustringbuffer_utf32.obj \
    $(SLO)$/test_oustring_compare.obj \
    $(SLO)$/test_oustring_convert.obj \
    $(SLO)$/test_oustring_endswith.obj \
    $(SLO)$/test_oustring_noadditional.obj
SHL1IMPLIB := i$(SHL1TARGET)
SHL1STDLIBS := $(CPPUNITLIB) $(SALLIB)
SHL1VERSIONMAP := $(PRJ)$/qa$/export.map
DEF1NAME := $(SHL1TARGET)

.INCLUDE: target.mk
.INCLUDE : _cppunit.mk
