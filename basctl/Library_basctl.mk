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

$(eval $(call gb_Library_Library,basctl))

$(eval $(call gb_Library_set_componentfile,basctl,basctl/util/basctl))

$(eval $(call gb_Library_set_include,basctl,\
	-I$(SRCDIR)/basctl/inc \
	-I$(SRCDIR)/basctl/sdi \
	-I$(SRCDIR)/basctl/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/SdiTarget/basctl/sdi \
))

$(eval $(call gb_Library_use_external,basctl,boost_headers))

$(eval $(call gb_Library_use_custom_headers,basctl,\
	officecfg/registry \
))

$(eval $(call gb_Library_set_precompiled_header,basctl,basctl/inc/pch/precompiled_basctl))

$(eval $(call gb_Library_use_sdk_api,basctl))

$(eval $(call gb_Library_use_libraries,basctl,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	fwk \
	sal \
	i18nlangtag \
	sb \
	sfx \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	tl \
	utl \
	ucbhelper \
	vcl \
	xmlscript \
))

ifneq ($(ENABLE_WASM_STRIP_ACCESSIBILITY),TRUE)
$(eval $(call gb_Library_add_exception_objects,basctl,\
	basctl/source/accessibility/accessibledialogcontrolshape \
	basctl/source/accessibility/accessibledialogwindow \
))
endif

$(eval $(call gb_Library_add_exception_objects,basctl,\
	basctl/source/basicide/basdoc \
	basctl/source/basicide/IDEComboBox \
	basctl/source/basicide/basicrenderable \
	basctl/source/basicide/baside2b \
	basctl/source/basicide/baside2 \
	basctl/source/basicide/baside3 \
	basctl/source/basicide/basidectrlr \
	basctl/source/basicide/basides1 \
	basctl/source/basicide/basides2 \
	basctl/source/basicide/basides3 \
	basctl/source/basicide/basidesh \
	basctl/source/basicide/basobj2 \
	basctl/source/basicide/basobj3 \
	basctl/source/basicide/bastype2 \
	basctl/source/basicide/bastype3 \
	basctl/source/basicide/bastypes \
	basctl/source/basicide/breakpoint \
	basctl/source/basicide/brkdlg \
	basctl/source/basicide/doceventnotifier \
	basctl/source/basicide/docsignature \
	basctl/source/basicide/documentenumeration \
	basctl/source/basicide/iderdll \
	basctl/source/basicide/layout \
	basctl/source/basicide/linenumberwindow \
	basctl/source/basicide/localizationmgr \
	basctl/source/basicide/macrodlg \
	basctl/source/basicide/moduldl2 \
	basctl/source/basicide/moduldlg \
	basctl/source/basicide/ObjectCatalog \
	basctl/source/basicide/sbxitem \
	basctl/source/basicide/scriptdocument \
	basctl/source/basicide/textwindowpeer \
	basctl/source/basicide/uiobject \
	basctl/source/basicide/unomodel \
	basctl/source/dlged/dlgedclip \
	basctl/source/dlged/dlged \
	basctl/source/dlged/dlgedfac \
	basctl/source/dlged/dlgedfunc \
	basctl/source/dlged/dlgedlist \
	basctl/source/dlged/dlgedmod \
	basctl/source/dlged/dlgedobj \
	basctl/source/dlged/dlgedpage \
	basctl/source/dlged/dlgedview \
	basctl/source/dlged/managelang \
	basctl/source/dlged/propbrw \
))

$(eval $(call gb_Library_add_sdi_headers,basctl,basctl/sdi/basslots))

$(eval $(call gb_SdiTarget_SdiTarget,basctl/sdi/basslots,basctl/sdi/baside))

$(eval $(call gb_SdiTarget_set_include,basctl/sdi/basslots,\
    -I$(SRCDIR)/svx/sdi \
    -I$(SRCDIR)/sfx2/sdi \
    $$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,basctl,\
	advapi32 \
	gdi32 \
	shell32 \
))
endif

# vim: set noet sw=4 ts=4:
