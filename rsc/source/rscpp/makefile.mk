#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2003-03-26 15:50:48 $
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
PRJ=..$/..

TARGETTYPE=CUI
TARGETTHREAD=ST

PRJNAME=rsc
TARGET=rscpp

.IF "$(cpp)" != ""
PRJNAME=CPP
TARGET=cpp
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

CDEFS+=-DSOLAR

.IF "$(cpp)" != ""
CDEFS+=-DNOMAIN
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=   \
            $(OBJ)$/cpp1.obj \
            $(OBJ)$/cpp2.obj \
            $(OBJ)$/cpp3.obj \
            $(OBJ)$/cpp4.obj \
            $(OBJ)$/cpp5.obj \
            $(OBJ)$/cpp6.obj	\

.IF "$(cpp)" == ""
LIBSALCPPRT=$(0)
APP1TARGET= $(TARGET)
APP1LIBS=$(LB)$/$(TARGET).lib
APP1STACK=  32768
.ENDIF

.IF "$(GUI)"=="UNX"
CDEFS+=-Dunix
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

cpp1.c: cppdef.h cpp.h
cpp2.c: cppdef.h cpp.h
cpp3.c: cppdef.h cpp.h
cpp4.c: cppdef.h cpp.h
cpp5.c: cppdef.h cpp.h
cpp6.c: cppdef.h cpp.h 


