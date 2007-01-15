#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: vg $ $Date: 2007-01-15 12:35:32 $
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

PRJNAME=officecfg
TARGET =cfgimport
PACKAGE=org$/openoffice$/configuration

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JARFILES = xml-apis.jar xercesImpl.jar

.IF "$(SYSTEM_XT)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XT_JAR)
.ELSE
JARFILES += xt.jar
.ENDIF

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/XMLDefaultGenerator.class \
    $(CLASSDIR)$/$(PACKAGE)$/Generator.class \
    $(CLASSDIR)$/$(PACKAGE)$/Trim.class \
    $(CLASSDIR)$/$(PACKAGE)$/Decoder.class \
    $(CLASSDIR)$/$(PACKAGE)$/FileHelper.class \
    $(CLASSDIR)$/$(PACKAGE)$/Inspector.class 

JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

RC_SUBDIRSDEPS=$(JAVATARGET)

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS     = TRUE


# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

.IF "$(JARTARGETN)"!=""
$(JARTARGETN) : $(JAVACLASSFILES) $(JAVATARGET)
.ENDIF          # "$(JARTARGETN)"!=""

