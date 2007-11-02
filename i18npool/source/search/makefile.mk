#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 12:38:04 $
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

PRJ=..$/..

PRJNAME=i18npool
TARGET=i18nsearch.uno
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Files --------------------------------------------------------

EXCEPTIONSFILES= \
            $(SLO)$/textsearch.obj

SLOFILES=	\
            $(EXCEPTIONSFILES) \
            $(SLO)$/levdis.obj

SHL1TARGET= $(TARGET)
SHL1OBJS=	$(SLOFILES)

.IF ("$(GUI)"=="UNX" || "$(COM)"=="GCC") && "$(GUI)"!="OS2"
I18NREGEXPLIB=-li18nregexp$(COMID)
.ELSE
I18NREGEXPLIB=ii18nregexp.lib
.ENDIF

SHL1STDLIBS= \
                $(CPPULIB) \
                $(CPPUHELPERLIB) \
                $(SALLIB) \
                $(I18NREGEXPLIB)

SHL1DEPN=		makefile.mk
SHL1VERSIONMAP= i18nsearch.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

