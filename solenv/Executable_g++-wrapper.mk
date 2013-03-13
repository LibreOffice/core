# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,g++-wrapper))

$(eval $(call gb_Executable_add_exception_objects,g++-wrapper,\
	solenv/gcc-wrappers/g++ \
))

$(eval $(call gb_Executable_use_static_libraries,g++-wrapper,\
    wrapper \
))

# vim:set noet sw=4 ts=4:
