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
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
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

$(eval $(call gb_CppunitTest_CppunitTest,starmath_qa_cppunit))

$(eval $(call gb_CppunitTest_set_include,starmath_qa_cppunit,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/starmath/inc) \
    -I$(realpath $(SRCDIR)/starmath/inc/pch) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_CppunitTest_set_defs,starmath_qa_cppunit,\
    $$(DEFS) \
    -DSMDLL \
))

$(eval $(call gb_CppunitTest_add_library_objects,starmath_qa_cppunit,\
    sm \
))

$(call gb_CxxObject_get_target,starmath/qa/cppunit/test_starmath): $(WORKDIR)/AllLangRes/sm

$(eval $(call gb_CppunitTest_add_linked_libs,starmath_qa_cppunit,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    i18npaper \
    sal \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    utl \
    vcl \
    xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,starmath_qa_cppunit,\
    starmath/qa/cppunit/test_nodetotextvisitors \
    starmath/qa/cppunit/test_starmath \
))

$(eval $(call gb_CppunitTest_uses_ure,starmath_qa_cppunit))

$(eval $(call gb_CppunitTest_add_type_rdbs,starmath_qa_cppunit,\
    types \
))

$(eval $(call gb_CppunitTest_add_service_rdbs,starmath_qa_cppunit,\
    starmath_qa_cppunit \
))

$(eval $(call gb_CppunitTest_set_args,starmath_qa_cppunit,\
    --headless \
    --invisible \
))

$(eval $(call gb_RdbTarget_RdbTarget,starmath_qa_cppunit))

$(eval $(call gb_RdbTarget_add_components,starmath_qa_cppunit,\
    framework/util/fwk \
    toolkit/util/tk \
    sfx2/util/sfx \
))

$(eval $(call gb_RdbTarget_add_old_components,starmath_qa_cppunit,\
    component/vcl/vcl \
    configmgr \
    i18npool \
    mcnttype \
))

ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_RdbTarget_add_old_components,starmath_qa_cppunit,\
    ftransl \
    sysdtrans \
))
endif

# vim: set noet sw=4:
