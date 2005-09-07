#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:53:49 $
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

APP1STDLIBS=	\
    $(TOOLSLIB)	\
    $(SALLIB)

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

.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
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

