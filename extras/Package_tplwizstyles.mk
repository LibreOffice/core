# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_tplwizstyles,$(SRCDIR)/extras/source/templates/wizard/styles))

$(eval $(call gb_Package_set_outdir,extras_tplwizstyles,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files,extras_tplwizstyles,$(LIBO_SHARE_FOLDER)/template/common/wizard/styles,\
	blackberry.ots \
	black_white.ots \
	default.ots \
	diner.ots \
	fall.ots \
	glacier.ots \
	green_grapes.ots \
	jeans.ots \
	marine.ots \
	millennium.ots \
	nature.ots \
	neon.ots \
	night.ots \
	nostalgic.ots \
	pastell.ots \
	pool.ots \
	pumpkin.ots \
	xos.ots \
))

# vim: set noet sw=4 ts=4:
