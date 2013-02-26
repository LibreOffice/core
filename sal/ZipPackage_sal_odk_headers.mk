# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ZipPackage_ZipPackage,sal_odk_headers,$(SRCDIR)/sal/inc))

$(eval $(call gb_ZipPackage_add_files,sal_odk_headers,inc/sal,include/sal,\
	sal/alloca.h \
	sal/ByteBufferWrapper.hxx \
	sal/config.h \
	sal/log.hxx \
	sal/macros.h \
	sal/main.h \
	sal/mathconf.h \
	sal/saldllapi.h \
	sal/types.h \
))

$(eval $(call gb_ZipPackage_add_files,sal_odk_headers,inc/sal/detail,include/sal/detail,\
	sal/detail/log.h \
))

$(eval $(call gb_ZipPackage_add_files,sal_odk_headers,inc/rtl,include/rtl,\
	rtl/allocator.hxx \
	rtl/alloc.h \
	rtl/bootstrap.h \
	rtl/bootstrap.hxx \
	rtl/byteseq.h \
	rtl/byteseq.hxx \
	rtl/cipher.h \
	rtl/crc.h \
	rtl/digest.h \
	rtl/instance.hxx \
	rtl/locale.h \
	rtl/logfile.h \
	rtl/logfile.hxx \
	rtl/malformeduriexception.hxx \
	rtl/math.h \
	rtl/math.hxx \
	rtl/process.h \
	rtl/random.h \
	rtl/ref.hxx \
	rtl/strbuf.h \
	rtl/strbuf.hxx \
	rtl/string.h \
	rtl/string.hxx \
	rtl/stringconcat.hxx \
	rtl/stringutils.hxx \
	rtl/tencinfo.h \
	rtl/textcvt.h \
	rtl/textenc.h \
	rtl/unload.h \
	rtl/uri.h \
	rtl/uri.hxx \
	rtl/ustrbuf.h \
	rtl/ustrbuf.hxx \
	rtl/ustring.h \
	rtl/ustring.hxx \
	rtl/uuid.h \
))

$(eval $(call gb_ZipPackage_add_files,sal_odk_headers,inc/osl,include/osl,\
	osl/conditn.h \
	osl/conditn.hxx \
	osl/diagnose.h \
	osl/diagnose.hxx \
	osl/doublecheckedlocking.h \
	osl/endian.h \
	osl/file.h \
	osl/file.hxx \
	osl/getglobalmutex.hxx \
	osl/interlck.h \
	osl/module.h \
	osl/module.hxx \
	osl/mutex.h \
	osl/mutex.hxx \
	osl/nlsupport.h \
	osl/pipe_decl.hxx \
	osl/pipe.h \
	osl/pipe.hxx \
	osl/process.h \
	osl/profile.h \
	osl/profile.hxx \
	osl/security_decl.hxx \
	osl/security.h \
	osl/security.hxx \
	osl/signal.h \
	osl/socket_decl.hxx \
	osl/socket.h \
	osl/socket.hxx \
	osl/thread.h \
	osl/thread.hxx \
	osl/time.h \
	osl/util.h \
))

# vim: set noet sw=4 ts=4:
