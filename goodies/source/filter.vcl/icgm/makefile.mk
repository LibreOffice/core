#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 02:53:03 $
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

PRJ=..$/..$/..
PRJNAME=goodies
TARGET=icgm
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Uno III Header -----------------------------------------------

# Every source directory generates headers in a own output directory to
# enable parallel building of the source directories within a project!
UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

UNOTYPES=\
        com.sun.star.uno.TypeClass \
        com.sun.star.uno.XInterface \
        com.sun.star.lang.XMultiServiceFactory\
        com.sun.star.drawing.XMasterPageTarget\
        com.sun.star.drawing.FillStyle\
        com.sun.star.drawing.XMasterPageTarget\
        com.sun.star.drawing.PointSequence\
        com.sun.star.drawing.CircleKind\
        com.sun.star.drawing.PointSequenceSequence\
        com.sun.star.drawing.LineStyle\
        com.sun.star.drawing.LineDash\
        com.sun.star.drawing.Hatch\
        com.sun.star.drawing.MeasureTextVertPos\
        com.sun.star.drawing.XMasterPagesSupplier\
        com.sun.star.drawing.XDrawPage\
        com.sun.star.drawing.XDrawPages\
        com.sun.star.drawing.XDrawPagesSupplier\
        com.sun.star.drawing.XShapeGrouper\
        com.sun.star.drawing.PolyPolygonBezierCoords\
        com.sun.star.drawing.FlagSequence\
        com.sun.star.drawing.TextAdjust\
        com.sun.star.frame.XModel\
        com.sun.star.task.XStatusIndicator\
        com.sun.star.beans.XPropertySet\
        com.sun.star.awt.Gradient\
        com.sun.star.awt.FontDescriptor\
        com.sun.star.awt.FontSlant\
        com.sun.star.awt.FontWeight\
        com.sun.star.awt.FontUnderline\
        com.sun.star.awt.XBitmap\
        com.sun.star.awt.XPointer\
        com.sun.star.text.XText\
        com.sun.star.style.HorizontalAlignment

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/cgm.obj		\
            $(SLO)$/chart.obj	\
            $(SLO)$/class0.obj	\
            $(SLO)$/class1.obj	\
            $(SLO)$/class2.obj	\
            $(SLO)$/class3.obj	\
            $(SLO)$/class4.obj	\
            $(SLO)$/class5.obj	\
            $(SLO)$/class7.obj	\
            $(SLO)$/classx.obj	\
            $(SLO)$/outact.obj	\
            $(SLO)$/actmeta.obj	\
            $(SLO)$/actimpr.obj	\
            $(SLO)$/bundles.obj	\
            $(SLO)$/bitmap.obj	\
            $(SLO)$/elements.obj
#			$(SLO)$/svdem.obj

SHL1TARGET	=	icg$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB	=	icgm
SHL1DEPN	=	$(LB)$/icgm.lib
SHL1VERSIONMAP=exports.map
SHL1DEF		=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS	=	$(SLB)$/icgm.lib

DEF1NAME=$(SHL1TARGET)

.IF "$(GUI)"=="OS2"
SHL1OBJS	=	$(SLO)$/class0.obj
.ENDIF

SHL1STDLIBS = \
            $(TKLIB)		\
            $(VCLLIB)		\
            $(UNOTOOLSLIB)	\
            $(TOOLSLIB)		\
            $(CPPULIB)		\
            $(SALLIB)

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

