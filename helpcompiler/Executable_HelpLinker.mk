# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,HelpLinker))

$(eval $(call gb_Executable_set_include,HelpLinker,\
	-I$(SRCDIR)/helpcompiler/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,HelpLinker,\
    sal \
    helplinker \
))

$(eval $(call gb_Executable_use_externals,HelpLinker,\
    expat \
    libxslt \
    libxml2 \
    clucene \
))

$(eval $(call gb_Executable_add_exception_objects,HelpLinker,\
    helpcompiler/source/HelpLinker_main \
))

# vim:set noet sw=4 ts=4:
