# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,juh))

$(eval $(call gb_Library_use_internal_comprehensive_api,juh,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,juh,\
    sal \
	$(gb_UWINAPI) \
))

ifneq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_Library_add_exception_objects,juh,\
    javaunohelper/source/preload \
))
else
# In the DISABLE_DYNLOADING case the juh library is a static archive that gets
# linked into the single .so, so we can put directly into it the code that in
# normal cases goes into the juhx library
$(eval $(call gb_Library_add_exception_objects,juh,\
    javaunohelper/source/bootstrap \
    javaunohelper/source/javaunohelper \
    javaunohelper/source/vm \
))
endif

# vim:set noet sw=4 ts=4:
