#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Module_Module,swext))

ifeq ($(ENABLE_MEDIAWIKI),TRUE)
$(eval $(call gb_Module_add_targets,swext,\
	Jar_mediawiki \
	Configuration_mediawiki \
	Extension_wiki-publisher \
))
endif
