#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 11:51:03 $
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


ENABLE_EXCEPTIONS=true

PRJNAME=vcl
TARGET=svpplug

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(listening)"!="" || "$(LISTENING)"!=""
CDEFS+= -DWITH_SVP_LISTENING
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)" == "unx"

SLOFILES=\
        $(SLO)$/svpinst.obj\
        $(SLO)$/svpdummies.obj\
        $(SLO)$/svpframe.obj\
        $(SLO)$/svpgdi.obj\
        $(SLO)$/svptext.obj\
        $(SLO)$/svpprn.obj\
        $(SLO)$/svppspgraphics.obj\
        $(SLO)$/svpvd.obj\
        $(SLO)$/svpbmp.obj\
        $(SLO)$/svpelement.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


