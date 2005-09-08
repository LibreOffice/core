#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 03:15:46 $
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
PRJINC=$(PRJ)$/source$/inc
PRJNAME=configmgr
TARGET=api2

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

SLOFILES=	\
        $(SLO)$/broadcaster.obj		\
        $(SLO)$/listenercontainer.obj		\
        $(SLO)$/provider.obj		\
        $(SLO)$/providerimpl.obj	\
        $(SLO)$/confprovider2.obj	\
        $(SLO)$/confproviderimpl2.obj \
        $(SLO)$/accessimpl.obj		\
        $(SLO)$/apiaccessobj.obj	\
        $(SLO)$/apiserviceinfo.obj	\
        $(SLO)$/apifactory.obj	\
        $(SLO)$/apifactoryimpl.obj	\
        $(SLO)$/apinodeaccess.obj	\
        $(SLO)$/apinodeupdate.obj	\
        $(SLO)$/apinotifierimpl.obj	\
        $(SLO)$/apitreeaccess.obj	\
        $(SLO)$/apitreeimplobj.obj	\
        $(SLO)$/confignotifier.obj	\
        $(SLO)$/committer.obj	\
        $(SLO)$/elementaccess.obj	\
        $(SLO)$/elementimpl.obj	\
        $(SLO)$/groupaccess.obj	\
        $(SLO)$/groupobjects.obj	\
        $(SLO)$/groupupdate.obj	\
        $(SLO)$/propertiesfilterednotifier.obj	\
        $(SLO)$/propertyinfohelper.obj	\
        $(SLO)$/propertysetaccess.obj	\
        $(SLO)$/propsetaccessimpl.obj	\
        $(SLO)$/setaccess.obj		\
        $(SLO)$/setobjects.obj		\
        $(SLO)$/setupdate.obj		\
        $(SLO)$/translatechanges.obj	\
        $(SLO)$/treeiterators.obj \
        $(SLO)$/updateimpl.obj		\


# --- Targets ----------------------------------

.INCLUDE : target.mk

