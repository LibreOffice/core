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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,vcl))

$(eval $(call gb_AllLangResTarget_set_reslocation,vcl,vcl))

$(eval $(call gb_AllLangResTarget_add_srs,vcl,\
    vcl/source/src \
))

$(eval $(call gb_SrsTarget_SrsTarget,vcl/source/src))

$(eval $(call gb_SrsTarget_use_packages,vcl/source/src,\
	svl_inc \
))

$(eval $(call gb_SrsTarget_set_include,vcl/source/src,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_SrsTarget_add_files,vcl/source/src,\
    vcl/source/src/btntext.src \
    vcl/source/src/helptext.src \
    vcl/source/src/images.src \
    vcl/source/src/menu.src \
    vcl/source/src/print.src \
    vcl/source/src/stdtext.src \
    vcl/source/src/throbber.src \
    vcl/source/src/units.src \
    vcl/source/src/fpicker.src \
    vcl/source/edit/textundo.src \
))


# vim: set noet sw=4 ts=4:
