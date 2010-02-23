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

PRJ=..$/..
PRJINC=$(PRJ)$/source

PRJNAME=configmgr

TARGET=cfgfile
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# ... common for all test executables ..............................
APPSTDLIBS=\
            $(SALLIB) \
         $(VOSLIB) \
            $(CPPULIB)	\
        $(UNOTOOLSLIB) \
            $(CPPUHELPERLIB) \


# ... FileTests .....................................................
APP1STDLIBS = $(APPSTDLIBS) \
    $(STDLIBCPP) \

APP1STDLIBS+=$(STDLIBCPP)

APP1TARGET= $(TARGET)

APP1OBJS=	\
    $(SLO)$/cfgfile.obj \
    $(SLO)$/oslstream.obj \
    $(SLO)$/filehelper.obj \
    $(SLO)$/simpletest.obj \
    $(SLO)$/strimpl.obj \
    $(SLO)$/confname.obj \

#	$(SLO)$/xmlexport.obj \
#	$(SLO)$/xmlimport.obj \

# 	  $(SLO)$/xmltreebuilder.obj \
# 	  $(SLO)$/cmtree.obj \
# 	  $(SLO)$/cmtreemodel.obj \
# 	  $(SLO)$/typeconverter.obj \
# 	  $(SLO)$/changes.obj \
# 	  $(SLO)$/xmlformater.obj \
# 	  $(SLO)$/attributes.obj \
# 	  $(SLO)$/tracer.obj \


# ... common for all test executables ..............................
# APP2STDLIBS = $(APPSTDLIBS)
# 
# APP2STDLIBS+=$(STDLIBCPP)
# 
# APP2TARGET= cfglocal
# APP2OBJS=	\
# 	  $(SLO)$/cfglocal.obj	\
# 	  $(SLO)$/receivethread.obj	\
# 	  $(SLO)$/redirector.obj	\
# 	  $(SLO)$/socketstream.obj	\
# 	  $(SLO)$/attributes.obj	\
# 	  $(SLO)$/localsession.obj \
# 	  $(SLO)$/saxtools.obj \
# 	  $(SLO)$/mergeupdates.obj \
# 	  $(SLO)$/oslstream.obj \
# 	  $(SLO)$/configsession.obj \
# 	  $(SLO)$/confname.obj \
# 	  $(SLO)$/sessionstream.obj \
# 	  $(SLO)$/filehelper.obj \
# 	  $(SLO)$/tracer.obj \
# 	  $(SLO)$/updatedom.obj \
# 	  $(SLO)$/strconverter.obj \
# 	  $(SLO)$/strimpl.obj \
# 

.INCLUDE :  target.mk


