# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,smd))

$(eval $(call gb_Library_use_packages,smd,\
))

$(eval $(call gb_Library_set_componentfile,smd,starmath/util/smd))

$(eval $(call gb_Library_set_include,smd,\
        -I$(SRCDIR)/starmath/inc \
        $$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,smd,boost_headers))

$(eval $(call gb_Library_use_sdk_api,smd))

$(eval $(call gb_Library_use_libraries,smd,\
        cppu \
        cppuhelper \
        sal \
        sfx \
        sot \
        svl \
        svt \
        tl \
        ucbhelper \
        vcl \
		$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,smd,\
	starmath/source/detreg \
	starmath/source/smdetect \
	starmath/source/eqnolefilehdr \
))

# vim: set noet sw=4 ts=4:
