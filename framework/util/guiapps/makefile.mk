#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2007-03-09 08:44:20 $
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
PRJ=..$/..

PRJNAME=			framework
TARGET=				framework_guiapp
TARGETTYPE=GUI

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
GEN_HID=                TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- login applikation -------------------------------------------------------

APP1TARGET= 	login

APP1OBJS=		$(SLO)$/login.obj					\
                $(SLO)$/servicemanager.obj			\
                $(SLO)$/lockhelper.obj				\
                $(SLO)$/transactionmanager.obj

DEPOBJFILES=$(APP1OBJS)

APP1STDLIBS=	$(CPPULIB)							\
                $(CPPUHELPERLIB)					\
                $(SALLIB)							\
                $(VOSLIB)							\
                $(TOOLSLIB) 						\
                $(SVTOOLLIB)						\
                $(TKLIB)							\
                $(COMPHELPERLIB)					\
                $(VCLLIB)

APP1DEPN=		$(SLO)$/servicemanager.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE :	target.mk

