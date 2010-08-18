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

PRJ := ..$/..$/..$/..$/..$/..$/..
PRJNAME := jurt

TARGET := com_sun_star_lib_uno_bridges_java_remote
PACKAGE = com$/sun$/star$/lib$/uno$/bridges$/java_remote

.INCLUDE: $(PRJ)$/util$/makefile.pmk

JAVAFILES = \
    BridgedObject.java \
    ProxyFactory.java \
    RequestHandler.java \
    XConnectionInputStream_Adapter.java \
    XConnectionOutputStream_Adapter.java \
    java_remote_bridge.java

.INCLUDE: target.mk
