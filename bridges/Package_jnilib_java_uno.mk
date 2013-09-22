# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,bridges_jnilib_java_uno,$(SRCDIR)))

$(eval $(call gb_Package_set_outdir,bridges_jnilib_java_uno,$(INSTROOT)))

$(eval $(call gb_Package_add_symbolic_link,bridges_jnilib_java_uno,$(LIBO_URE_LIB_FOLDER)/$(basename $(notdir $(call gb_Library_get_target,java_uno))).jnilib,$(notdir $(call gb_Library_get_target,java_uno))))

# vim: set noet sw=4 ts=4:
