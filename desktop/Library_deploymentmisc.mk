# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,deploymentmisc))

$(eval $(call gb_Library_set_include,deploymentmisc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
    -I$(SRCDIR)/desktop/source/deployment/inc \
))

$(eval $(call gb_Library_use_external,deploymentmisc,boost_headers))

$(eval $(call gb_Library_use_sdk_api,deploymentmisc))

$(eval $(call gb_Library_add_defs,deploymentmisc,\
    -DDESKTOP_DEPLOYMENTMISC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,deploymentmisc,$(SRCDIR)/desktop/inc/pch/precompiled_deploymentmisc))

$(eval $(call gb_Library_use_libraries,deploymentmisc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    utl \
    vcl \
    xmlscript \
    i18nlangtag \
))

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
