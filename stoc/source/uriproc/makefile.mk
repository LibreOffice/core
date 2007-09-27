#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2007-09-27 13:07:20 $
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
PRJNAME=stoc
TARGET=uriproc
ENABLE_EXCEPTIONS := TRUE

# --- openoffice.org.orig/Settings -----------------------------------------------------

.INCLUDE: settings.mk
DLLPRE = 

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES = \
    $(SLO)$/ExternalUriReferenceTranslator.obj \
    $(SLO)$/UriReference.obj \
    $(SLO)$/UriReferenceFactory.obj \
    $(SLO)$/UriSchemeParser_vndDOTsunDOTstarDOTexpand.obj \
    $(SLO)$/UriSchemeParser_vndDOTsunDOTstarDOTscript.obj \
    $(SLO)$/VndSunStarPkgUrlReferenceFactory.obj \
    $(SLO)$/supportsService.obj

# ------------------------------------------------------------------

.INCLUDE: target.mk
