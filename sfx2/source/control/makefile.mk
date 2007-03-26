#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 15:04:02 $
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

PRJNAME=sfx2
TARGET=control
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
CFLAGS+=-Od
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/bindings.obj \
    $(SLO)$/ctrlitem.obj \
    $(SLO)$/dispatch.obj \
    $(SLO)$/macrconf.obj \
        $(SLO)$/macro.obj \
    $(SLO)$/minfitem.obj \
    $(SLO)$/msg.obj \
       $(SLO)$/msgpool.obj \
       $(SLO)$/objface.obj \
    $(SLO)$/querystatus.obj \
    $(SLO)$/request.obj \
        $(SLO)$/sfxstatuslistener.obj \
    $(SLO)$/shell.obj \
    $(SLO)$/sorgitm.obj \
    $(SLO)$/srchitem.obj \
    $(SLO)$/statcach.obj \
        $(SLO)$/unoctitm.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

