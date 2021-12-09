# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ctl))

$(eval $(call gb_Library_set_include,ctl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/UnoControls/inc \
    -I$(SRCDIR)/UnoControls/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,ctl))

$(eval $(call gb_Library_use_libraries,ctl,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
))

$(eval $(call gb_Library_set_componentfile,ctl,UnoControls/util/ctl,services))

$(eval $(call gb_Library_add_exception_objects,ctl,\
    UnoControls/source/base/basecontainercontrol \
    UnoControls/source/base/basecontrol \
    UnoControls/source/base/multiplexer \
    UnoControls/source/controls/OConnectionPointContainerHelper \
    UnoControls/source/controls/OConnectionPointHelper \
    UnoControls/source/controls/framecontrol \
    UnoControls/source/controls/progressbar \
    UnoControls/source/controls/progressmonitor \
    UnoControls/source/controls/statusindicator \
))

# vim:set noet sw=4 ts=4:
