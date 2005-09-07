#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:44:52 $
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

PRJ		=..$/..$/..$/..$/..$/..
PRJNAME = juhelper
PACKAGE = com$/sun$/star$/comp$/helper
TARGET  = com_sun_star_comp_helper_test

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = jurt.jar ridl.jar

JAVACLASSFILES=	\
    $(CLASSDIR)$/$(PACKAGE)$/ComponentContext_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/SharedLibraryLoader_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/RegistryServiceFactory_Test.class \
    $(CLASSDIR)$/$(PACKAGE)$/Bootstrap_Test.class

JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

