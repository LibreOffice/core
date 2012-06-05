# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Module_Module,touch))

ifneq (,$(filter IOS ANDROID,$(OS)))

$(eval $(call gb_Module_add_targets,touch,\
	CustomTarget_touch_javamaker \
	InternalUnoApi_touch \
	Library_libotouch \
	Package_inc \
))

endif

# vim: set noet sw=4 ts=4:
