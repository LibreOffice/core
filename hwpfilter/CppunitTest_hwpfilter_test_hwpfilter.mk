#*************************************************************************
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
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#       Caolán McNamara, Red Hat, Inc. <caolanm@redhat.com>
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,hwpfilter_test_hwpfilter))

$(eval $(call gb_CppunitTest_add_exception_objects,hwpfilter_test_hwpfilter, \
    hwpfilter/qa/cppunit/test_hwpfilter \
))

$(eval $(call gb_CppunitTest_add_linked_libs,hwpfilter_test_hwpfilter, \
    ucbhelper \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,hwpfilter_test_hwpfilter,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_add_api,hwpfilter_test_hwpfilter,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_uses_ure,hwpfilter_test_hwpfilter))

$(eval $(call gb_CppunitTest_add_type_rdbs,hwpfilter_test_hwpfilter,\
    types \
))

$(eval $(call gb_CppunitTest_add_service_rdbs,hwpfilter_test_hwpfilter,\
    hwpfilter_test_hwpfilter \
))

$(eval $(call gb_CppunitTest_set_args,hwpfilter_test_hwpfilter,\
    --headless \
    --invisible \
))

$(eval $(call gb_RdbTarget_RdbTarget,hwpfilter_test_hwpfilter))

$(eval $(call gb_RdbTarget_add_components,hwpfilter_test_hwpfilter,\
    hwpfilter/source/hwp,\
))

$(eval $(call gb_RdbTarget_add_old_components,hwpfilter_test_hwpfilter,\
    ucb1 \
    ucpfile1 \
))

# we need to explicitly depend on library hwp because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,hwpfilter_test_hwpfilter) : $(call gb_Library_get_target,hwp)

# vim: set noet sw=4:
