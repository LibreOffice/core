#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: rt $ $Date: 2007-11-13 14:15:54 $
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
PRJNAME=svtools
TARGET=bmp
LIBTARGET=NO

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true

.INCLUDE :  settings.mk

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
    $(VCLLIB)		\
    $(TOOLSLIB)		\
    $(VOSLIB) 		\
    $(SALLIB)

APP1OBJS=   $(OBJ)$/bmp.obj			\
            $(OBJ)$/bmpcore.obj		
            
APP1BASE=0x10000000

# --- APP2TARGET --------------------------------------------------

APP2TARGET	=	bmpsum
APP2BASE	=	0x10000000
APP2OBJS	=   $(OBJ)$/bmpsum.obj

APP2STDLIBS	=	$(VCLLIB)		\
                $(TOOLSLIB)		\
                $(VOSLIB) 		\
                $(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
