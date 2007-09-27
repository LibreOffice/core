#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 08:40:47 $
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

PRJNAME=sw
TARGET=docnode

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=        \
        $(SLO)$/cancellablejob.obj \
        $(SLO)$/finalthreadmanager.obj \
        $(SLO)$/ndtbl.obj \
        $(SLO)$/node.obj \
        $(SLO)$/observablethread.obj \
        $(SLO)$/retrieveinputstream.obj \
        $(SLO)$/retrievedinputstreamdata.obj \
        $(SLO)$/swbaslnk.obj \
        $(SLO)$/threadmanager.obj

SLOFILES =	\
        $(SLO)$/ndindex.obj \
        $(SLO)$/ndcopy.obj \
        $(SLO)$/ndnotxt.obj \
        $(SLO)$/ndnum.obj \
        $(SLO)$/ndsect.obj \
        $(SLO)$/ndtbl.obj \
        $(SLO)$/ndtbl1.obj \
        $(SLO)$/node.obj \
        $(SLO)$/node2lay.obj \
        $(SLO)$/nodes.obj \
        $(SLO)$/section.obj \
        $(SLO)$/swbaslnk.obj \
        $(SLO)$/retrievedinputstreamdata.obj \
        $(SLO)$/observablethread.obj \
        $(SLO)$/cancellablejob.obj \
        $(SLO)$/retrieveinputstream.obj \
        $(SLO)$/retrieveinputstreamconsumer.obj \
        $(SLO)$/threadlistener.obj \
        $(SLO)$/threadmanager.obj \
        $(SLO)$/swthreadmanager.obj \
        $(SLO)$/swthreadjoiner.obj \
        $(SLO)$/pausethreadstarting.obj \
        $(SLO)$/finalthreadmanager.obj


# --- Targets -------------------------------------------------------

.INCLUDE :	target.mk

