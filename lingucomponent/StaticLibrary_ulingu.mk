# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,ulingu))

$(eval $(call gb_StaticLibrary_use_api,ulingu,\
	udkapi \
	offapi \
))

ifeq ($(SYSTEM_DICTS),YES)
$(eval $(call gb_StaticLibrary_add_defs,ulingu,\
	-DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\" \
))

endif

$(eval $(call gb_StaticLibrary_use_externals,ulingu,\
	boost_headers \
	hunspell \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,ulingu,\
	lingucomponent/source/lingutil/lingutil \
))

# vim: set noet sw=4 ts=4:
