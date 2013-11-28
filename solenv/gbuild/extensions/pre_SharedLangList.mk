#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# share language list with dmake build system
include $(SRCDIR)/solenv/inc/langlist.mk

ifneq ($(WITH_LANG),ALL)
gb_WITH_LANG=$(WITH_LANG)
else
# expand ALL based on language list)
gb_WITH_LANG=$(completelangiso)
endif
gb_HELP_LANGS := en-US

ifneq ($(ENABLE_RELEASE_BUILD),TRUE)
ifneq ($(WITH_LANG),)
gb_WITH_LANG += qtz
gb_HELP_LANGS += qtz
endif
endif

gb_TRANS_LANGS = $(filter-out en-US,$(filter-out qtz,$(gb_WITH_LANG)))

gb_HELP_LANGS += \
	$(foreach lang,$(filter-out $(WITH_POOR_HELP_LOCALIZATIONS),$(gb_TRANS_LANGS)),\
		$(if \
			$(and \
				$(wildcard $(SRCDIR)/helpcontent2/source/auxiliary/$(lang)),\
				$(wildcard $(SRCDIR)/translations/source/$(lang)/helpcontent2)),\
			$(lang)))

# Langs that need special handling for registry files. This is done by
# MAKE_LANG_SPECIFIC flag in scp.
gb_CJK_LANGS := $(filter $(gb_WITH_LANG),ja ko zh-CN zh-TW)
gb_CTL_LANGS := $(filter $(gb_WITH_LANG),ar bo dz fa gu he hi km ky-CN lo my ne or pa-IN ta th)
gb_CTLSEQCHECK_LANGS := $(filter $(gb_CTL_LANGS),km lo th)

# vim: set noet ts=4 sw=4:
