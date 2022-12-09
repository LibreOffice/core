# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/libstdcpp))

$(call gb_CustomTarget_get_workdir,extras/libstdcpp)/libstdc++.so.6.0.28: \
	/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28 \
	$(SRCDIR)/extras/CustomTarget_libstdcpp.mk \
	| $(call gb_CustomTarget_get_workdir,extras/libstdcpp)/.dir

$(call gb_CustomTarget_get_workdir,extras/libstdcpp)/libstdc++.so.6.0.28:
	cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28 $@

# vim: set noet sw=4 ts=4:
