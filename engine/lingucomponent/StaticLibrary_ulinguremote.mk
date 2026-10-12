# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# What the remote sentence checkers share. Kept apart from ulingu so the
# spell checker, hyphenator and thesaurus do not pick up a curl dependency.

$(eval $(call gb_StaticLibrary_StaticLibrary,ulinguremote))

$(eval $(call gb_StaticLibrary_use_api,ulinguremote,\
	udkapi \
	offapi \
))

$(eval $(call gb_StaticLibrary_use_externals,ulinguremote,\
	boost_headers \
	curl \
))

$(eval $(call gb_StaticLibrary_use_custom_headers,ulinguremote,\
	officecfg/registry \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,ulinguremote,\
	lingucomponent/source/lingutil/remotechecker \
))

# vim: set noet sw=4 ts=4:
