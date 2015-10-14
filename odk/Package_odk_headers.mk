
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_headers,$(SRCDIR)/include))

$(eval $(call gb_Package_set_outdir,odk_headers,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,odk_headers,$(SDKDIRNAME)/include,\
	com/sun/star/uno/Any.h \
	com/sun/star/uno/Any.hxx \
	com/sun/star/uno/Reference.h \
	com/sun/star/uno/Reference.hxx \
	com/sun/star/uno/Sequence.h \
	com/sun/star/uno/Sequence.hxx \
	com/sun/star/uno/Type.h \
	com/sun/star/uno/Type.hxx \
	com/sun/star/uno/genfunc.h \
	com/sun/star/uno/genfunc.hxx \
	cppu/Enterable.hxx \
	cppu/EnvDcp.hxx \
	cppu/EnvGuards.hxx \
	cppu/Map.hxx \
	cppu/cppudllapi.h \
	cppu/helper/purpenv/Environment.hxx \
	cppu/helper/purpenv/Mapping.hxx \
	cppu/macros.hxx \
	cppu/unotype.hxx \
	cppuhelper/access_control.hxx \
	cppuhelper/basemutex.hxx \
	cppuhelper/bootstrap.hxx \
	cppuhelper/compbase.hxx \
	cppuhelper/compbase_ex.hxx \
	cppuhelper/component.hxx \
	cppuhelper/component_context.hxx \
	cppuhelper/cppuhelperdllapi.h \
	cppuhelper/exc_hlp.hxx \
	cppuhelper/factory.hxx \
	cppuhelper/findsofficepath.h \
	cppuhelper/implbase.hxx \
	cppuhelper/implbase_ex.hxx \
	cppuhelper/implbase_ex_post.hxx \
	cppuhelper/implbase_ex_pre.hxx \
	cppuhelper/implementationentry.hxx \
	cppuhelper/interfacecontainer.h \
	cppuhelper/interfacecontainer.hxx \
	cppuhelper/propertysetmixin.hxx \
	cppuhelper/propshlp.hxx \
	cppuhelper/proptypehlp.h \
	cppuhelper/proptypehlp.hxx \
	cppuhelper/queryinterface.hxx \
	cppuhelper/shlib.hxx \
	cppuhelper/supportsservice.hxx \
	cppuhelper/typeprovider.hxx \
	cppuhelper/unourl.hxx \
	cppuhelper/weak.hxx \
	cppuhelper/weakagg.hxx \
	cppuhelper/weakref.hxx \
	osl/conditn.h \
	osl/conditn.hxx \
	osl/diagnose.h \
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
	osl/pipe.h \
	osl/pipe.hxx \
	osl/pipe_decl.hxx \
	osl/process.h \
	osl/profile.h \
	osl/profile.hxx \
	osl/security.h \
	osl/security.hxx \
	osl/security_decl.hxx \
	osl/signal.h \
	osl/socket.h \
	osl/socket.hxx \
	osl/socket_decl.hxx \
	osl/thread.h \
	osl/thread.hxx \
	osl/time.h \
	rtl/alloc.h \
	rtl/bootstrap.h \
	rtl/bootstrap.hxx \
	rtl/byteseq.h \
	rtl/byteseq.hxx \
	rtl/character.hxx \
	rtl/cipher.h \
	rtl/crc.h \
	rtl/digest.h \
	rtl/instance.hxx \
	rtl/locale.h \
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
	sal/alloca.h \
	sal/config.h \
	sal/detail/log.h \
	sal/log.hxx \
	sal/macros.h \
	sal/main.h \
	sal/mathconf.h \
	sal/saldllapi.h \
	sal/types.h \
	salhelper/condition.hxx \
	salhelper/dynload.hxx \
	salhelper/linkhelper.hxx \
	salhelper/refobj.hxx \
	salhelper/salhelperdllapi.h \
	salhelper/simplereferenceobject.hxx \
	salhelper/singletonref.hxx \
	salhelper/thread.hxx \
	salhelper/timer.hxx \
	systools/win32/snprintf.h \
	typelib/typeclass.h \
	typelib/typedescription.h \
	typelib/typedescription.hxx \
	typelib/uik.h \
	uno/Enterable.h \
	uno/EnvDcp.h \
	uno/any2.h \
	uno/current_context.h \
	uno/current_context.hxx \
	uno/data.h \
	uno/dispatcher.h \
	uno/dispatcher.hxx \
	uno/environment.h \
	uno/environment.hxx \
	uno/lbnames.h \
	uno/mapping.h \
	uno/mapping.hxx \
	uno/sequence2.h \
	uno/threadpool.h \
))

# vim: set noet sw=4 ts=4:
