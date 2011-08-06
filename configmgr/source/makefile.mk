#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

PRJ = ..
PRJNAME = configmgr
TARGET = configmgr

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

DLLPRE =

SLOFILES = \
    $(SLO)/access.obj \
    $(SLO)/broadcaster.obj \
    $(SLO)/childaccess.obj \
    $(SLO)/components.obj \
    $(SLO)/configurationprovider.obj \
    $(SLO)/configurationregistry.obj \
    $(SLO)/data.obj \
    $(SLO)/defaultprovider.obj \
    $(SLO)/groupnode.obj \
    $(SLO)/localizedpropertynode.obj \
    $(SLO)/localizedvaluenode.obj \
    $(SLO)/lock.obj \
    $(SLO)/modifications.obj \
    $(SLO)/node.obj \
    $(SLO)/nodemap.obj \
    $(SLO)/parsemanager.obj \
    $(SLO)/partial.obj \
    $(SLO)/propertynode.obj \
    $(SLO)/rootaccess.obj \
    $(SLO)/services.obj \
    $(SLO)/setnode.obj \
    $(SLO)/type.obj \
    $(SLO)/update.obj \
    $(SLO)/valueparser.obj \
    $(SLO)/writemodfile.obj \
    $(SLO)/xcdparser.obj \
    $(SLO)/xcsparser.obj \
    $(SLO)/xcuparser.obj \
    $(SLO)/xmldata.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALHELPERLIB) \
    $(SALLIB) \
    $(XMLREADERLIB)
SHL1TARGET = configmgr.uno
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR : $(MISC)/configmgr.component

$(MISC)/configmgr.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        configmgr.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt configmgr.component
