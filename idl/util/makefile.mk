#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

PRJPCH=

PRJNAME=SVIDL
TARGET=idl
VERSION=$(UPD)

TARGETTYPE=CUI

# --- Settings -----------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------------

LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1FILES = $(LB)$/prj.lib		 \
            $(LB)$/objects.lib	 \
            $(LB)$/cmptools.lib

.IF "$(GUI)" != "WIN"
APP1TARGET= svidl

.IF "$(GUI)" != "MAC"
APP1DEPN=       $(L)$/itools.lib
.ENDIF

APP1STDLIBS=\
                $(TOOLSLIB)

APP1LIBS=	$(LIBPRE) $(LB)$/$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(COM)"=="WTC"
APP1OBJS=	$(OBJ)$/svidl.obj
.ELSE
APP1OBJS=	$(OBJ)$/svidl.obj	\
            $(OBJ)$/command.obj
.ENDIF
.ENDIF
.IF "$(COM)" == "WTC"
APP1STACK=32768
.ENDIF
.IF "$(GUI)" != "OS2"
APP1STACK=32768
.ENDIF
.ENDIF

.IF "$(OS)"=="LINUX"
APP1STDLIBS+=-lcrypt
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

# -------------------------------------------------------------------
# PM2
# -------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(APP1TARGET).def : makefile
    echo  NAME			SV-IDL WINDOWCOMPAT 				>$@
    echo  DESCRIPTION	'SV-IDL-Compiler'                  >>$@
.IF "$(COM)" != "BLC"
    echo  STUB			'os2STUB.EXE'                      >>$@
.ENDIF
    echo  DATA			MULTIPLE						   >>$@
    echo  EXETYPE		OS2 							   >>$@
    echo  PROTMODE										   >>$@
    echo  HEAPSIZE		16000							   >>$@
    echo  STACKSIZE 	48000							   >>$@
.ENDIF

# --- Filter-Datei ---

.IF "$(GUI)" != "DOS"
$(MISC)$/$(SHL1TARGET).flt: makefile
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo Stg>>$@
.ENDIF

