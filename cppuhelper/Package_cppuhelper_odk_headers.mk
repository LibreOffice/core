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

$(eval $(call gb_Package_Package,cppuhelper_odk_headers,$(SRCDIR)/cppuhelper/inc))

$(eval $(call gb_Package_set_outdir,cppuhelper_odk_headers,$(INSTDIR)))

$(eval $(call gb_Package_add_files,cppuhelper_odk_headers,$(gb_Package_SDKDIRNAME)/include/cppuhelper,\
	cppuhelper/cppuhelperdllapi.h \
	cppuhelper/access_control.hxx \
	cppuhelper/basemutex.hxx \
	cppuhelper/bootstrap.hxx \
	cppuhelper/compbase10.hxx \
	cppuhelper/compbase11.hxx \
	cppuhelper/compbase12.hxx \
	cppuhelper/compbase1.hxx \
	cppuhelper/compbase2.hxx \
	cppuhelper/compbase3.hxx \
	cppuhelper/compbase4.hxx \
	cppuhelper/compbase5.hxx \
	cppuhelper/compbase6.hxx \
	cppuhelper/compbase7.hxx \
	cppuhelper/compbase8.hxx \
	cppuhelper/compbase9.hxx \
	cppuhelper/compbase_ex.hxx \
	cppuhelper/compbase.hxx \
	cppuhelper/component_context.hxx \
	cppuhelper/component.hxx \
	cppuhelper/exc_hlp.hxx \
	cppuhelper/factory.hxx \
	cppuhelper/findsofficepath.h \
	cppuhelper/implbase10.hxx \
	cppuhelper/implbase11.hxx \
	cppuhelper/implbase12.hxx \
	cppuhelper/implbase13.hxx \
	cppuhelper/implbase1.hxx \
	cppuhelper/implbase2.hxx \
	cppuhelper/implbase3.hxx \
	cppuhelper/implbase4.hxx \
	cppuhelper/implbase5.hxx \
	cppuhelper/implbase6.hxx \
	cppuhelper/implbase7.hxx \
	cppuhelper/implbase8.hxx \
	cppuhelper/implbase9.hxx \
	cppuhelper/implbase_ex.hxx \
	cppuhelper/implbase_ex_post.hxx \
	cppuhelper/implbase_ex_pre.hxx \
	cppuhelper/implbase.hxx \
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
	cppuhelper/weakagg.hxx \
	cppuhelper/weak.hxx \
	cppuhelper/weakref.hxx \
))

# vim: set noet sw=4 ts=4:
