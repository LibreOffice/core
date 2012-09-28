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

$(eval $(call gb_Library_Library,unoxml))

$(eval $(call gb_Library_set_componentfile,unoxml,unoxml/source/service/unoxml))

$(eval $(call gb_Library_set_include,unoxml,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,unoxml))

$(eval $(call gb_Library_use_libraries,unoxml,\
    ucbhelper \
    sax \
    comphelper \
    cppuhelper \
    cppu \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_external,unoxml,libxml2))

$(eval $(call gb_Library_add_exception_objects,unoxml,\
    unoxml/source/dom/node \
    unoxml/source/dom/document \
    unoxml/source/dom/element \
    unoxml/source/dom/attr \
    unoxml/source/dom/cdatasection \
    unoxml/source/dom/characterdata \
    unoxml/source/dom/comment \
    unoxml/source/dom/documentbuilder \
    unoxml/source/dom/documentfragment \
    unoxml/source/dom/documenttype \
    unoxml/source/dom/entity \
    unoxml/source/dom/entityreference \
    unoxml/source/dom/notation \
    unoxml/source/dom/processinginstruction \
    unoxml/source/dom/text \
    unoxml/source/dom/domimplementation \
    unoxml/source/dom/elementlist \
    unoxml/source/dom/childlist \
    unoxml/source/dom/notationsmap \
    unoxml/source/dom/entitiesmap \
    unoxml/source/dom/attributesmap \
    unoxml/source/dom/saxbuilder \
    unoxml/source/xpath/xpathobject \
    unoxml/source/xpath/nodelist \
    unoxml/source/xpath/xpathapi \
    unoxml/source/events/event \
    unoxml/source/events/eventdispatcher \
    unoxml/source/events/mutationevent \
    unoxml/source/events/uievent \
    unoxml/source/events/mouseevent \
    unoxml/source/events/testlistener \
    unoxml/source/service/services \
))

# vim: set noet sw=4 ts=4:
