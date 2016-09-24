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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,sb))


$(eval $(call gb_AllLangResTarget_add_srs,sb,\
	sb/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,sb/res))

$(eval $(call gb_SrsTarget_set_include,sb/res,\
	$$(INCLUDE) \
    -I$(SRCDIR)/basic/inc \
))

$(eval $(call gb_SrsTarget_add_files,sb/res,\
	basic/source/classes/sb.src \
	basic/source/sbx/format.src \
))

# vim: set noet sw=4 ts=4:
