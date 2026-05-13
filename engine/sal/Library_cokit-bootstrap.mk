# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cokit-bootstrap))

ifeq (ANDROID,$(OS))

# We explicitly *don't* want gb_STDLIBS to be linked here
$(eval $(call gb_Library_disable_standard_system_libs,cokit-bootstrap))

$(eval $(call gb_Library_add_libs,cokit-bootstrap,\
	-llog \
	-landroid \
))

$(eval $(call gb_Library_add_cobjects,cokit-bootstrap,\
	sal/android/cokit-jni \
	sal/android/cokit-bootstrap \
))
endif

ifeq (EMSCRIPTEN,$(OS))
$(eval $(call gb_Library_add_cobjects,cokit-bootstrap,\
	sal/emscripten/cokit-bootstrap \
))
endif

$(eval $(call gb_Library_set_include,cokit-bootstrap,\
	$$(INCLUDE) \
	-I$(SRCDIR)/sal/inc \
))

# vim: set noet sw=4 ts=4:
