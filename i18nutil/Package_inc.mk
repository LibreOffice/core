#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,i18nutil_inc,$(SRCDIR)/i18nutil/inc))

$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/i18nutildllapi.h,i18nutil/i18nutildllapi.h))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/casefolding.hxx,i18nutil/casefolding.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/oneToOneMapping.hxx,i18nutil/oneToOneMapping.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/paper.hxx,i18nutil/paper.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/scripttypedetector.hxx,i18nutil/scripttypedetector.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/unicode.hxx,i18nutil/unicode.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/widthfolding.hxx,i18nutil/widthfolding.hxx))

# vim: set noet sw=4:
