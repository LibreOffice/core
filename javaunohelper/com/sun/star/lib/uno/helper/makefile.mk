#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:41:32 $
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

PRJ=..$/..$/..$/..$/..$/..

PRJNAME = juhelper
PACKAGE = com$/sun$/star$/lib$/uno$/helper
TARGET  = com_sun_star_lib_uno_helper


# --- Settings -----------------------------------------------------
.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/util$/settings.pmk

# Files --------------------------------------------------------
JAVACLASSFILES= \
    $(CLASSDIR)$/$(PACKAGE)$/WeakBase.class 	\
        $(CLASSDIR)$/$(PACKAGE)$/WeakAdapter.class  	\
    $(CLASSDIR)$/$(PACKAGE)$/ComponentBase.class		\
    $(CLASSDIR)$/$(PACKAGE)$/InterfaceContainer.class	\
    $(CLASSDIR)$/$(PACKAGE)$/MultiTypeInterfaceContainer.class \
        $(CLASSDIR)$/$(PACKAGE)$/PropertySet.class	\
    $(CLASSDIR)$/$(PACKAGE)$/UnoUrl.class		\
    $(CLASSDIR)$/$(PACKAGE)$/Factory.class
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

