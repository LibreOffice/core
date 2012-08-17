# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_JunitTest_JunitTest,svl_complex))

$(eval $(call gb_JunitTest_set_defs,svl_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/svl/qa/complex/broken_document/test_documents \
))

$(eval $(call gb_JunitTest_use_jars,svl_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
    ConnectivityTools \
))

$(eval $(call gb_JunitTest_add_sourcefiles,svl_complex,\
    svl/qa/complex/ConfigItems/CheckConfigItems \
    svl/qa/complex/passwordcontainer/PasswordContainerUnitTest \
    svl/qa/complex/passwordcontainer/TestHelper \
    svl/qa/complex/passwordcontainer/Test03 \
    svl/qa/complex/passwordcontainer/Test02 \
    svl/qa/complex/passwordcontainer/Test01 \
    svl/qa/complex/passwordcontainer/PasswordContainerTest \
    svl/qa/complex/passwordcontainer/MasterPasswdHandler \
))

$(eval $(call gb_JunitTest_add_classes,svl_complex,\
    complex.passwordcontainer.PasswordContainerUnitTest \
))

# vim: set noet sw=4 ts=4:
