#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: jsc $ $Date: 2005-09-09 13:50:31 $
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

PRJ := ..$/..
PRJNAME := unodevtools

TARGET := uno-skeletonmaker
TARGETTYPE := CUI
LIBTARGET := NO

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

APP1TARGET = $(TARGET)
APP1OBJS = $(OBJ)$/skeletonmaker.obj \
    $(OBJ)$/skeletoncommon.obj \
    $(OBJ)$/javatypemaker.obj \
    $(OBJ)$/cpptypemaker.obj \
    $(OBJ)$/javacompskeleton.obj \
    $(OBJ)$/cppcompskeleton.obj



APP1STDLIBS = $(REGLIB) $(SALLIB) $(SALHELPERLIB) $(CPPULIB) $(CPPUHELPERLIB)
APP1LIBS = $(LB)$/unodevtools.lib \
    $(SOLARLIBDIR)$/codemaker.lib \
    $(SOLARLIBDIR)$/commonjava.lib \
    $(SOLARLIBDIR)$/commoncpp.lib

OBJFILES = $(APP1OBJS)

.INCLUDE: target.mk
