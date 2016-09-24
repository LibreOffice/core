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

$(eval $(call gb_JunitTest_JunitTest,filter_complex))

$(eval $(call gb_JunitTest_use_jars,filter_complex,\
	OOoRunner \
	ridl \
	test \
	unoil \
	jurt \
))

$(eval $(call gb_JunitTest_add_sourcefiles,filter_complex,\
	filter/qa/complex/filter/misc/FinalizedMandatoryTest \
	filter/qa/complex/filter/misc/TypeDetection6FileFormat \
))

$(eval $(call gb_JunitTest_add_classes,filter_complex,\
	complex.filter.misc.FinalizedMandatoryTest \
	complex.filter.misc.TypeDetection6FileFormat \
))

# vim: set noet sw=4 ts=4:
