# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

i18npool_ICULT53 := $(filter 1, $(shell expr $(ICU_MAJOR) \< 53))
i18npool_LCDALL := $(wildcard $(SRCDIR)/i18npool/source/collator/data/*.txt)
i18npool_LCDTXTS := $(if $(i18npool_ICULT53), $(i18npool_LCDALL), $(filter-out %/ko_charset.txt, $(i18npool_LCDALL)))

$(eval $(call gb_Library_Library,collator_data))

$(eval $(call gb_Library_add_generated_exception_objects,collator_data,\
	$(foreach txt,$(i18npool_LCDTXTS),\
		CustomTarget/i18npool/collator/collator_$(notdir $(basename $(txt)))) \
))

# vim: set noet sw=4 ts=4:
