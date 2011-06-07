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
#       Caolán McNamara, Red Hat, Inc. <caolanm@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_filters_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filters_test, \
    sw/qa/core/filters-test \
))

$(eval $(call gb_CppunitTest_add_linked_libs,sw_filters_test, \
    sw \
    sfx \
    svl \
    svt \
    vcl \
    tl \
    ucbhelper \
    utl \
    i18nisolang1 \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sw_filters_test,\
    -I$(realpath $(SRCDIR)/sw/inc/pch) \
    -I$(realpath $(SRCDIR)/sw/source/ui/inc) \
    -I$(realpath $(SRCDIR)/sw/inc) \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_uses_ure,sw_filters_test))

$(eval $(call gb_CppunitTest_add_type_rdbs,sw_filters_test,\
    types \
))

$(eval $(call gb_CppunitTest_add_service_rdbs,sw_filters_test,\
    sw_filters_test \
))

$(eval $(call gb_CppunitTest_set_args,sw_filters_test,\
    --headless \
    --invisible \
))

$(eval $(call gb_RdbTarget_RdbTarget,sw_filters_test))

$(eval $(call gb_RdbTarget_add_components,sw_filters_test,\
    sw/util/sw \
    sw/util/msword \
    sfx2/util/sfx \
    framework/util/fwk \
    unoxml/source/service/unoxml \
    fileaccess/source/fileacc \
    comphelper/util/comphelp \
))

$(eval $(call gb_RdbTarget_add_old_components,sw_filters_test,\
    i18npool \
    ucb1 \
    ucpfile1 \
))

# vim: set noet sw=4:
