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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,i18npool_generated,$(WORKDIR)/i18npool/source))


$(eval $(call gb_Package_add_customtarget,i18npool_generated,i18npool/source/breakiterator,SRCDIR))
$(eval $(call gb_CustomTarget_add_dependencies,i18npool/source/breakiterator,\
	i18npool/source/breakiterator/data/*.txt \
	i18npool/source/breakiterator/data/ja.dic \
	i18npool/source/breakiterator/data/zh.dic \
))
$(eval $(call gb_CustomTarget_add_outdir_dependencies,i18npool/source/breakiterator,\
	$(call gb_Executable_get_target_for_build,gendict) \
))


$(eval $(call gb_Package_add_customtarget,i18npool_generated,i18npool/source/collator,SRCDIR))
$(eval $(call gb_CustomTarget_add_dependencies,i18npool/source/collator,\
	i18npool/source/collator/data/*.txt \
))
$(eval $(call gb_CustomTarget_add_outdir_dependencies,i18npool/source/collator,\
	$(call gb_Executable_get_target_for_build,gencoll_rule) \
))


$(eval $(call gb_Package_add_customtarget,i18npool_generated,i18npool/source/indexentry,SRCDIR))
$(eval $(call gb_CustomTarget_add_dependencies,i18npool/source/indexentry,\
	i18npool/source/indexentry/data/*.txt \
))
$(eval $(call gb_CustomTarget_add_outdir_dependencies,i18npool/source/indexentry,\
	$(call gb_Executable_get_target_for_build,genindex_data) \
))


$(eval $(call gb_Package_add_customtarget,i18npool_generated,i18npool/source/localedata,SRCDIR))
$(eval $(call gb_CustomTarget_add_dependencies,i18npool/source/localedata,\
	i18npool/source/localedata/data/*.xml \
))
$(eval $(call gb_CustomTarget_add_outdir_dependencies,i18npool/source/localedata,\
	$(call gb_Executable_get_target_for_build,saxparser) \
	$(call gb_ComponentTarget_get_outdir_inbuild_target,sax/source/expatwrap/expwrap) \
	$(gb_XSLTPROCTARGET) \
	$(SOLARENV)/bin/packcomponents.xslt \
	$(OUTDIR)/bin/types.rdb \
))


$(eval $(call gb_Package_add_customtarget,i18npool_generated,i18npool/source/textconversion,SRCDIR))
$(eval $(call gb_CustomTarget_add_dependencies,i18npool/source/textconversion,\
	i18npool/source/textconversion/data/*.dic \
))
$(eval $(call gb_CustomTarget_add_outdir_dependencies,i18npool/source/textconversion,\
	$(call gb_Executable_get_target_for_build,genconv_dict) \
))

# vim: set noet sw=4 ts=4:
