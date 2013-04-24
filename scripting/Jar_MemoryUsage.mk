# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,MemoryUsage))

$(eval $(call gb_Jar_use_jars,MemoryUsage,\
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_add_sourcefiles,MemoryUsage,\
	scripting/examples/java/MemoryUsage/MemoryUsage \
))

$(eval $(call gb_Jar_set_packageroot,MemoryUsage,org))

# vim: set noet sw=4 ts=4:
