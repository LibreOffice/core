#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 14:35:55 $
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
PRJNAME=bmpmaker
TARGET=bmp
TARGETTYPE= 
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

OBJFILES=   $(OBJ)$/bmp.obj			\
            $(OBJ)$/bmpgui.obj		\
            $(OBJ)$/bmpcore.obj		\
            $(OBJ)$/g2g.obj			\
            $(OBJ)$/bmpsum.obj      


SRC1FILES=bmp.src
SRS1NAME=bmp
RES1TARGET=bmp
SRS1FILES=$(SRS)$/bmp.srs

# --- APP1TARGET ---------------------------------------------------

APP1TARGET= $(TARGET)

APP1STDLIBS =		\
    $(SVLIB)		\
    $(TOOLSLIB)		\
    $(VOSLIB) 		\
    $(SALLIB)

APP1DEPN=   $(L)$/itools.lib  $(SVLIBDEPEND)

APP1OBJS=   $(OBJ)$/bmp.obj			\
            $(OBJ)$/bmpcore.obj		
            
APP1BASE=0x10000000

# .IF "$(GUI)"!="UNX"
# APP1STDLIBS+= svtool.lib
# .ELSE
# APP1STDLIBS+= -lsvt$(UPD)$(DLLSUFFIX) 
# APP1STDLIBS+= -lsvl$(UPD)$(DLLSUFFIX) 
# .ENDIF

.IF "$(OS)"=="MACOSX"
ALL:	$(BIN)$/$(RES1TARGET).res ALLTAR
.ENDIF

# --- APP2TARGET --------------------------------------------------

APP2TARGET	=	bmpsum
APP2BASE	=	0x10000000
APP2DEPN	=   $(L)$/itools.lib  $(SVLIBDEPEND)
APP2OBJS	=   $(OBJ)$/bmpsum.obj

APP2STDLIBS	=	$(SVLIB)		\
                $(TOOLSLIB)		\
                $(VOSLIB) 		\
                $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
