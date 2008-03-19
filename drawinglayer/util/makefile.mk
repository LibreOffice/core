#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: aw $ $Date: 2008-03-19 04:37:08 $
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

PRJNAME=drawinglayer
TARGET=drawinglayer

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/drawinglayer.lib
LIB1FILES=\
    $(SLB)$/primitive2d.lib	\
    $(SLB)$/primitive3d.lib	\
    $(SLB)$/geometry.lib	\
    $(SLB)$/processor2d.lib	\
    $(SLB)$/processor3d.lib	\
    $(SLB)$/attribute.lib	\
    $(SLB)$/animation.lib	\
    $(SLB)$/texture.lib

SHL1TARGET= drawinglayer$(DLLPOSTFIX)
SHL1IMPLIB= idrawinglayer

SHL1STDLIBS=\
        $(GOODIESLIB)		\
        $(VCLLIB)			\
        $(BASEGFXLIB)		\
        $(TOOLSLIB)			\
        $(SVLLIB)			\
        $(TKLIB)			\
        $(CANVASTOOLSLIB)	\
        $(SVTOOLLIB)		\
        $(SALLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(AVMEDIALIB)		\
        $(COMPHELPERLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/drawinglayer.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=DrawingLayer
DEFLIB1NAME	=drawinglayer

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @+$(TYPE) $(TARGET).flt > $@

