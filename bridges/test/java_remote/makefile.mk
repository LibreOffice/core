#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ := ..$/..$/..$/..$/..$/..$/..$/..
PRJNAME := bridges
TARGET := test_com_sun_star_lib_uno_bridges_javaremote

PACKAGE := test$/javaremote
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
