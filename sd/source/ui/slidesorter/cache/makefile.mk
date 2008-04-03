#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 14:21:51 $
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

PRJ=..$/..$/..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=slscache
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
PRJINC=..$/..

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =      							\
    $(SLO)$/SlsBitmapCache.obj				\
    $(SLO)$/SlsBitmapCompressor.obj			\
    $(SLO)$/SlsBitmapFactory.obj			\
    $(SLO)$/SlsCacheCompactor.obj			\
    $(SLO)$/SlsCacheConfiguration.obj		\
    $(SLO)$/SlsGenericPageCache.obj			\
    $(SLO)$/SlsIdleDetector.obj				\
    $(SLO)$/SlsPageCache.obj				\
    $(SLO)$/SlsPageCacheManager.obj			\
    $(SLO)$/SlsQueueProcessor.obj			\
    $(SLO)$/SlsRequestFactory.obj			\
    $(SLO)$/SlsRequestQueue.obj				


EXCEPTIONSFILES= 

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

