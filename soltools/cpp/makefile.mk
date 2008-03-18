#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:07:10 $
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

PRJNAME=soltools
TARGET=cpp
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/_cpp.obj		\
    $(OBJ)$/_eval.obj 	\
    $(OBJ)$/_getopt.obj	\
    $(OBJ)$/_include.obj \
    $(OBJ)$/_lex.obj 	\
    $(OBJ)$/_macro.obj 	\
    $(OBJ)$/_mcrvalid.obj \
    $(OBJ)$/_nlist.obj 	\
    $(OBJ)$/_tokens.obj 	\
    $(OBJ)$/_unix.obj

APP1TARGET	=	$(TARGET)
.IF "$(GUI)" != "OS2"
APP1STACK	=	1000000
.ENDIF
APP1LIBS	=	$(LB)$/$(TARGET).lib
APP1DEPN	=   $(LB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


