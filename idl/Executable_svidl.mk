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

$(eval $(call gb_Executable_Executable,svidl))

$(eval $(call gb_Executable_use_external,svidl,boost_headers))

$(eval $(call gb_Executable_set_include,svidl,\
	$$(INCLUDE) \
	-I$(SRCDIR)/idl/inc \
))

$(eval $(call gb_Executable_use_sdk_api,svidl))

$(eval $(call gb_Executable_use_libraries,svidl,\
	tl \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,svidl,\
	idl/source/cmptools/hash \
	idl/source/cmptools/lex \
	idl/source/objects/basobj \
	idl/source/objects/bastype \
	idl/source/objects/module \
	idl/source/objects/object \
	idl/source/objects/slot \
	idl/source/objects/types \
	idl/source/prj/command \
	idl/source/prj/database \
	idl/source/prj/globals \
	idl/source/prj/svidl \
))

# vim: set noet sw=4 ts=4:
