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

$(eval $(call gb_Library_Library,xmlreader))

$(eval $(call gb_Library_add_defs,xmlreader,\
	-DOOO_DLLIMPLEMENTATION_XMLREADER \
))

$(eval $(call gb_Library_set_is_ure_library_or_dependency,xmlreader))

$(eval $(call gb_Library_use_libraries,xmlreader,\
    cppu \
	sal \
))
    # cppu is only needed due to the cppumaker -C hack

$(eval $(call gb_Library_add_exception_objects,xmlreader,\
	xmlreader/source/pad \
	xmlreader/source/span \
	xmlreader/source/xmlreader \
))

$(eval $(call gb_Library_use_udk_api,xmlreader))

# vim: set noet sw=4 ts=4:
