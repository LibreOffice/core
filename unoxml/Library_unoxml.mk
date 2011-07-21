#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,unoxml))

$(eval $(call gb_Library_set_componentfile,unoxml,unoxml/source/service/unoxml))

$(eval $(call gb_Library_set_include,unoxml,\
    $$(INCLUDE) \
))

))

$(eval $(call gb_Library_add_api,unoxml,\
	udkapi \
	offapi \
$(eval $(call gb_Library_add_linked_libs,unoxml,\
    ucbhelper \
    sax \
    comphelper \
    cppuhelper \
    cppu \
    sal \
    $(gb_STDLIBS) \
))

$(call gb_Library_use_external,unoxml,libxml2)

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

