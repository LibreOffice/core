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
# $Revision: 1.9 $
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
TARGET=				threadtest
LIBTARGET=			NO
ENABLE_EXCEPTIONS=	TRUE
USE_DEFFILE=		TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- application: "threadtest" --------------------------------------------------

APP1TARGET= 	threadtest

APP1OBJS=		$(SLO)$/threadtest.obj							\
                $(SLO)$/lockhelper.obj

DEPOBJFILES=$(APP1OBJS)

# [ed] 6/16/02 Add the transaction manager library on OS X

APP1STDLIBS=	$(CPPULIB)										\
                $(CPPUHELPERLIB)								\
                $(SALLIB)										\
                $(VOSLIB)										\
                $(VCLLIB)

APP1DEPN=		$(INC)$/threadhelp$/threadhelpbase.hxx			\
                $(INC)$/threadhelp$/transactionbase.hxx			\
                $(INC)$/threadhelp$/transactionmanager.hxx		\
                $(INC)$/threadhelp$/transactionguard.hxx		\
                $(INC)$/threadhelp$/resetableguard.hxx			\
                $(INC)$/threadhelp$/readguard.hxx				\
                $(INC)$/threadhelp$/writeguard.hxx

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

