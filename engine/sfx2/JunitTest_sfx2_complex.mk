# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
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

$(eval $(call gb_JunitTest_JunitTest,sfx2_complex))

$(eval $(call gb_JunitTest_set_defs,sfx2_complex,\
    $$(DEFS) \
    -Dorg.openoffice.test.arg.tdoc=$(SRCDIR)/sfx2/qa/complex/sfx2/testdocuments \
))

$(eval $(call gb_JunitTest_use_unoapi_jars,sfx2_complex))

$(eval $(call gb_JunitTest_use_jars,sfx2_complex,\
    test-tools \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sfx2_complex,\
    sfx2/qa/complex/sfx2/GlobalEventBroadcaster \
    sfx2/qa/complex/sfx2/DocumentEvents \
    sfx2/qa/complex/sfx2/tools/WriterHelper \
))

$(eval $(call gb_JunitTest_add_classes,sfx2_complex,\
    complex.sfx2.DocumentEvents \
))
# fd#35663 fails currently:
#    complex.sfx2.UndoManager \
# #i115674# fails currently: misses some OnUnfocus event
#	complex.sfx2.GlobalEventBroadcaster \

# vim: set noet sw=4 ts=4:
