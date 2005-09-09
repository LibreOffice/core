#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 04:44:08 $
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
TARGET=swg

#ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------
SLOFILES =  \
        $(SLO)$/rdcont.obj \
        $(SLO)$/rdflds.obj \
        $(SLO)$/rdfmts.obj \
        $(SLO)$/rdhnt.obj \
        $(SLO)$/rdmisc.obj \
        $(SLO)$/rdnds.obj \
        $(SLO)$/rdnum.obj \
        $(SLO)$/rdpage.obj \
        $(SLO)$/rdswg.obj \
        $(SLO)$/rdtox.obj \
        $(SLO)$/swgpar.obj \
        $(SLO)$/swgstr.obj \
                $(SLO)$/sw2block.obj \
        $(SLO)$/swblocks.obj	\
        $(SLO)$/SwXMLBlockImport.obj	\
        $(SLO)$/SwXMLSectionList.obj	\
        $(SLO)$/SwXMLBlockExport.obj	\
        $(SLO)$/SwXMLBlockListContext.obj	\
        $(SLO)$/SwXMLTextBlocks.obj \
        $(SLO)$/SwXMLTextBlocks1.obj

EXCEPTIONSFILES = \
        $(SLO)$/SwXMLBlockImport.obj	\
        $(SLO)$/SwXMLSectionList.obj	\
        $(SLO)$/SwXMLBlockExport.obj	\
        $(SLO)$/SwXMLBlockListContext.obj	\
        $(SLO)$/SwXMLTextBlocks.obj \
        $(SLO)$/SwXMLTextBlocks1.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

