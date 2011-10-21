#
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
# Copyright (C) 2011 Jan Holesovsky <kendy@suse.cz> (initial developer)
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
#

$(eval $(call gb_CppunitTest_CppunitTest,extensions_test_update))

$(eval $(call gb_CppunitTest_add_exception_objects,extensions_test_update, \
	extensions/source/update/check/download \
	extensions/source/update/check/updatecheck \
	extensions/source/update/check/updatecheckconfig \
	extensions/source/update/check/updatehdl \
	extensions/source/update/check/updateprotocol \
	\
	extensions/qa/update/test_update \
))

$(eval $(call gb_CppunitTest_add_linked_libs,extensions_test_update, \
	cppu \
	cppuhelper \
	curl \
	sal \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,extensions_test_update,\
	$$(INCLUDE) \
	-I$(realpath $(SRCDIR)/extensions/inc) \
	-I$(realpath $(SRCDIR)/extensions/inc/pch) \
	-I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_add_api,extensions_test_update,\
	offapi \
	udkapi \
))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
