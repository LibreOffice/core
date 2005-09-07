#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:12:19 $
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

PRJ=..$/..$/..

PRJNAME=		sysshell
TARGET=			testsyssh
TARGET1=		testsmplmail
TARGET2=		testprx
LIBTARGET=		NO
TARGETTYPE=		CUI


# --- Settings -----------------------------------------------------
#.INCLUDE :		$(PRJ)$/util$/makefile.pmk

.INCLUDE :  settings.mk

.INCLUDE : ..$/..$/cppumaker.mk

# --- Files --------------------------------------------------------

CFLAGS+=/GX

OBJFILES1=$(OBJ)$/TestSysShExec.obj
OBJFILES2=$(OBJ)$/TestSmplMail.obj
OBJFILES3=$(OBJ)$/TestProxySet.obj

OBJFILES=$(OBJFILES1)\
         $(OBJFILES2)

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES1)

APP1STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                user32.lib
                
APP1DEF=		$(MISC)$/$(APP1TARGET).def


# --- TestSmplMail ---

APP2TARGET=$(TARGET1)
APP2OBJS=$(OBJFILES2)

APP2STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                user32.lib
                
APP2DEF=		$(MISC)$/$(APP2TARGET).def

# --- TestProxy ---

APP3TARGET=$(TARGET2)
APP3OBJS=$(OBJFILES3)

APP3STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB)

APP3DEF=		$(MISC)$/$(APP3TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :		target.mk


