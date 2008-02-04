#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.9 $
#
#  last change: $Author: ihi $ $Date: 2008-02-04 13:00:17 $
#
#  The Contents of this file are made available subject to
#  the terms of GNU Lesser General Public License Version 2.1.
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
# ***********************************************************************/
PRJ=..
PRJNAME=writerfilter
TARGET=writerfilter.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/component.obj

SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
RESOURCEMODELLIB=-lresourcemodel
.ELSE
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)\
    $(UCBHELPERLIB)\
    $(DOCTOKLIB) \
    $(OOXMLLIB) \
    $(RESOURCEMODELLIB)

SHL1LIBS=\
    $(SLB)$/debugservices_doctok.lib \
    $(SLB)$/debugservices_ooxml.lib

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS = $(SLO)$/component.obj

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

