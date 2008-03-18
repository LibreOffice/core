#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:07:47 $
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

PRJNAME=soltools
TARGET=make_makedepend
PRJ=..
TARGETTYPE=CUI
LIBTARGET=NO
# noadjust here to have dependencies over there
noadjust=TRUE
nodep=true

# "mkdepend" is written in K&R style C. Modern compilers will generate
# lots of warning. There is no point in cleaning this up, so we just
# ignore warnings
EXTERNAL_WARNINGS_NOT_ERRORS=TRUE

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE : settings.mk

LIBSALCPPRT=
UWINAPILIB=

CDEFS+=-DNO_X11 -DXP_PC -DHW_THREADS

OBJFILES=  \
        $(OBJ)$/cppsetup.obj \
        $(OBJ)$/ifparser.obj \
        $(OBJ)$/include.obj \
        $(OBJ)$/main.obj \
        $(OBJ)$/parse.obj \
        $(OBJ)$/pr.obj \
        $(OBJ)$/collectdircontent.obj \
        $(NULL)

APP1TARGET=makedepend
APP1OBJS=$(OBJFILES)


.INCLUDE : target.mk


