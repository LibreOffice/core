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

$(eval $(call gb_Library_Library,svgio))

$(eval $(call gb_Library_set_componentfile,svgio,svgio/svgio))

$(eval $(call gb_Library_set_include,svgio,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svgio/inc \
))

$(eval $(call gb_Library_use_external,svgio,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,svgio,svgio/inc/pch/precompiled_svgio))

$(eval $(call gb_Library_use_sdk_api,svgio))

$(eval $(call gb_Library_use_libraries,svgio,\
    basegfx \
    drawinglayer \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    tk \
    tl \
    sax \
    vcl \
    svt \
))

$(eval $(call gb_Library_add_exception_objects,svgio,\
    svgio/source/svgreader/svgcharacternode \
    svgio/source/svgreader/svgcirclenode \
    svgio/source/svgreader/svgclippathnode \
    svgio/source/svgreader/svgdocument \
    svgio/source/svgreader/svgdocumenthandler \
    svgio/source/svgreader/svgellipsenode \
    svgio/source/svgreader/svggnode \
    svgio/source/svgreader/svganode \
    svgio/source/svgreader/svggradientnode \
    svgio/source/svgreader/svggradientstopnode \
    svgio/source/svgreader/svgimagenode \
    svgio/source/svgreader/svglinenode \
    svgio/source/svgreader/svgmarkernode \
    svgio/source/svgreader/svgmasknode \
    svgio/source/svgreader/svgnode \
    svgio/source/svgreader/SvgNumber \
    svgio/source/svgreader/svgpaint \
    svgio/source/svgreader/svgpathnode \
    svgio/source/svgreader/svgpatternnode \
    svgio/source/svgreader/svgpolynode \
    svgio/source/svgreader/svgrectnode \
    svgio/source/svgreader/svgstyleattributes \
    svgio/source/svgreader/svgstylenode \
    svgio/source/svgreader/svgsvgnode \
    svgio/source/svgreader/svgsymbolnode \
    svgio/source/svgreader/svgtextnode \
    svgio/source/svgreader/svgtitledescnode \
    svgio/source/svgreader/svgtoken \
    svgio/source/svgreader/svgtrefnode \
    svgio/source/svgreader/svgtools \
    svgio/source/svgreader/svgtextpathnode \
    svgio/source/svgreader/svgtspannode \
    svgio/source/svgreader/svgusenode \
    svgio/source/svgreader/svgvisitor \
    svgio/source/svguno/xsvgparser \
))

# vim: set noet sw=4 ts=4:
