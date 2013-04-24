# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,HelloWorld))

$(eval $(call gb_Jar_use_jars,HelloWorld,\
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_add_sourcefiles,HelloWorld,\
	scripting/examples/java/HelloWorld/HelloWorld \
))

$(eval $(call gb_Jar_set_packageroot,HelloWorld,org))

# vim: set noet sw=4 ts=4:
