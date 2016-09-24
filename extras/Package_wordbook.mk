# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_wordbook,$(SRCDIR)/extras/source/wordbook))

$(eval $(call gb_Package_add_files,extras_wordbook,$(LIBO_SHARE_FOLDER)/wordbook,\
	en-GB.dic \
	en-US.dic \
	sl.dic \
	technical.dic \
))

# vim: set noet sw=4 ts=4:
