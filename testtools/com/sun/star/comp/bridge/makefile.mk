#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 14:44:16 $
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

PRJ=..$/..$/..$/..$/..
PRJNAME=testtools
PACKAGE=com$/sun$/star$/comp$/bridge
TARGET=com_sun_star_comp_bridge


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------


JARFILES 		= ridl.jar jurt.jar juh.jar


JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/CurrentContextChecker.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponent.class \
    $(CLASSDIR)$/$(PACKAGE)$/TestComponentMain.class

JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 


JARCLASSDIRS	= $(PACKAGE) test$/testtools$/bridgetest
JARTARGET		= testComponent.jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


