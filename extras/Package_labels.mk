# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_labels,$(SRCDIR)/extras/source/labels))

$(eval $(call gb_Package_set_outdir,extras_labels,$(INSTDIR)))

$(eval $(call gb_Package_add_file,extras_labels,$(LIBO_SHARE_FOLDER)/labels/labels.xml,labels.xml))

# vim: set noet sw=4 ts=4:
