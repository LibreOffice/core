# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,recentfile))

$(eval $(call gb_Library_set_include,recentfile,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_externals,recentfile,\
	boost_headers \
	expat \
))

$(eval $(call gb_Library_use_libraries,recentfile,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_static_libraries,recentfile,\
	shell_xmlparser \
))

$(eval $(call gb_Library_add_exception_objects,recentfile,\
    shell/source/unix/sysshell/recently_used_file \
    shell/source/unix/sysshell/recently_used_file_handler \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
