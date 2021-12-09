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

$(eval $(call gb_Library_Library,unordf))

$(eval $(call gb_Library_set_componentfile,unordf,unoxml/source/rdf/unordf,services))

$(eval $(call gb_Library_use_sdk_api,unordf))

$(eval $(call gb_Library_use_libraries,unordf,\
    comphelper \
    cppuhelper \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,unordf,\
	boost_headers \
	librdf \
	redland_headers \
	raptor_headers \
	rasqal_headers \
	libxslt \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,unordf,\
    unoxml/source/rdf/CBlankNode \
    unoxml/source/rdf/CURI \
    unoxml/source/rdf/CLiteral \
    unoxml/source/rdf/librdf_repository \
))

# vim: set noet sw=4 ts=4:
