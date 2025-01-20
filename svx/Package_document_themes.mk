# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,svx_document_themes,$(SRCDIR)/svx/uiconfig/themes))

$(eval $(call gb_Package_add_files_with_dir,svx_document_themes,$(LIBO_SHARE_FOLDER)/themes,\
    Beach.theme \
    Breeze.theme \
    Forest.theme \
    Libreoffice.theme \
    Ocean.theme \
    Rainbow.theme \
    Sunset.theme \
))

# vim: set noet sw=4 ts=4:
