# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Technically these files do not belong to extras module, but they must
# be delivered from somewhere, so why not from extras...
$(eval $(call gb_Package_Package,extras_extensions,$(SRCDIR)/src))

ifneq ($(BARCODE_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Barcode.oxt,$(BARCODE_EXTENSION_PACK)))
endif
ifneq ($(DIAGRAM_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/SmART.oxt,$(DIAGRAM_EXTENSION_PACK)))
endif
ifneq ($(GOOGLE_DOCS_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/gdocs.oxt,$(GOOGLE_DOCS_EXTENSION_PACK)))
endif
ifneq ($(HUNART_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/hunart.oxt,$(HUNART_EXTENSION_PACK)))
endif
ifneq ($(NUMBERTEXT_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/numberext.oxt,$(NUMBERTEXT_EXTENSION_PACK)))
endif
ifneq ($(OOOBLOGGER_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/oooblogger.oxt,$(OOOBLOGGER_EXTENSION_PACK)))
endif
ifneq ($(SUNTEMPLATES_DE_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-de,$(SUNTEMPLATES_DE_PACK)))
endif
ifneq ($(SUNTEMPLATES_EN_US_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-en-US,$(SUNTEMPLATES_EN_US_PACK)))
endif
ifneq ($(SUNTEMPLATES_ES_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-es,$(SUNTEMPLATES_ES_PACK)))
endif
ifneq ($(SUNTEMPLATES_FR_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-fr,$(SUNTEMPLATES_FR_PACK)))
endif
ifneq ($(SUNTEMPLATES_HU_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-hu,$(SUNTEMPLATES_HU_PACK)))
endif
ifneq ($(SUNTEMPLATES_IT_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Sun-ODF-Template-Pack-it,$(SUNTEMPLATES_IT_PACK)))
endif
ifneq ($(TYPO_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/typo.oxt,$(TYPO_EXTENSION_PACK)))
endif
ifneq ($(VALIDATOR_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/Validator.oxt,$(VALIDATOR_EXTENSION_PACK)))
endif
ifneq ($(WATCH_WINDOW_EXTENSION_PACK),)
$(eval $(call gb_Package_add_file,extras_extensions,bin/WatchWindow.oxt,$(WATCH_WINDOW_EXTENSION_PACK)))
endif

# vim: set noet sw=4 ts=4:
