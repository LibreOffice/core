#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2005-10-25 10:45:39 $
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
TARGET =schema
PACKAGE=org$/openoffice$/helper

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JARFILES = jaxp.jar parser.jar

JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/DefaultNamespaceRemover.class \
    $(CLASSDIR)$/$(PACKAGE)$/Validator.class \
    $(CLASSDIR)$/$(PACKAGE)$/PrettyPrinter.class

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

