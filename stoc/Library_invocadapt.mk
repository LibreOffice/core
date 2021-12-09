# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,invocadapt))

$(eval $(call gb_Library_use_external,invocadapt,boost_headers))

$(eval $(call gb_Library_use_udk_api,invocadapt))

$(eval $(call gb_Library_use_libraries,invocadapt,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_set_componentfile,invocadapt,stoc/source/invocation_adapterfactory/invocadapt,ure/services))

$(eval $(call gb_Library_add_exception_objects,invocadapt,\
    stoc/source/invocation_adapterfactory/iafactory \
))

# vim:set noet sw=4 ts=4:
