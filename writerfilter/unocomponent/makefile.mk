#************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: hbrinkm $ $Date: 2006-11-01 09:30:31 $
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
ODIAPILIB=-lodiapi
RTFTOKLIB=-lrtftok
DOCTOKLIB=-ldoctok
.ELIF "$(GUI)"=="WNT"
ODIAPILIB=$(LB)$/iodiapi.lib
RTFTOKLIB=$(LB)$/irtftok.lib
DOCTOKLIB=$(LB)$/idoctok.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
    $(CPPULIB)\
    $(COMPHELPERLIB)\
    $(CPPUHELPERLIB)\
    $(UCBHELPERLIB)\
    $(ODIAPILIB) \
    $(RTFTOKLIB) \
    $(DOCTOKLIB)

SHL1LIBS=\
    $(SLB)$/debugservices_xxml.lib \
    $(SLB)$/debugservices_rtftok.lib \
    $(SLB)$/debugservices_odsl.lib \
    $(SLB)$/debugservices_rtfsl.lib \
    $(SLB)$/debugservices_doctok.lib

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS = $(SLO)$/component.obj

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

