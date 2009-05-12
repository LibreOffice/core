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
PRJ=..

PRJNAME=			framework
TARGET=				test
LIBTARGET=			NO
ENABLE_EXCEPTIONS=	TRUE
USE_DEFFILE=		TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- applikation: "test" --------------------------------------------------

#APP1TARGET= 	test

#APP1OBJS=		$(SLO)$/test.obj
#DEPOBJFILES+= $(APP1OBJS)

#APP1LIBS=		$(SLB)$/fwk_classes.lib				\
#				$(SLB)$/fwk_helper.lib

#APP1STDLIBS=	$(CPPULIB)							\
#				$(CPPUHELPERLIB)					\
#				$(SALLIB)							\
#				$(VOSLIB)							\
#				$(TOOLSLIB) 						\
#				$(SVTOOLLIB)						\
#				$(TKLIB)							\
#				$(COMPHELPERLIB)					\
#				$(VCLLIB)

#APP1DEPN=		$(SLB)$/fwk_helper.lib				\
#				$(SLB)$/fwk_classes.lib

# --- application: "threadtest" --------------------------------------------------

APP2TARGET= 	threadtest

APP2OBJS=		$(SLO)$/threadtest.obj				\
                $(SLO)$/transactionmanager.obj		\
                $(SLO)$/transactionguard.obj		\
                $(SLO)$/fairrwlock.obj				\
                $(SLO)$/resetableguard.obj			\
                $(SLO)$/gate.obj					\
                $(SLO)$/readguard.obj				\
                $(SLO)$/writeguard.obj

DEPOBJFILES+= $(APP2OBJS)

APP2STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(SALLIB)							\
                $(VOSLIB)							\
                $(VCLLIB)

APP2DEPN=		$(SLO)$/fairrwlock.obj				\
                $(SLO)$/transactionmanager.obj		\
                $(SLO)$/transactionguard.obj		\
                $(SLO)$/resetableguard.obj			\
                $(SLO)$/gate.obj					\
                $(SLO)$/readguard.obj				\
                $(SLO)$/writeguard.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

