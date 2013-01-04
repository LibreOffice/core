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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,SunPresentationMinimizer))

$(eval $(call gb_Library_set_componentfile,SunPresentationMinimizer,sdext/source/minimizer/minimizer))

$(eval $(call gb_Library_use_externals,SunPresentationMinimizer,\
	boost_headers \
))

$(eval $(call gb_Library_use_sdk_api,SunPresentationMinimizer))

$(eval $(call gb_Library_use_api,SunPresentationMinimizer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Library_use_libraries,SunPresentationMinimizer,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_add_exception_objects,SunPresentationMinimizer,\
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
    sdext/source/minimizer/pppoptimizeruno \
    sdext/source/minimizer/unodialog \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
