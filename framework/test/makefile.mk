#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: obo $ $Date: 2007-03-09 08:43:47 $
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

