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

$(eval $(call gb_Executable_Executable,g2g))

$(eval $(call gb_Executable_set_include,g2g,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svtools/inc \
    -I$(SRCDIR)/svtools/inc/svtools \
))

$(eval $(call gb_Executable_use_external,g2g,boost_headers))

$(eval $(call gb_Executable_use_sdk_api,g2g))

$(eval $(call gb_Executable_use_libraries,g2g,\
    cppuhelper \
    jvmfwk \
    sal \
    svt \
    tl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,g2g,\
    svtools/bmpmaker/g2g \
))

# vim: set noet sw=4 ts=4:
