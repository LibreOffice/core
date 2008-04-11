#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.15 $
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
#*************************************************************************
PRJ=..$/..

PRJNAME=			framework
TARGET=				fwk_dispatch
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=framework

# --- Generate -----------------------------------------------------

SLOFILES=			$(SLO)$/interceptionhelper.obj				\
                    $(SLO)$/dispatchprovider.obj				\
                    $(SLO)$/dispatchinformationprovider.obj	    \
                    $(SLO)$/loaddispatcher.obj                  \
                    $(SLO)$/menudispatcher.obj					\
                    $(SLO)$/helpagentdispatcher.obj				\
                    $(SLO)$/mailtodispatcher.obj				\
                    $(SLO)$/interaction.obj						\
                    $(SLO)$/servicehandler.obj                  \
                    $(SLO)$/closedispatcher.obj                 \
                    $(SLO)$/systemexec.obj						\
                    $(SLO)$/popupmenudispatcher.obj             \
                    $(SLO)$/windowcommanddispatch.obj

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk
