# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,package_dtd,$(SRCDIR)/package/dtd))

$(eval $(call gb_Package_set_outdir,package_dtd,$(INSTROOT)))

$(eval $(call gb_Package_add_file,package_dtd,$(LIBO_SHARE_FOLDER)/dtd/officedocument/1_0/Manifest.dtd,Manifest.dtd))

# vim: set noet sw=4 ts=4:
