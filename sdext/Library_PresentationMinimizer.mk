# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,PresentationMinimizer))

$(eval $(call gb_Library_set_componentfile,PresentationMinimizer,sdext/source/minimizer/minimizer,services))

$(eval $(call gb_Library_set_precompiled_header,PresentationMinimizer,sdext/inc/pch/precompiled_PresentationMinimizer))

$(eval $(call gb_Library_use_externals,PresentationMinimizer,\
	boost_headers \
))

$(eval $(call gb_Library_use_sdk_api,PresentationMinimizer))

$(eval $(call gb_Library_use_api,PresentationMinimizer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_set_include,PresentationMinimizer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sdext/inc \
))

$(eval $(call gb_Library_use_libraries,PresentationMinimizer,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
	sfx \
	svt \
	vcl \
    tl \
))

$(eval $(call gb_Library_add_exception_objects,PresentationMinimizer,\
    sdext/source/minimizer/configurationaccess \
    sdext/source/minimizer/fileopendialog \
    sdext/source/minimizer/graphiccollector \
    sdext/source/minimizer/impoptimizer \
    sdext/source/minimizer/informationdialog \
    sdext/source/minimizer/optimizationstats \
    sdext/source/minimizer/optimizerdialog \
    sdext/source/minimizer/optimizerdialogcontrols \
    sdext/source/minimizer/pagecollector \
    sdext/source/minimizer/pppoptimizer \
    sdext/source/minimizer/pppoptimizerdialog \
    sdext/source/minimizer/pppoptimizertoken \
    sdext/source/minimizer/unodialog \
))

# vim:set noet sw=4 ts=4:
