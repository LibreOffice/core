#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hjs $ $Date: 2001-09-18 16:13:48 $
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

PRJ=..

PRJNAME=basic
TARGET=miniapp
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- SBASIC IDE --------------------------------------------------------

APP1TARGET=$(PRJNAME)
APP1STDLIBS= \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(UNOTOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(SVLIB) \
            $(SO2LIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SJLIB) \
            $(SOTLIB) \
            $(VOSLIB) \
            $(SVMEMLIB)

.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
APP1STDLIBS+=$(CPPULIB)
.ENDIF
.IF "$(GUI)"=="UNX"
APP1STDLIBS+= \
            $(VOSLIB) \
            $(SALLIB)
.ENDIF

APP1LIBS= \
            $(LIBPRE) $(LB)$/basic.lib \
            $(LIBPRE) $(LB)$/app.lib \
            $(LIBPRE) $(LB)$/sample.lib
.IF "$(GUI)"=="UNX"
APP1STDLIBS+=	\
            $(BASICLIB)
.ENDIF


APP1DEPN=	$(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/basic.lib $(LB)$/app.lib $(LB)$/sample.lib

APP1OBJS = $(OBJ)$/ttbasic.obj 

.IF "$(GUI)" != "UNX"
APP1OBJS+=	\
            $(OBJ)$/app.obj \
            $(SLO)$/sbintern.obj
.ENDIF

# --- TESTTOOL MINIAPP ------------------------------------------------------

SRS3FILES= $(SRS)$/miniapp.srs
RES3TARGET=miniapp

APP3TARGET=miniapp
APP3STDLIBS= \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(PLUGCTORLIB) \
            $(SVLIB) \
            $(SVMEMLIB) \
            $(SJLIB) \
            $(SO2LIB)
.IF "$(GUI)"=="UNX"
APP3STDLIBS+= \
            $(VOSLIB) $(OSLLIB)
.ENDIF


APP3LIBS= \
        $(LIBPRE) $(LB)$/miniapp.lib

APP3DEPN=\
        $(L)$/svtool.lib \
        $(L)$/itools.lib \
        $(SVLIBDEPEND) \
        $(LB)$/miniapp.lib

.IF "$(GUI)" != "UNX"
#               win16 braucht ein appobj
APP3OBJS=               $(OBJ)$/testapp.obj
.ENDIF

APP3RES=        $(RES)$/miniapp.res

.INCLUDE :  target.mk

