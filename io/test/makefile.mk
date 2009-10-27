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
# $Revision: 1.12 $
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

PRJNAME=extensions
TARGET=workben
LIBTARGET=NO

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# --- Files --------------------------------------------------------

OBJFILES=	$(OBJ)$/testcomponent.obj \
        $(OBJ)$/testconnection.obj

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

UNOTYPES =	com.sun.star.connection.XConnector \
        com.sun.star.connection.XAcceptor  \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.lang.XComponent \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.test.XSimpleTest            \
        com.sun.star.lang.XSingleComponentFactory \
        com.sun.star.lang.XMultiComponentFactory


#
# std testcomponent
#

APP1TARGET = testcomponent
APP1OBJS   = $(OBJ)$/testcomponent.obj
APP1STDLIBS = 	$(SALLIB) \
        $(CPPULIB)\
        $(CPPUHELPERLIB)

APP2TARGET = testconnection
APP2OBJS   = $(OBJ)$/testconnection.obj
APP2STDLIBS = 	$(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB)



# --- Targets ------------------------------------------------------

ALL : 	$(BIN)$/applicat.rdb	\
    ALLTAR

$(BIN)$/applicat.rdb: $(SOLARBINDIR)$/udkapi.rdb
    rm -f $@
    regmerge $@ / $?

.ENDIF 		# L10N_framework

.INCLUDE :  target.mk
