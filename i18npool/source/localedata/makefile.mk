#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: ihi $ $Date: 2008-01-16 14:33:08 $
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
#************************************************************************/

PRJ=..$/..


PRJNAME=i18npool
TARGET=locale

TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)" == "SOLARIS"
# the xmlparser build breaks in this header file
# 
#      /opt/SUNWspro/WS6U1/include/CC/Cstd/./limits
#
# which defines a class with member functions called min() and max().
# the build breaks because in solar.h, there is something like this
# 
#      #define max(x,y) ((x) < (y) ? (y) : (x))
#      #define min(x,y) ((x) < (y) ? (x) : (y))
#
# so the only choice is to prevent "CC/Cstd/./limits" from being 
# included:
CDEFS+=-D__STD_LIMITS
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/localedata.obj

OBJFILES   = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

APP1TARGET = saxparser

APP1OBJS   = $(OBJFILES)

APP1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB)

DEPOBJFILES = \
        $(OBJ)$/saxparser.obj \
        $(OBJ)$/LocaleNode.obj \
        $(OBJ)$/filewriter.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

