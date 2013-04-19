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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,editeng))

$(eval $(call gb_AllLangResTarget_set_reslocation,editeng,svx))

$(eval $(call gb_AllLangResTarget_add_srs,editeng, editeng/res))
$(eval $(call gb_SrsTarget_SrsTarget,editeng/res))

$(eval $(call gb_SrsTarget_use_packages,editeng/res,\
))

$(eval $(call gb_SrsTarget_set_include,editeng/res,\
    $$(INCLUDE) \
    -I$(SRCDIR)/editeng/inc \
))

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,editeng/res,\
    editeng/source/accessibility/accessibility.src \
    editeng/source/editeng/editeng.src \
    editeng/source/items/page.src \
    editeng/source/items/svxitems.src \
    editeng/source/misc/lingu.src \
    editeng/source/outliner/outliner.src \
))

# vim: set noet sw=4 ts=4:
