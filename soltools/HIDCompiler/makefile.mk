#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 14:06:58 $
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
TARGET=hidc
TARGETTYPE=CUI
LIBTARGET=no
NO_DEFAULT_STL=TRUE
INCPRE=$(MISC)

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk
.INCLUDE :  libs.mk
CDEFS+= -DYY_NEVER_INTERACTIVE=1

UWINAPILIB=$(0)

# --- Files --------------------------------------------------------

# HID compiler
APP1TARGET=     $(TARGET)
APP1OBJS=   $(OBJ)$/wrap_hidclex.obj
APP1DEPN=   $(OBJ)$/wrap_hidclex.obj
APP1LIBSALCPPRT=

DEPOBJFILES=$(APP1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/%_yy.cxx : %lex.l
    flex -l -8 -o$@ $<

$(OBJ)$/wrap_hidclex.obj: $(MISC)$/hidc_yy.cxx
