# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cli_uno))

$(eval $(call gb_Library_add_cxxflags,cli_uno,\
	-AI $(gb_Helper_OUTDIRLIBDIR) \
	-clr:oldSyntax \
	-wd4339 \
))

$(eval $(call gb_Library_use_libraries,cli_uno,\
	cppu \
	mscoree \
	$(if $(USE_DEBUG_RUNTIME)\
		,msvcmrtd \
		,msvcmrt \
	) \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,cli_uno,\
    cli_ure/source/uno_bridge/cli_bridge \
    cli_ure/source/uno_bridge/cli_data \
    cli_ure/source/uno_bridge/cli_environment \
    cli_ure/source/uno_bridge/cli_proxy \
    cli_ure/source/uno_bridge/cli_uno \
))

# vim: set noet sw=4 ts=4:
