# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,rustmaker))

$(eval $(call gb_Executable_use_libraries,rustmaker,\
    salhelper \
    sal \
    unoidl \
))

$(eval $(call gb_Executable_use_static_libraries,rustmaker,\
    codemaker \
))

$(eval $(call gb_Executable_add_exception_objects,rustmaker,\
    codemaker/source/rustmaker/rustmaker \
    codemaker/source/rustmaker/rustoptions \
    codemaker/source/rustmaker/rustproduce \
    codemaker/source/rustmaker/rustfile \
    codemaker/source/rustmaker/cpproduce \
    codemaker/source/rustmaker/unoproduce \
    codemaker/source/rustmaker/type_analyzer \
    codemaker/source/rustmaker/cpp_include_manager \
))

$(eval $(call gb_Executable_use_external,rustmaker,frozen))

# vim:set noet sw=4 ts=4:
