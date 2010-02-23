#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ		= ..$/..$/..$/..
PRJNAME = jurt
PACKAGE = com$/sun$/star$/uno
TARGET  = com_sun_star_uno

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

JAVAFILES = \
    AnyConverter.java \
    Ascii.java \
    AsciiString.java \
    MappingException.java \
    WeakReference.java

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

doc:
    pwd
    cd $(PRJ) && javadoc -sourcepath /usr/local/java/src:unxlngi3.pro/japi:.:../jlibs  com.sun.star.lib.util com.sun.star.uno com.sun.star.lib.uno.typeinfo com.sun.star.lib.uno.environments.java com.sun.star.lib.uno.environments.remote com.sun.star.lib.uno.protocols.iiop com.sun.star.lib.uno.bridges.java_remote com.sun.star.comp.loader com.sun.star.comp.connections -d unxlngi3.pro/doc 
