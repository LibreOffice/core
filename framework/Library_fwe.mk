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

$(eval $(call gb_Library_Library,fwe))

$(eval $(call gb_Library_set_include,fwe,\
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,fwe,\
    -DFWE_DLLIMPLEMENTATION\
))

$(eval $(call gb_Library_use_sdk_api,fwe))

$(eval $(call gb_Library_use_external,fwe,boost_headers))

$(eval $(call gb_Library_use_libraries,fwe,\
    comphelper \
    cppu \
    cppuhelper \
    fwi \
    sal \
	i18nlangtag \
    svl \
    svt \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,fwe,\
    framework/source/fwe/classes/actiontriggercontainer \
    framework/source/fwe/classes/actiontriggerpropertyset \
    framework/source/fwe/classes/actiontriggerseparatorpropertyset \
    framework/source/fwe/classes/addonmenu \
    framework/source/fwe/classes/addonsoptions \
    framework/source/fwe/classes/bmkmenu \
    framework/source/fwe/classes/framelistanalyzer \
    framework/source/fwe/classes/fwkresid \
    framework/source/fwe/classes/imagewrapper \
    framework/source/fwe/classes/menuextensionsupplier \
    framework/source/fwe/classes/rootactiontriggercontainer \
    framework/source/fwe/classes/sfxhelperfunctions \
    framework/source/fwe/dispatch/interaction \
    framework/source/fwe/helper/actiontriggerhelper \
    framework/source/fwe/helper/configimporter \
    framework/source/fwe/helper/imageproducer \
    framework/source/fwe/helper/propertysetcontainer \
    framework/source/fwe/helper/titlehelper \
    framework/source/fwe/helper/documentundoguard \
    framework/source/fwe/helper/undomanagerhelper \
    framework/source/fwe/interaction/preventduplicateinteraction \
    framework/source/fwe/xml/menuconfiguration \
    framework/source/fwe/xml/menudocumenthandler \
    framework/source/fwe/xml/saxnamespacefilter \
    framework/source/fwe/xml/statusbarconfiguration \
    framework/source/fwe/xml/statusbardocumenthandler \
    framework/source/fwe/xml/toolboxconfiguration \
    framework/source/fwe/xml/toolboxdocumenthandler \
    framework/source/fwe/xml/xmlnamespaces \
))

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,fwe,fwe))

# vim: set noet sw=4 ts=4:
