# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

LO_PERSONAS_LIST=dark gray green pink sand white

$(foreach persona,$(LO_PERSONAS_LIST),\
    $(eval $(call gb_Package_Package,extras_persona_$(persona),$(SRCDIR)/extras/source/gallery/personas/$(persona))) \
    $(eval $(call gb_Package_add_files,extras_persona_$(persona),$(LIBO_SHARE_FOLDER)/gallery/personas/$(persona),\
        footer.png \
        header.png \
        preview.png \
    ))\
    $(eval $(call gb_Package_get_target,extras_personas): $(call gb_Package_get_target,extras_persona_$(persona))) \
)

$(eval $(call gb_Package_Package,extras_personas,$(SRCDIR)/extras/source/gallery/personas))

$(call gb_Package_get_clean_target,extras_personas):
	rm -rf $(INSTROOT)/$(LIBO_SHARE_FOLDER)/gallery/personas

# TODO: generate this, and potentially make labels translatable
$(eval $(call gb_Package_add_files,extras_personas,$(LIBO_SHARE_FOLDER)/gallery/personas,\
	personas_list.txt \
))	

# vim: set noet sw=4 ts=4:
