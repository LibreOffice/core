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
# Major Contributor(s): Caolán McNamara, Red Hat, Inc. <caolanm@redhat.com>
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,lotuswordpro_test_lotuswordpro))

$(eval $(call gb_CppunitTest_uses_ure,lotuswordpro_test_lotuswordpro))
$(eval $(call gb_CppunitTest_add_package_headers,lotuswordpro_test_lotuswordpro,lotuswordpro_qa_cppunit))

$(eval $(call gb_CppunitTest_add_exception_objects,lotuswordpro_test_lotuswordpro, \
    lotuswordpro/qa/cppunit/test_lotuswordpro \
))

$(eval $(call gb_CppunitTest_set_args,lotuswordpro_test_lotuswordpro,\
	--headless \
	--invisible \
	"-env:UNO_TYPES=$(foreach binrdb,udkapi.rdb types.rdb,\
		file://$(if $(filter WNT,$(OS)),/)$(OUTDIR)/bin/$(binrdb))" \
	"-env:UNO_SERVICES=$(foreach rdb,$(OUTDIR)/xml/ure/services.rdb $(WORKDIR)/CustomTarget/lotuswordpro/qa/cppunit/services.rdb,\
		file://$(if $(filter WNT,$(OS)),/)$(rdb))" \
	$(foreach dir,URE_INTERNAL_LIB_DIR OOO_BASE_DIR BRAND_BASE_DIR, \
		-env:$(dir)=file://$(if $(filter WNT,$(OS)),/$(OUTDIR)/bin,$(OUTDIR)/lib)) \
))

$(eval $(call gb_CppunitTest_add_linked_libs,lotuswordpro_test_lotuswordpro, \
    vcl \
    comphelper \
    cppuhelper \
    cppu \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,lotuswordpro_test_lotuswordpro,\
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

# we need to explicitly depend on library lwpft because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,lotuswordpro_test_lotuswordpro) : $(call gb_LinkTarget_get_target,lwpft)

# vim: set noet sw=4:
