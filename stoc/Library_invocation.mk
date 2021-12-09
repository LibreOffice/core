# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,invocation))

$(eval $(call gb_Library_use_external,invocation,boost_headers))

$(eval $(call gb_Library_use_udk_api,invocation))

$(eval $(call gb_Library_use_libraries,invocation,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_set_componentfile,invocation,stoc/source/invocation/invocation,ure/services))

$(eval $(call gb_Library_add_exception_objects,invocation,\
    stoc/source/invocation/invocation \
))

# vim:set noet sw=4 ts=4:
