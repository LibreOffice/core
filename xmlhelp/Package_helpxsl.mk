# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,xmlhelp_helpxsl,$(SRCDIR)/xmlhelp/util))

$(eval $(call gb_Package_set_outdir,xmlhelp_helpxsl,$(INSTDIR)))

$(eval $(call gb_Package_add_files,xmlhelp_helpxsl,$(LIBO_SHARE_HELP_FOLDER),\
	idxcaption.xsl \
	idxcontent.xsl \
	main_transform.xsl \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
