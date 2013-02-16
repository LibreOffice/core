# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,instooofiltmsi))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,instooofiltmsi,\
	/DEF:$(SRCDIR)/setup_native/source/win32/customactions/indexingfilter/instooofiltmsi.def \
))
endif

$(eval $(call gb_Library_add_exception_objects,instooofiltmsi,\
    setup_native/source/win32/customactions/indexingfilter/restartindexingservice \
))

# vim: set noet sw=4 ts=4:
