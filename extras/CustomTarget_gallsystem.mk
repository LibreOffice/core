# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/gallsysstr))

# bullets, fontwork symbolshapes not listed in ulf/not currently enabled for translation
$(eval $(call gb_CustomTarget_register_targets,extras/gallsysstr,\
    $(addsuffix .str,$(filter-out bullets fontwork symbolshapes,$(system_galleries))) \
))

$(eval $(call gb_CustomTarget_ulfex_rule,\
    $(gb_CustomTarget_workdir)/extras/gallsysstr/extras_gallsystem.ulf,\
    $(SRCDIR)/extras/source/gallery/share/gallery_names.ulf,\
    $(foreach lang,$(gb_TRANS_LANGS),\
        $(gb_POLOCATION)/$(lang)/extras/source/gallery/share.po)))

# desktop-translate.py is ugly af/doesn't play nice with make dependencies.
# It expects the target filename to exist and modifies it, so do the hack with own
# temporary dir
$(gb_CustomTarget_workdir)/extras/gallsysstr/%.str : \
                    $(gb_CustomTarget_workdir)/extras/gallsysstr/extras_gallsystem.ulf \
                    $(SRCDIR)/extras/source/gallery/gallery_system/dummy.str \
                    $(call gb_ExternalExecutable_get_dependencies,python) \
                    $(SRCDIR)/solenv/bin/desktop-translate.py
	mkdir -p $(@D)/$* && cp $(SRCDIR)/extras/source/gallery/gallery_system/dummy.str $(@D)/$*/$*.str && \
	$(call gb_ExternalExecutable_get_command,python) $(SRCDIR)/solenv/bin/desktop-translate.py \
            --ext "str" -d $(@D)/$*/ $(@D)/extras_gallsystem.ulf && \
	mv $(@D)/$*/$*.str $@

# vim: set noet sw=4 ts=4:
