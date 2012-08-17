# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_JunitTest_JunitTest,sc_complex))

# sc/qa/complex/calcPreview/ tests do not work autmatically (they request "Press
# any key after resizing" etc.)

$(eval $(call gb_JunitTest_add_classes,sc_complex, \
    complex.cellRanges.CheckXCellRangesQuery \
    complex.dataPilot.CheckDataPilot \
    complex.sc.CalcRTL \
))

$(eval $(call gb_JunitTest_use_jars,sc_complex, \
    OOoRunner \
    jurt \
    ridl \
    test \
    unoil \
))

$(eval $(call gb_JunitTest_add_sourcefiles,sc_complex, \
    sc/qa/complex/cellRanges/CheckXCellRangesQuery \
    sc/qa/complex/dataPilot/CheckDataPilot \
    sc/qa/complex/dataPilot/_XDataPilotDescriptor \
    sc/qa/complex/dataPilot/_XDataPilotTable \
    sc/qa/complex/dataPilot/_XNamed \
    sc/qa/complex/dataPilot/_XPropertySet \
    sc/qa/complex/sc/CalcRTL \
))

$(eval $(call gb_JunitTest_set_defs,sc_complex, \
    $$(DEFS) \
))

# vim: set noet sw=4 ts=4:
