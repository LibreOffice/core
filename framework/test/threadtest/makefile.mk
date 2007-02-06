#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 13:01:57 $
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
TARGET=				threadtest
LIBTARGET=			NO
ENABLE_EXCEPTIONS=	TRUE
USE_DEFFILE=		TRUE
NO_BSYMBOLIC=		TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
.ENDIF

# --- application: "threadtest" --------------------------------------------------

APP1TARGET= 	threadtest

APP1OBJS=		$(SLO)$/threadtest.obj							\
                $(SLO)$/lockhelper.obj

# [ed] 6/16/02 Add the transaction manager library on OS X

APP1STDLIBS=	$(CPPULIB)										\
                $(CPPUHELPERLIB)								\
                $(OSLLIB)										\
                $(SALLIB)										\
                $(VOSLIB)										\
                $(SVLIB)

APP1DEPN=		$(INC)$/threadhelp$/threadhelpbase.hxx			\
                $(INC)$/threadhelp$/transactionbase.hxx			\
                $(INC)$/threadhelp$/transactionmanager.hxx		\
                $(INC)$/threadhelp$/transactionguard.hxx		\
                $(INC)$/threadhelp$/resetableguard.hxx			\
                $(INC)$/threadhelp$/readguard.hxx				\
                $(INC)$/threadhelp$/writeguard.hxx

.IF "$(GUI)"=="WIN" || "$(GUI)"=="OS2"
APP1DEF=		$(MISC)$/threadtest.def
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
