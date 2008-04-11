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
# $Revision: 1.28 $
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
TARGET=				fwk_services
USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- Generate -----------------------------------------------------

SLOFILES=			\
                    $(SLO)$/desktop.obj								\
                    $(SLO)$/frame.obj								\
                    $(SLO)$/urltransformer.obj						\
                    $(SLO)$/mediatypedetectionhelper.obj			\
                    $(SLO)$/substitutepathvars.obj					\
                    $(SLO)$/pathsettings.obj                        \
                    $(SLO)$/backingcomp.obj							\
                    $(SLO)$/backingwindow.obj						\
                    $(SLO)$/dispatchhelper.obj                      \
                    $(SLO)$/license.obj                      		\
                    $(SLO)$/modulemanager.obj                       \
                    $(SLO)$/autorecovery.obj                        \
                    $(SLO)$/sessionlistener.obj                     \
                    $(SLO)$/taskcreatorsrv.obj						\
                    $(SLO)$/uriabbreviation.obj

SRS1NAME=$(TARGET)
SRC1FILES= fwk_services.src

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk

