# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
