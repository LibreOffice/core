# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,hyphen,hyphen))

ifneq ($(COM),MSC)

$(eval $(call gb_ExternalPackage_use_external_project,hyphen,hyphen))

ifeq ($(SYSTEM_HYPH),NO)
$(eval $(call gb_ExternalPackage_add_file,hyphen,lib/libhyphen.a,.libs/libhyphen.a))
endif

endif

ifeq ($(WITH_MYSPELL_DICTS),YES)
$(eval $(call gb_ExternalPackage_add_files,hyphen,bin,\
    hyph_en_US.dic \
))
endif

# vim: set noet sw=4 ts=4:
