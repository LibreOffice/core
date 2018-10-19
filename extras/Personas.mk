# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_personas,$(SRCDIR)/extras/source/gallery/personas))

$(eval $(call gb_Package_add_files,extras_personas,$(LIBO_SHARE_FOLDER)/gallery/personas,\
	personas_list.txt \
))	

# vim: set noet sw=4 ts=4:
