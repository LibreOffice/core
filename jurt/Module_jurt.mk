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

$(eval $(call gb_Module_Module,jurt))

ifneq ($(SOLAR_JAVA),)

$(eval $(call gb_Module_add_targets,jurt,\
    Jar_jurt \
    Library_jpipe \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Module_add_targets,jurt,\
    CustomTarget_jnilib \
    Package_jnilib \
))
endif

ifeq ($(OS),WNT)
$(eval $(call gb_Module_add_targets,jurt,\
    Library_jpipx \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,jurt,\
    CustomTarget_test_urp \
    InternalUnoApi_test_urp \
    JunitTest_bridgefactory \
    JunitTest_connections \
    JunitTest_java \
    JunitTest_java_remote \
    JunitTest_remote \
    JunitTest_uno \
    JunitTest_urp \
    JunitTest_util \
))

endif

# vim:set noet sw=4 ts=4:
