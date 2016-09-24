#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

# comprehensive type info, so rdb need not to be installed
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
        $(SALLIB)

SHL1DEPN=makefile.mk
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk
