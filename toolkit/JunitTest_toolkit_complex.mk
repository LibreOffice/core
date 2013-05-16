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

$(eval $(call gb_JunitTest_JunitTest,toolkit_complex))

$(eval $(call gb_JunitTest_use_jars,toolkit_complex,\
    OOoRunner \
    ridl \
    test \
    unoil \
))

$(eval $(call gb_JunitTest_add_sourcefiles,toolkit_complex,\
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleComponent \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleContext \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleEventBroadcaster \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleExtendedComponent \
    toolkit/qa/complex/toolkit/accessibility/_XAccessibleText \
    toolkit/qa/complex/toolkit/Assert \
    toolkit/qa/complex/toolkit/awtgrid/GridDataListener \
    toolkit/qa/complex/toolkit/awtgrid/TMutableGridDataModel \
    toolkit/qa/complex/toolkit/awtgrid/DummyColumn \
    toolkit/qa/complex/toolkit/GridControl \
    toolkit/qa/complex/toolkit/UnitConversion \
))

$(eval $(call gb_JunitTest_add_classes,toolkit_complex,\
    complex.toolkit.GridControl \
    complex.toolkit.UnitConversion \
))

# vim: set noet sw=4 ts=4:
