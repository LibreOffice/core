# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl,\
	sal/qa/ByteSequence/ByteSequence \
	sal/qa/OStringBuffer/rtl_OStringBuffer \
	sal/qa/rtl/alloc/rtl_alloc \
	sal/qa/rtl/bootstrap/expand \
	sal/qa/rtl/cipher/rtl_cipher \
	sal/qa/rtl/crc32/rtl_crc32 \
	sal/qa/rtl/digest/rtl_digest \
	sal/qa/rtl/doublelock/rtl_doublelocking \
	sal/qa/rtl/locale/rtl_locale \
	sal/qa/rtl/math/test-rtl-math \
	sal/qa/rtl/oustringbuffer/test_oustringbuffer_appendchar \
	sal/qa/rtl/oustringbuffer/test_oustringbuffer_appenduninitialized \
	sal/qa/rtl/oustringbuffer/test_oustringbuffer_assign \
	sal/qa/rtl/oustringbuffer/test_oustringbuffer_tostring \
	sal/qa/rtl/oustringbuffer/test_oustringbuffer_utf32 \
	sal/qa/rtl/oustring/rtl_OUString2 \
	sal/qa/rtl/process/rtl_Process \
	sal/qa/rtl/random/rtl_random \
	sal/qa/rtl/ref/rtl_ref \
	sal/qa/rtl/strings/nonconstarray \
	sal/qa/rtl/strings/test_strings_replace \
	sal/qa/rtl/strings/test_ostring \
	sal/qa/rtl/strings/test_ostring_concat \
	sal/qa/rtl/strings/test_ostring_stringliterals \
	sal/qa/rtl/strings/test_oustring_compare \
	sal/qa/rtl/strings/test_oustring_concat \
	sal/qa/rtl/strings/test_oustring_convert \
	sal/qa/rtl/strings/test_oustring_endswith \
	sal/qa/rtl/strings/test_oustring_startswith \
	sal/qa/rtl/strings/test_oustring_stringliterals \
	sal/qa/rtl/strings/test_strings_toint \
	sal/qa/rtl/strings/test_strings_valuex \
	sal/qa/rtl/textenc/rtl_tencinfo \
	sal/qa/rtl/textenc/rtl_textcvt \
	sal/qa/rtl/uri/rtl_Uri \
	sal/qa/rtl/uri/rtl_testuri \
	sal/qa/rtl/uuid/rtl_Uuid \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl,\
	sal \
))

$(eval $(call gb_CppunitTest_set_include,sal_rtl,\
	-I$(SRCDIR)/sal/qa/inc \
	$$(INCLUDE) \
))

# The test uses O(U)String capabilities that dlopen this lib:
$(call gb_CppunitTest_get_target,sal_rtl) : \
	$(call gb_Library_get_target,sal_textenc)

$(eval $(call gb_CppunitTest_use_external,sal_rtl,boost_headers))

ifeq ($(ENABLE_CIPHER_OPENSSL_BACKEND),TRUE)
$(eval $(call gb_CppunitTest_add_defs,sal_rtl,-DLIBO_CIPHER_OPENSSL_BACKEND))
endif

# vim: set noet sw=4 ts=4:
