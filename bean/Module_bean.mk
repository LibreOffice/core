#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Module_Module,bean))

ifneq ($(ENABLE_JAVA),)

ifneq ($(OS),MACOSX)
ifneq ($(OS),ANDROID)
$(eval $(call gb_Module_add_targets,bean,\
	Jar_officebean \
	Library_officebean \
))

# complex tests compileable but fail at runtime
#$(eval $(call gb_Module_add_subsequentcheck_targets,bean,\
#	JunitTest_bean_complex \
#))
endif
endif

endif
