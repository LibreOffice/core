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

$(eval $(call gb_Executable_Executable,so_checksum))

$(eval $(call gb_Executable_set_include,so_checksum,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc \
    -I$(SRCDIR)/tools/bootstrp \
))

$(eval $(call gb_Executable_use_libraries,so_checksum,\
    sal \
    tl \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))
# used to link against basegfxlx comphelp4gcc3 i18nisolang1gcc3 ucbhelper4gcc3 uno_cppu uno_cppuhelpergcc3 uno_salhelpergcc3 vos3gcc3 - seems to be superficial

$(eval $(call gb_Executable_add_exception_objects,so_checksum,\
    tools/bootstrp/md5 \
    tools/bootstrp/so_checksum \
))

# vim: set noet sw=4 ts=4:
