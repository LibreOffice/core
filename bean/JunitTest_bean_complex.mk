#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_JunitTest_JunitTest,bean_complex))

$(eval $(call gb_JunitTest_use_jars,bean_complex,\
	OOoRunner \
	ridl \
	test \
	unoil \
	officebean \
))

$(eval $(call gb_JunitTest_add_sourcefiles,bean_complex,\
	bean/qa/complex/bean/OOoBeanTest \
	bean/qa/complex/bean/ScreenComparer \
	bean/qa/complex/bean/WriterFrame \
))

$(eval $(call gb_JunitTest_add_classes,bean_complex,\
	complex.bean.OOoBeanTest \
))
