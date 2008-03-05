#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:40:38 $
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

PRJNAME=officecfg
TARGET=data_oopenoffice
PACKAGE=org.openoffice

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES= \
    Inet.xcu \
    Setup.xcu \
    VCL.xcu \
    FirstStartWizard.xcu \
    UserProfile.xcu

MODULEFILES= \
    Inet-defaultsearchengine.xcu \
    Setup-writer.xcu   \
    Setup-calc.xcu   \
    Setup-draw.xcu   \
    Setup-impress.xcu   \
    Setup-base.xcu   \
    Setup-math.xcu \
    Setup-report.xcu \
    Setup-start.xcu

LOCALIZEDFILES=Setup.xcu

SAMPLEFILES= \
    LDAP.xcu.sample

.INCLUDE :  target.mk

LANGUAGEPACKS=$(MISC)$/registry$/spool$/Langpack-{$(alllangiso)}.xcu
SAMPLETARGETS=$(MISC)$/registry$/data$/$(PACKAGEDIR)$/{$(SAMPLEFILES)}

ALLTAR : $(LANGUAGEPACKS)
ALLTAR : $(SAMPLETARGETS)

$(SAMPLETARGETS) : $(SAMPLEFILES)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

