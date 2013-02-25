# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,moz_lib,moz_lib))

$(eval $(call gb_ExternalPackage_add_unpacked_files,moz_lib,lib,\
	embed_base_s.lib \
	mozreg_s.lib \
	nslber32v50.lib \
	nsldap32v50.lib \
	xpcom.lib \
	xpcom_core.lib \
))

# vim: set noet sw=4 ts=4:
