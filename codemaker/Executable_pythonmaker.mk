# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,pythonmaker))

$(eval $(call gb_Executable_use_external,pythonmaker,frozen))

$(eval $(call gb_Executable_use_external,pythonmaker,boost_headers))

$(eval $(call gb_Executable_use_libraries,pythonmaker,\
    unoidl \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),reg) \
    $(if $(filter TRUE,$(DISABLE_DYNLOADING)),store) \
    salhelper \
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,pythonmaker,\
    codemaker \
))

# Source files are located in codemaker/source/pythonmaker/
# Paths are relative to the 'libreoffice' root directory
$(eval $(call gb_Executable_add_exception_objects,pythonmaker,\
    codemaker/source/pythonmaker/pythonmaker \
    codemaker/source/pythonmaker/pythonoptions \
    codemaker/source/pythonmaker/pythontype \
))

# vim:set noet sw=4 ts=4:
