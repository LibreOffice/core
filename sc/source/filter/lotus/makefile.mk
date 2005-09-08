#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 19:42:27 $
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

PRJNAME=sc
TARGET=lotus

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES =				\
        filter.cxx		\
        lotus.cxx		\
        lotimpop.cxx	\
        lotread.cxx		\
        lotform.cxx		\
        memory.cxx		\
        op.cxx			\
        optab.cxx		\
        tool.cxx		\
        expop.cxx		\
        export.cxx		\
        lotattr.cxx



SLOFILES =					\
        $(SLO)$/filter.obj	\
        $(SLO)$/lotus.obj	\
        $(SLO)$/lotimpop.obj	\
        $(SLO)$/lotread.obj	\
        $(SLO)$/lotform.obj	\
        $(SLO)$/memory.obj	\
        $(SLO)$/op.obj		\
        $(SLO)$/optab.obj	\
        $(SLO)$/tool.obj		\
        $(SLO)$/expop.obj	\
        $(SLO)$/export.obj	\
        $(SLO)$/lotattr.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

