#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-08-12 08:19:39 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#	
#
#*************************************************************************

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=quickstarter
TARGET1=sdqsmsi
TARGET2=qslnkmsi

.IF "$(GUI)"=="WNT"

# --- Settings -----------------------------------------------------

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

CFLAGS+=-D_STLP_USE_STATIC_LIB
UWINAPILIB=

# --- Files --------------------------------------------------------


SHL1OBJS =	$(SLO)$/shutdown_quickstart.obj \
            $(SLO)$/quickstarter.obj

SHL1STDLIBS=	kernel32.lib\
                user32.lib\
                advapi32.lib\
                shell32.lib\
                msi.lib\
                $(LIBSTLPORTST)								

SHL1TARGET = $(TARGET1)
SHL1IMPLIB = i$(TARGET1)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
SHL1DEPN=$(SHL1OBJS)
DEF1EXPORTFILE=$(TARGET1).dxp

# --- Files --------------------------------------------------------

SHL2OBJS =	$(SLO)$/remove_quickstart_link.obj \
            $(SLO)$/quickstarter.obj

SHL2STDLIBS=	kernel32.lib\
                user32.lib\
                advapi32.lib\
                shell32.lib\
                msi.lib\
                $(LIBSTLPORTST)								

SHL2TARGET = $(TARGET2)
SHL2IMPLIB = i$(TARGET2)

SHL2DEF = $(MISC)$/$(SHL2TARGET).def
SHL2BASE = 0x1c000000
DEF2NAME=$(SHL2TARGET)
SHL2DEPN=$(SHL1OBJS)
DEF2EXPORTFILE=$(TARGET2).dxp

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------


.ENDIF

