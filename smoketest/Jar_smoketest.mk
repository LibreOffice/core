# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,smoketest))

$(eval $(call gb_Jar_add_sourcefiles,smoketest, \
    smoketest/org/libreoffice/smoketest/Services \
    smoketest/org/libreoffice/smoketest/SmoketestCommandEnvironment \
))

$(eval $(call gb_Jar_set_componentfile,smoketest,smoketest/org/libreoffice/smoketest/smoketest,OOO,services))

$(eval $(call gb_Jar_set_manifest,smoketest,$(SRCDIR)/smoketest/org/libreoffice/smoketest/manifest))

$(eval $(call gb_Jar_set_packageroot,smoketest,org))

$(eval $(call gb_Jar_use_jars,smoketest, \
    libreoffice \
))

# vim: set noet sw=4 ts=4:
