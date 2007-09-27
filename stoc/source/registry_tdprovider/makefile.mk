#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 12:59:57 $
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

PRJNAME= stoc
TARGET = regtypeprov
ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=TRUE
UNOUCROUT= $(OUT)$/inc$/bootstrap

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

SLOFILES=	\
        $(SLO)$/tdprovider.obj	\
        $(SLO)$/td.obj		\
        $(SLO)$/tdef.obj	\
        $(SLO)$/tdenum.obj	\
        $(SLO)$/tdcomp.obj	\
        $(SLO)$/tdconsts.obj	\
        $(SLO)$/tdiface.obj \
        $(SLO)$/tdmodule.obj \
        $(SLO)$/tdprop.obj \
        $(SLO)$/tdservice.obj \
        $(SLO)$/tdsingleton.obj \
        $(SLO)$/rdbtdp_tdenumeration.obj \
        $(SLO)$/functiondescription.obj \
        $(SLO)$/methoddescription.obj \
        $(SLO)$/structtypedescription.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

