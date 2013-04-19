# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sunjavaplugin))

$(eval $(call gb_Library_add_defs,sunjavaplugin,\
    -DJVMFWK_PLUGIN_DLLIMPLEMENTATION \
))

ifneq ($(JVM_ONE_PATH_CHECK),)
$(eval $(call gb_Library_add_defs,sunjavaplugin,\
    -DJVM_ONE_PATH_CHECK=\"$(JVM_ONE_PATH_CHECK)\" \
))
endif

$(eval $(call gb_Library_use_api,sunjavaplugin,\
    udkapi \
))

$(eval $(call gb_Library_use_package,sunjavaplugin,\
    jvmfwk_inc \
))

$(eval $(call gb_Library_use_libraries,sunjavaplugin,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_use_libraries,sunjavaplugin,\
    lo-bootstrap \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,sunjavaplugin,\
	advapi32 \
))
endif

$(eval $(call gb_Library_use_externals,sunjavaplugin,\
	boost_headers \
    valgrind \
))

$(eval $(call gb_Library_add_exception_objects,sunjavaplugin,\
    jvmfwk/plugins/sunmajor/pluginlib/gnujre \
    jvmfwk/plugins/sunmajor/pluginlib/otherjre \
    jvmfwk/plugins/sunmajor/pluginlib/sunjavaplugin \
    jvmfwk/plugins/sunmajor/pluginlib/sunjre \
    jvmfwk/plugins/sunmajor/pluginlib/sunversion \
    jvmfwk/plugins/sunmajor/pluginlib/util \
    jvmfwk/plugins/sunmajor/pluginlib/vendorbase \
    jvmfwk/plugins/sunmajor/pluginlib/vendorlist \
))

# vim:set noet sw=4 ts=4:
