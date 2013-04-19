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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,fwe))

$(eval $(call gb_AllLangResTarget_set_reslocation,fwe,framework))

$(eval $(call gb_AllLangResTarget_add_srs,fwe,\
    fwe/fwk_classes \
    fwe/fwk_services \
))


$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_classes))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_classes,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/classes \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_classes,\
    framework/source/classes/resource.src \
))

$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_services))

$(eval $(call gb_SrsTarget_use_packages,fwe/fwk_services,\
))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_services,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/services \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_services,\
    framework/source/services/fwk_services.src \
))
# vim: set noet sw=4 ts=4:
