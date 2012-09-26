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
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,deploymentmisc))

$(eval $(call gb_Library_set_include,deploymentmisc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
))

$(eval $(call gb_Library_use_sdk_api,deploymentmisc))

$(eval $(call gb_Library_add_defs,deploymentmisc,\
    -DDESKTOP_DEPLOYMENTMISC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,deploymentmisc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    utl \
    xmlscript \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,deploymentmisc))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_use_externals,deploymentmisc,\
    berkeleydb \
))
endif

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_add_exception_objects,deploymentmisc,\
    desktop/source/deployment/misc/db \
))
endif

$(eval $(call gb_Library_add_exception_objects,deploymentmisc,\
    desktop/source/deployment/misc/dp_dependencies \
    desktop/source/deployment/misc/dp_descriptioninfoset \
    desktop/source/deployment/misc/dp_identifier \
    desktop/source/deployment/misc/dp_interact \
    desktop/source/deployment/misc/dp_misc \
    desktop/source/deployment/misc/dp_platform \
    desktop/source/deployment/misc/dp_resource \
    desktop/source/deployment/misc/dp_ucb \
    desktop/source/deployment/misc/dp_update \
    desktop/source/deployment/misc/dp_version \
    desktop/source/deployment/misc/lockfile \
))

# vim: set ts=4 sw=4 et:
