# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,reflection))

$(eval $(call gb_Library_use_external,reflection,boost_headers))

$(eval $(call gb_Library_use_udk_api,reflection))

$(eval $(call gb_Library_use_libraries,reflection,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_set_componentfile,reflection,stoc/source/corereflection/reflection,ure/services))

$(eval $(call gb_Library_add_exception_objects,reflection,\
    stoc/source/corereflection/crarray \
    stoc/source/corereflection/crbase \
    stoc/source/corereflection/crcomp \
    stoc/source/corereflection/crefl \
    stoc/source/corereflection/crenum \
    stoc/source/corereflection/criface \
))

# vim:set noet sw=4 ts=4:
