#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2007-09-20 15:59:01 $
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

PRJ=..

PRJNAME=soltools
TARGET=soltools_giparser
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

.IF "$(GUI)" == "OS2"
STL_OS2_BUILDING=1
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES=\
    $(OBJ)$/gen_info.obj	\
    $(OBJ)$/gi_list.obj	    \
    $(OBJ)$/gi_parse.obj    \
    $(OBJ)$/st_gilrw.obj

SLOFILES=\
    $(SLO)$/gen_info.obj	\
    $(SLO)$/gi_list.obj	    \
    $(SLO)$/gi_parse.obj    \
    $(SLO)$/st_gilrw.obj

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

