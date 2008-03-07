#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.42 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 12:09:11 $
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
PRJNAME=svtools
TARGET=misc

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

#use local "bmp" as it may not yet be delivered

SRS1NAME=misc
SRC1FILES=\
    config.src	\
    iniman.src 	\
    ehdl.src \
    imagemgr.src      \
    helpagent.src \
    langtab.src 

SRS2NAME=ehdl
SRC2FILES=\
    errtxt.src

SLOFILES=\
    $(SLO)$/acceleratorexecute.obj  \
    $(SLO)$/cliplistener.obj        \
    $(SLO)$/embedhlp.obj            \
    $(SLO)$/embedtransfer.obj       \
    $(SLO)$/imagemgr.obj            \
    $(SLO)$/imageresourceaccess.obj \
    $(SLO)$/templatefoldercache.obj \
    $(SLO)$/transfer.obj            \
    $(SLO)$/transfer2.obj           \
    $(SLO)$/stringtransfer.obj      \
    $(SLO)$/urihelper.obj           \
    $(SLO)$/svtaccessiblefactory.obj \
    $(SLO)$/ehdl.obj                \
    $(SLO)$/flbytes.obj             \
    $(SLO)$/helpagentwindow.obj     \
    $(SLO)$/imap.obj                \
    $(SLO)$/imap2.obj               \
    $(SLO)$/imap3.obj               \
    $(SLO)$/ownlist.obj             \
    $(SLO)$/vcldata.obj             \
    $(SLO)$/restrictedpaths.obj     \
    $(SLO)$/dialogclosedlistener.obj\
    $(SLO)$/dialogcontrolling.obj   \
    $(SLO)$/sharecontrolfile.obj   \
    $(SLO)$/langtab.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

