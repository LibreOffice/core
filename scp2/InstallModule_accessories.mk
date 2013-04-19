# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/accessories))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/accessories,\
    scp2/source/accessories/module_accessories \
))

ifneq ($(WITH_EXTRA_FONT),)
$(eval $(call gb_InstallModule_add_scpfiles,scp2/accessories,\
    scp2/source/accessories/file_font_accessories \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/accessories,\
    scp2/source/accessories/module_font_accessories \
))
endif

ifneq ($(WITH_EXTRA_GALLERY),)
$(eval $(call gb_InstallModule_add_scpfiles,scp2/accessories,\
    scp2/source/accessories/file_gallery_accessories \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/accessories,\
    scp2/source/accessories/module_gallery_accessories \
))
endif

ifneq ($(WITH_EXTRA_SAMPLE),)
$(eval $(call gb_InstallModule_add_templates,scp2/accessories,\
    scp2/source/templates/module_langpack_accessories_samples \
    scp2/source/templates/module_langpack_accessories_samples_root \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/accessories,\
    scp2/source/accessories/file_samples_accessories \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/accessories,\
    scp2/source/accessories/module_samples_accessories \
))
endif

ifneq ($(WITH_EXTRA_TEMPLATE),)
$(eval $(call gb_InstallModule_add_templates,scp2/accessories,\
    scp2/source/templates/module_langpack_accessories_templates \
    scp2/source/templates/module_langpack_accessories_templates_root \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/accessories,\
    scp2/source/accessories/file_templates_accessories \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/accessories,\
    scp2/source/accessories/module_templates_accessories \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
