# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,collator_data))

$(eval $(call gb_Library_use_libraries,collator_data,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_generated_exception_objects,collator_data,\
	$(foreach txt,$(wildcard $(SRCDIR)/i18npool/source/collator/data/*.txt),\
		CustomTarget/i18npool/collator/collator_$(notdir $(basename $(txt)))) \
))

# vim: set noet sw=4 ts=4:
