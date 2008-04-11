#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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


