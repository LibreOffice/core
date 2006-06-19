#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hr $ $Date: 2006-06-19 23:50:35 $
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

PRJNAME=bridges
TARGET=bridges_remote_static
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

SLOFILES= 	\
        $(SLO)$/proxy.obj \
        $(SLO)$/stub.obj \
        $(SLO)$/remote.obj \
        $(SLO)$/mapping.obj \
        $(SLO)$/helper.obj \
        $(SLO)$/remote_types.obj

# Forte6 update 1 on Solaris Intel dies with internal compiler error
# on stub.cxx if optimization is on. Switch it off for now.
# To be reevaluated on compiler upgrade
.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES=\
        $(SLO)$/stub.obj
.ENDIF        

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


