# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,gie))

$(eval $(call gb_Library_use_external,gie,boost_headers))

$(eval $(call gb_Library_use_sdk_api,gie))

$(eval $(call gb_Library_use_libraries,gie,\
	basegfx \
    comphelper \
    i18nlangtag \
    svt \
	vcl \
    utl \
	tl \
    tk \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,gie,\
	filter/source/graphicfilter/egif/egif \
	filter/source/graphicfilter/etiff/etiff \
	filter/source/graphicfilter/eos2met/eos2met \
	filter/source/graphicfilter/eps/eps \
	filter/source/graphicfilter/egif/giflzwc \
	filter/source/graphicfilter/itiff/ccidecom \
	filter/source/graphicfilter/itiff/itiff \
	filter/source/graphicfilter/itiff/lzwdecom \
	filter/source/graphicfilter/ipict/ipict \
	filter/source/graphicfilter/ipict/shape \
	filter/source/graphicfilter/ipcx/ipcx \
	filter/source/graphicfilter/iras/iras \
	filter/source/graphicfilter/itga/itga \
	filter/source/graphicfilter/ieps/ieps \
	filter/source/graphicfilter/ipsd/ipsd \
	filter/source/graphicfilter/ipbm/ipbm \
	filter/source/graphicfilter/ios2met/ios2met \
	filter/source/graphicfilter/idxf/dxf2mtf \
	filter/source/graphicfilter/idxf/dxfblkrd \
	filter/source/graphicfilter/idxf/dxfentrd \
	filter/source/graphicfilter/idxf/dxfgrprd \
	filter/source/graphicfilter/idxf/dxfreprd \
	filter/source/graphicfilter/idxf/dxftblrd \
	filter/source/graphicfilter/idxf/dxfvec \
	filter/source/graphicfilter/idxf/idxf \
	filter/source/graphicfilter/ipcd/ipcd \
))

# vim: set noet sw=4 ts=4:
