#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.35 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 17:21:40 $
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
TARGET=				fwk_classes
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- Generate -----------------------------------------------------

SLOFILES=           \
                    $(SLO)$/propertysethelper.obj					\
                    $(SLO)$/framecontainer.obj						\
                    $(SLO)$/taskcreator.obj							\
                    $(SLO)$/targetfinder.obj						\
                    $(SLO)$/menumanager.obj							\
                    $(SLO)$/bmkmenu.obj								\
                    $(SLO)$/droptargetlistener.obj					\
                    $(SLO)$/converter.obj							\
                    $(SLO)$/actiontriggerpropertyset.obj			\
                    $(SLO)$/actiontriggerseparatorpropertyset.obj	\
                    $(SLO)$/actiontriggercontainer.obj				\
                    $(SLO)$/imagewrapper.obj						\
                    $(SLO)$/rootactiontriggercontainer.obj          \
                    $(SLO)$/protocolhandlercache.obj				\
                    $(SLO)$/addonmenu.obj							\
                    $(SLO)$/addonsoptions.obj						\
                    $(SLO)$/fwkresid.obj                            \
                    $(SLO)$/colorlistener.obj                       \
                    $(SLO)$/framelistanalyzer.obj                   \
                    $(SLO)$/sfxhelperfunctions.obj                  \
                    $(SLO)$/menuextensionsupplier.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
    resource.src

# --- Targets ------------------------------------------------------

.INCLUDE :			target.mk

