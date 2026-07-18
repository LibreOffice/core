# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cofficeotron))

$(eval $(call gb_Module_add_targets,cofficeotron,\
	CustomTarget_cofficeotron_schema \
	Executable_cofficeotron \
	StaticLibrary_cofficeotron \
))

$(eval $(call gb_Module_add_slowcheck_targets,cofficeotron,\
	CppunitTest_cofficeotron \
	CustomTarget_cofficeotron_e2e \
))

# vim: set noet sw=4 ts=4:
