# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
icns_WRKDIR := $(gb_CustomTarget_workdir)/sysui/icns
icns_SRCDIR := $(SRCDIR)/sysui/desktop/icons/macos

icns_mimetypes := database drawing drawing-template extension formula master-document \
                  master-document-template presentation presentation-template spreadsheet \
                  spreadsheet-template text text-template

icns_sizes := 16x16 16x16@2x 32x32 32x32@2x 128x128 128x128@2x 256x256 256x256@2x 512x512 512x512@2x

$(icns_WRKDIR)/.dir:
	mkdir -p $(@D) && touch $@

# the writer, calc, impress, draw & base app-icons are gone...
$(icns_WRKDIR)/main.icns: $(foreach size,$(icns_sizes),$(icns_SRCDIR)/app-launcher/main.iconset/icon_$(size).png) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),ICNS,4)
	iconutil -c icns -o $@ $(<D)

$(icns_WRKDIR)/generic-%.icns: $(foreach size,$(icns_sizes),$(icns_SRCDIR)/mime-generic/%.iconset/icon_$(size).png) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),ICNS,3)
	iconutil -c icns -o $@ $(<D)

$(icns_WRKDIR)/oasis-%.icns: $(foreach size,$(icns_sizes),$(icns_SRCDIR)/mime-oasis/%.iconset/icon_$(size).png) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),ICNS,2)
	iconutil -c icns -o $@ $(<D)

$(eval $(call gb_Package_Package,sysui_osxicons,$(gb_CustomTarget_workdir)/sysui/icns))
$(call gb_Package_get_clean_target,sysui_osxicons): $(call gb_CustomTarget_get_clean_target,sysui/icns)

$(eval $(call gb_Package_add_files_with_dir,sysui_osxicons,Resources,\
    main.icns \
    $(foreach type,$(icns_mimetypes),generic-$(type).icns oasis-$(type).icns) \
))

# vim: set ts=4 sw=4 et:
