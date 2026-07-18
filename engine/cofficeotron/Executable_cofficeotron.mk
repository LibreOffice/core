# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,cofficeotron))

# The zip reading code comes from the package module's library, whose
# objects are linked in because it does not export those classes.
$(eval $(call gb_Executable_use_library_objects,cofficeotron,\
	package2 \
))

$(eval $(call gb_Executable_use_static_libraries,cofficeotron,\
	cofficeotron \
))

$(eval $(call gb_Executable_use_libraries,cofficeotron,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	sax \
	tl \
	ucbhelper \
	utl \
))

$(eval $(call gb_Executable_use_externals,cofficeotron,\
	argon2 \
	expat \
	rnv \
	zlib \
	zstd \
))

$(eval $(call gb_Executable_add_exception_objects,cofficeotron,\
	cofficeotron/source/main \
))

# vim: set noet sw=4 ts=4:
