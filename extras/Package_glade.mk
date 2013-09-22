# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_glade,$(call gb_CustomTarget_get_workdir,extras/source/glade)))

$(eval $(call gb_Package_set_outdir,extras_glade,$(INSTROOT)))

$(eval $(call gb_Package_add_file,extras_glade,$(LIBO_SHARE_FOLDER)/glade/libreoffice-catalog.xml,libreoffice-catalog.xml))

# vim: set noet sw=4 ts=4:
