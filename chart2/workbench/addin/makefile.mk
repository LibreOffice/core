#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:59:10 $
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
PRJNAME=chartaddin

TARGET=chartsampleaddin

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

# comprehensive type info, so rdb needn't be installed
# CPPUMAKERFLAGS*=-C

# UNOTYPES=\
# 	com.sun.star.lang.XInitialization \
# 	com.sun.star.lang.XServiceName \
# 	com.sun.star.lang.XServiceInfo \
# 	com.sun.star.util.XRefreshable \
# 	com.sun.star.lang.XLocalizable  \
# 	com.sun.star.chart.XDiagram \
# 	com.sun.star.chart.XChartDocument \
# 	com.sun.star.chart.XAxisXSupplier \
# 	com.sun.star.chart.XAxisYSupplier \
# 	com.sun.star.chart.XStatisticDisplay \
# 	com.sun.star.lang.XMultiServiceFactory

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/sampleaddin.obj

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB) 

SHL1DEPN=makefile.mk
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk
