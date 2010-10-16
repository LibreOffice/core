#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2007/11/23 13:58:12 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2007 by Sun Microsystems, Inc.
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

PRJ := ..$/..
PRJNAME := configmgr
TARGET := qa_unit

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

DLLPRE = # no leading "lib" on .so files

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/ucalc.obj
SHL1STDLIBS=       \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXCORELIB)		\
    $(EDITENGLIB)		\
    $(SVXLIB)		\
    $(BASEGFXLIB) \
    $(DRAWINGLAYERLIB) \
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(TKLIB)		\
    $(VOSLIB)		\
    $(SALLIB)		\
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(UNOTOOLSLIB) \
    $(SOTLIB)		\
    $(XMLOFFLIB)	\
    $(AVMEDIALIB) \
    $(FORLIB) \
    $(FORUILIB) \
    $(CPPUNITLIB)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/scalc3.lib $(SLB)$/scalc3c.lib 
# SHL1VERSIONMAP = export.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR: test

test .PHONY: $(SHL1TARGETN)
    LD_LIBRARY_PATH=$(SOLARVER)$/$(INPATH)$/lib $(SOLARVER)$/$(INPATH)$/bin$/cppunittester $(PRJ)$/$(INPATH)$/lib$/$(TARGET)$(DLLPOST)
