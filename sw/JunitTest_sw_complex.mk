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

$(eval $(call gb_JunitTest_JunitTest,sw_complex))

$(eval $(call gb_JunitTest_set_defs,sw_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sw/qa/complex/writer/testdocuments \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sw_complex,\
    sw/qa/complex/accessibility/AccessibleRelationSet \
    sw/qa/complex/indeterminateState/CheckIndeterminateState \
    sw/qa/complex/writer/CheckBookmarks \
    sw/qa/complex/writer/TestDocument \
    sw/qa/complex/writer/TextPortionEnumerationTest \
))

$(eval $(call gb_JunitTest_use_jars,sw_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
    jurt \
))

$(eval $(call gb_JunitTest_add_classes,sw_complex,\
    complex.accessibility.AccessibleRelationSet \
    complex.writer.CheckBookmarks \
    complex.writer.TextPortionEnumerationTest \
))

# vim: set noet sw=4 ts=4:
