#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 09:42:59 $
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
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=config

ENABLE_EXCEPTIONS=TRUE

# --- Settings common for the whole project -----

#UNOTYPES=

UNOTYPES+= com.sun.star.configuration.XTemplateContainer \
            com.sun.star.util.XChangesListener  \
            com.sun.star.util.XChangesNotifier  \
            com.sun.star.util.XStringEscape     \
            com.sun.star.util.XChangesBatch     \
            com.sun.star.uno.XWeak              \
            com.sun.star.uno.XAggregation

.INCLUDE : $(PRJINC)$/unotools$/unotools.mk

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk


# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/configvaluecontainer.obj \
        $(SLO)$/confignode.obj \
        $(SLO)$/configitem.obj \
        $(SLO)$/configmgr.obj  \
        $(SLO)$/configpathes.obj  \
        $(SLO)$/bootstrap.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

