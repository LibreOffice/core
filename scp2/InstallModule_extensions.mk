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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_InstallModule_InstallModule,scp2/extensions))

$(eval $(call gb_InstallModule_define_if_set,scp2/extensions,\
	SYSTEM_BSH \
	SYSTEM_RHINO \
))

$(eval $(call gb_InstallModule_add_defs,scp2/extensions,\
	$(if $(filter YES,$(SYSTEM_BSH)),\
		-DBSH_JAR=\""$(call gb_Helper_make_url,$(BSH_JAR))"\" \
	) \
	$(if $(filter YES,$(SYSTEM_RHINO)),\
		-DRHINO_JAR=\""$(call gb_Helper_make_url,$(RHINO_JAR))"\" \
	) \
))

$(eval $(call gb_InstallModule_add_templates,scp2/extensions,\
    scp2/source/templates/module_langpack_extensions_templates \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/extensions,\
    scp2/source/extensions/directory_extensions \
    scp2/source/extensions/file_extensions \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/extensions,\
    scp2/source/extensions/module_extensions \
    scp2/source/extensions/module_extensions_sun_templates \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
