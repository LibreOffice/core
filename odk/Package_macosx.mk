# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_macosx,$(SRCDIR)/odk/pack/copying))

$(eval $(call gb_Package_set_outdir,odk_macosx,$(INSTDIR)))

$(eval $(call gb_Package_add_file,odk_macosx,$(gb_Package_SDKDIRNAME)/bin/addsym-macosx.sh,addsym-macosx.sh))

# vim: set noet sw=4 ts=4:
