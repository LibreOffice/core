#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:03:12 $
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

PRJNAME=tl
TARGET=tldem
LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

CXXFILES=   tldem.cxx solar.c demostor.cxx fstest.cxx

OBJFILES=   $(OBJ)$/tldem.obj $(OBJ)$/solar.obj $(OBJ)$/demostor.obj $(OBJ)$/fstest.obj

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/tldem.obj
.IF "$(GUI)" == "UNX"
APP1LIBS=    $(TOOLSLIB)
.ELSE
APP1LIBS=   $(LB)$/itools.lib
# APP1DEPN=   $(LB)$/$(TOOLSLIB)
.ENDIF

APP2TARGET= solar
APP2OBJS=   $(OBJ)$/solar.obj
.IF "$(GUI)" == "UNX"
APP2STDLIBS=    $(TOOLSLIB)
.ELSE
APP2LIBS=   $(LB)$/itools.lib
APP2DEPN=   $(LB)$/itools.lib
.ENDIF

APP3TARGET=     demostor
APP3OBJS=       $(OBJ)$/demostor.obj
.IF "$(GUI)" == "UNX"
APP3STDLIBS=    $(TOOLSLIB) $(VOSLIB) $(SALLIB)
.ELSE
APP3STDLIBS=    $(LB)$/itools.lib ivos.lib isal.lib
.ENDIF

APP4TARGET=     fstest
APP4OBJS=       $(OBJ)$/fstest.obj
.IF "$(GUI)" == "UNX"
APP4STDLIBS=    $(TOOLSLIB) $(VOSLIB) $(SALLIB)
.ELSE
APP4STDLIBS=    $(LB)$/itools.lib ivos.lib isal.lib
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

