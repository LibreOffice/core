
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-11-11 12:07:27 $
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

PRJNAME=			framework
TARGET=				fwk_uiconfiguration
USE_DEFFILE=		TRUE
NO_BSYMBOLIC=		TRUE
ENABLE_EXCEPTIONS=	TRUE
BOOTSTRAP_SERVICE=	FALSE

# --- Settings -----------------------------------------------------

.INCLUDE :  		svpre.mk
.INCLUDE :  		settings.mk
.INCLUDE :  		sv.mk

# --- Generate -----------------------------------------------------

SLOFILES=			\
                    $(SLO)$/uiconfigurationmanager.obj          \
                    $(SLO)$/moduleuiconfigurationmanager.obj    \
                    $(SLO)$/moduleuicfgsupplier.obj             \
                    $(SLO)$/windowstateconfiguration.obj        \
                    $(SLO)$/moduleimagemanager.obj              \
                    $(SLO)$/imagemanager.obj                    \
                    $(SLO)$/graphicnameaccess.obj               \
                    $(SLO)$/uicategorydescription.obj           \
                    $(SLO)$/globalsettings.obj

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk
