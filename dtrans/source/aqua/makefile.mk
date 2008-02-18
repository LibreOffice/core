#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2008-02-18 14:50:30 $
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

PRJNAME=dtrans
TARGET=dtransaqua
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
.IF "$(OS)"=="MACOSX" 
.IF "$(GUIBASE)"=="aqua"
COMP1TYPELIST=$(TARGET)
.ENDIF # "$(GUIBASE)"=="aqua"
.ENDIF # "$(OS)"=="MACOSX" 

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(OS)"!="MACOSX"
dummy:
    @echo "Nothing to build for this platform"
.ELSE # "$(OS)"!="MACOSX"
.IF "$(GUIBASE)"!="aqua"
dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"
.ELSE

CFLAGSCXX+=-fconstant-cfstrings -x objective-c++ -fobjc-exceptions

SLOFILES= \
        $(SLO)$/aqua_clipboard.obj \
        $(SLO)$/DataFlavorMapping.obj \
        $(SLO)$/OSXTransferable.obj \
        $(SLO)$/HtmlFmtFlt.obj \
        $(SLO)$/PictToBmpFlt.obj \
        $(SLO)$/DropTarget.obj \
        $(SLO)$/DragSource.obj \
        $(SLO)$/service_entry.obj \
        $(SLO)$/DragSourceContext.obj \
        $(SLO)$/DragActionConversion.obj

SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        -framework Carbon \
        -framework QuickTime \
        -framework Cocoa

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1OBJS=	$(SLOFILES)

#APP1NOSAL=TRUE
#APP1TARGET=test_aquacb
#APP1OBJS=$(SLO)$/test_aquacb.obj
#APP1STDLIBS= 
#$(SALLIB)
#$(CPPULIB)
#$(CPPUHELPERLIB)
#-framework Cocoa

# --- Targets ------------------------------------------------------

ALL : ALLTAR
    +cd $(LB) && regcomp -register -r ../bin/$(COMP1TYPELIST).rdb -c $(SHL1TARGET)

.INCLUDE :	target.mk

.ENDIF		# "$(GUIBASE)"!="aqua"
.ENDIF		# "$(OS)"!="MACOSX"

