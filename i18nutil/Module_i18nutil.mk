#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,i18nutil))

$(eval $(call gb_Module_add_targets,i18nutil,\
	Library_i18nutil \
))

$(eval $(call gb_Module_add_check_targets,i18nutil,\
	CppunitTest_i18nutil \
))

# vim: set noet sw=4:
