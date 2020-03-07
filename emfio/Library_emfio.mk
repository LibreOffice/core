#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,emfio))

$(eval $(call gb_Library_set_componentfile,emfio,emfio/emfio))

$(eval $(call gb_Library_set_include,emfio,\
    $$(INCLUDE) \
    -I$(SRCDIR)/emfio/inc \
))

$(eval $(call gb_Library_add_defs,emfio,\
	-DEMFIO_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_custom_headers,emfio,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_external,emfio,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,emfio,emfio/inc/pch/precompiled_emfio))

$(eval $(call gb_Library_use_sdk_api,emfio))

$(eval $(call gb_Library_use_libraries,emfio,\
    basegfx \
    drawinglayercore \
    drawinglayer \
    cppu \
    cppuhelper \
    sal \
    comphelper \
    tl \
    salhelper \
    sax \
    vcl \
    svt \
    utl \
))

$(eval $(call gb_Library_add_exception_objects,emfio,\
    emfio/source/emfuno/xemfparser \
    emfio/source/reader/mtftools \
    emfio/source/reader/wmfreader \
    emfio/source/reader/emfreader \
))

# vim: set noet sw=4 ts=4:
