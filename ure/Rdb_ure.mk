# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Rdb_Rdb_install,ure/services,$(LIBO_URE_MISC_FOLDER)/services.rdb))

$(eval $(call gb_Rdb_add_components,ure/services,\
    binaryurp/source/binaryurp \
    io/source/io \
    stoc/util/bootstrap \
    stoc/source/inspect/introspection \
    stoc/source/invocation_adapterfactory/invocadapt \
    stoc/source/invocation/invocation \
    stoc/source/namingservice/namingservice \
    stoc/source/proxy_factory/proxyfac \
    stoc/source/corereflection/reflection \
    stoc/util/stocservices \
    remotebridges/source/unourl_resolver/uuresolver \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Rdb_add_components,ure/services,\
    stoc/source/javaloader/javaloader \
    stoc/source/javavm/javavm \
))
endif

# vim:set noet sw=4 ts=4:
