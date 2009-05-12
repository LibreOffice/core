#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.14 $
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
PRJ=..$/..

PRJNAME=			framework
TARGET=             typecfg
LIBTARGET=			NO
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- applikation: "xml2xcd" --------------------------------------------------

# --- applikation: "cfgview" --------------------------------------------------

APP2TARGET=     cfgview

APP2OBJS=		$(SLO)$/cfgview.obj					\
                $(SLO)$/servicemanager.obj			\
                $(SLO)$/filtercachedata.obj			\
                $(SLO)$/filtercache.obj				\
                $(SLO)$/wildcard.obj				\
                $(SLO)$/lockhelper.obj

DEPOBJFILES=$(APP2OBJS)

APP2STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(SALLIB)							\
                $(VOSLIB)							\
                $(TOOLSLIB) 						\
                $(SVTOOLLIB)						\
                $(TKLIB)							\
                $(COMPHELPERLIB)					\
                $(UNOTOOLSLIB)						\
                $(VCLLIB)

APP2DEPN=		$(SLO)$/servicemanager.obj			\
                $(SLO)$/filtercachedata.obj			\
                $(SLO)$/filtercache.obj				\
                $(SLO)$/wildcard.obj				\
                $(SLO)$/lockhelper.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

