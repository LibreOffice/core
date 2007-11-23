#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: ihi $ $Date: 2007-11-23 13:10:01 $
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

PRJ := ..$/..$/..$/..$/..$/..$/..$/..
PRJNAME := bridges
TARGET := test_com_sun_star_lib_uno_bridges_javaremote

PACKAGE := com$/sun$/star$/lib$/uno$/bridges$/javaremote
JAVATESTFILES := \
    Bug51323_Test.java \
    Bug92174_Test.java \
    Bug97697_Test.java \
    Bug98508_Test.java \
    Bug107753_Test.java \
    Bug108825_Test.java \
    Bug110892_Test.java \
    Bug111153_Test.java \
    Bug114133_Test.java \
    MethodIdTest.java \
    PolyStructTest.java \
    StopMessageDispatcherTest.java
IDLTESTFILES := \
    Bug98508_Test.idl \
    PolyStructTest.idl
JARFILES := juh.jar jurt.jar ridl.jar

.INCLUDE: javaunittest.mk
