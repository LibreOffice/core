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

# actool invocation creates all three files, (the icns files now only contains low-res icons)
# the --output-partial-info-plist is a required switch, but we just specify it in the Info.plist.in directly
# unfortunately it doesn't report error with exit status, but only via messages, and it also doesn't
# complain if not all images defined in the Contents.json are present, so the recipe is not
# all that reliable/error checking by it claiming it did (re)create all files is a bit hacky…
$(icns_WRKDIR)/Assets.car $(icns_WRKDIR)/AppIcon.icns $(icns_WRKDIR)/ignored.plist &: $(addprefix $(icns_SRCDIR)/Assets.xcassets/AppIcon.appiconset/,Contents.json $(foreach size,$(icns_sizes),icon_$(size).png)) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),CAR,4)
	actool $(icns_SRCDIR)/Assets.xcassets --compile $(@D) --output-format human-readable-text --platform macosx --app-icon AppIcon \
	    --minimum-deployment-target $(MACOSX_DEPLOYMENT_TARGET) --output-partial-info-plist $(@D)/ignored.plist \
	| awk '$(if $(verbose),{print $$0}) \
	    /$(subst /,\/,$(icns_WRKDIR)/Assets.car)/    {catalog=1} \
	    /$(subst /,\/,$(icns_WRKDIR)/AppIcon.icns)/  {icon=1} \
	    /$(subst /,\/,$(icns_WRKDIR)/ignored.plist)/ {plist=1} \
	    END {if (!(catalog && icon && plist)) {print "Error: actool did not create all required files" > "/dev/stderr"; exit 1}}'

$(icns_WRKDIR)/generic-%.icns: $(foreach size,$(icns_sizes),$(icns_SRCDIR)/mime-generic/%.iconset/icon_$(size).png) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),ICNS,3)
	iconutil -c icns -o $@ $(<D)

$(icns_WRKDIR)/oasis-%.icns: $(foreach size,$(icns_sizes),$(icns_SRCDIR)/mime-oasis/%.iconset/icon_$(size).png) | $(icns_WRKDIR)/.dir
	$(call gb_Output_announce,$(@F),$(true),ICNS,2)
	iconutil -c icns -o $@ $(<D)

$(eval $(call gb_Package_Package,sysui_osxicons,$(gb_CustomTarget_workdir)/sysui/icns))
$(call gb_Package_get_clean_target,sysui_osxicons): $(call gb_CustomTarget_get_clean_target,sysui/icns)

$(eval $(call gb_Package_add_files_with_dir,sysui_osxicons,Resources,\
    AppIcon.icns Assets.car \
    $(foreach type,$(icns_mimetypes),generic-$(type).icns oasis-$(type).icns) \
))

# vim: set ts=4 sw=4 et:
