#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: tra $ $Date: 2001-03-14 14:54:51 $
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

PRJ=..$/..$/..

PRJNAME=		dtrans
TARGET=			testwincb
TARGET1=		testmshl
LIBTARGET=		NO
TARGETTYPE=		CUI
USE_BOUNDCHK=
TESTCB=TRUE

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------------------------------
#.INCLUDE :		$(PRJ)$/util$/makefile.pmk

.INCLUDE :  settings.mk

.IF "$(TESTCB)"=="TRUE"

.INCLUDE : ..$/..$/cppumaker.mk

CFLAGS+=/D_WIN32_DCOM /EHsc

# --- Files --------------------------------------------------------

OBJFILES=   	$(OBJ)$/test_wincb.obj
APP1TARGET=		$(TARGET)
APP1OBJS=		$(OBJ)$/test_wincb.obj
                
                
APP1STDLIBS=	$(SALLIB) \
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                user32.lib \
                ole32.lib

APP1LIBS= $(SLB)$/dtutils.lib\
          $(SOLARLIBDIR)$/user9x.lib\
          $(SOLARLIBDIR)$/tools32.lib

APP1NOSAL=  TRUE

.ENDIF

.IF "$(TESTCB)"==""

OBJFILES=   	$(OBJ)$/testmarshal.obj
APP1TARGET=		$(TARGET1)
APP1OBJS=		$(OBJ)$/testmarshal.obj\
                $(OBJ)$/XTDo.obj\
                $(OBJ)$/DTransHelper.obj
                
APP1STDLIBS=	$(SALLIB)\
                user32.lib\
                ole32.lib

APP1LIBS= 

APP1NOSAL=  TRUE

.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE :		target.mk
#.INCLUDE :		$(PRJ)$/util$/target.pmk
