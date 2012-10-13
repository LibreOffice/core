# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
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

$(eval $(call gb_Package_Package,sal_inc,$(SRCDIR)/sal/inc))

$(eval $(call gb_Package_add_file,sal_inc,inc/protectorfactory.hxx,cppunittester/protectorfactory.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/armarch.h,osl/armarch.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/conditn.h,osl/conditn.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/conditn.hxx,osl/conditn.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/diagnose.h,osl/diagnose.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/diagnose.hxx,osl/diagnose.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/doublecheckedlocking.h,osl/doublecheckedlocking.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/endian.h,osl/endian.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/file.h,osl/file.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/file.hxx,osl/file.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/getglobalmutex.hxx,osl/getglobalmutex.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/interlck.h,osl/interlck.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/module.h,osl/module.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/module.hxx,osl/module.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/mutex.h,osl/mutex.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/mutex.hxx,osl/mutex.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/nlsupport.h,osl/nlsupport.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/pipe_decl.hxx,osl/pipe_decl.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/pipe.h,osl/pipe.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/pipe.hxx,osl/pipe.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/process.h,osl/process.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/profile.h,osl/profile.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/profile.hxx,osl/profile.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/security_decl.hxx,osl/security_decl.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/security.h,osl/security.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/security.hxx,osl/security.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/semaphor.h,osl/semaphor.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/semaphor.hxx,osl/semaphor.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/signal.h,osl/signal.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/socket_decl.hxx,osl/socket_decl.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/socket.h,osl/socket.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/socket.hxx,osl/socket.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/thread.h,osl/thread.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/thread.hxx,osl/thread.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/time.h,osl/time.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/util.h,osl/util.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/file.h,osl/detail/file.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/android_native_app_glue.h,osl/detail/android_native_app_glue.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/android-bootstrap.h,osl/detail/android-bootstrap.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/ios-bootstrap.h,osl/detail/ios-bootstrap.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/osl/detail/component-mapping.h,osl/detail/component-mapping.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/allocator.hxx,rtl/allocator.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/alloc.h,rtl/alloc.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/bootstrap.h,rtl/bootstrap.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/bootstrap.hxx,rtl/bootstrap.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/byteseq.h,rtl/byteseq.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/byteseq.hxx,rtl/byteseq.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/cipher.h,rtl/cipher.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/crc.h,rtl/crc.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/digest.h,rtl/digest.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/instance.hxx,rtl/instance.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/locale.h,rtl/locale.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/locale.hxx,rtl/locale.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/logfile.h,rtl/logfile.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/logfile.hxx,rtl/logfile.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/malformeduriexception.hxx,rtl/malformeduriexception.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/math.h,rtl/math.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/math.hxx,rtl/math.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/memory.h,rtl/memory.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/oustringostreaminserter.hxx,rtl/oustringostreaminserter.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/process.h,rtl/process.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/random.h,rtl/random.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/ref.hxx,rtl/ref.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/strbuf.h,rtl/strbuf.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/strbuf.hxx,rtl/strbuf.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/string.h,rtl/string.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/string.hxx,rtl/string.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/stringutils.hxx,rtl/stringutils.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/tencinfo.h,rtl/tencinfo.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/textcvt.h,rtl/textcvt.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/textenc.h,rtl/textenc.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/unload.h,rtl/unload.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/uri.h,rtl/uri.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/uri.hxx,rtl/uri.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/ustrbuf.h,rtl/ustrbuf.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/ustrbuf.hxx,rtl/ustrbuf.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/ustring.h,rtl/ustring.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/ustring.hxx,rtl/ustring.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/rtl/uuid.h,rtl/uuid.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/alloca.h,sal/alloca.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/ByteBufferWrapper.hxx,sal/ByteBufferWrapper.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/config.h,sal/config.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/log-areas.dox,sal/log-areas.dox))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/log.hxx,sal/log.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/macros.h,sal/macros.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/main.h,sal/main.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/mathconf.h,sal/mathconf.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/saldllapi.h,sal/saldllapi.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/types.h,sal/types.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/sal/detail/log.h,sal/detail/log.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/comptr.hxx,systools/win32/comptr.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/comtools.hxx,systools/win32/comtools.hxx))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/qswin32.h,systools/win32/qswin32.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/snprintf.h,systools/win32/snprintf.h))
$(eval $(call gb_Package_add_file,sal_inc,inc/systools/win32/uwinapi.h,systools/win32/uwinapi.h))

# vim: set noet sw=4 ts=4:
