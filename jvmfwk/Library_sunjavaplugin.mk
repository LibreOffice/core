# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	$(gb_STDLIBS) \
))

ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_use_libraries,sunjavaplugin,\
    lo-bootstrap \
))
endif

ifeq ($(GUI),WNT)
$(eval $(call gb_Library_use_libraries,sunjavaplugin,\
    advapi32 \
    uwinapi \
))
endif

$(eval $(call gb_Library_use_externals,sunjavaplugin,\
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

# vim:set shiftwidth=4 softtabstop=4 expandtab:
