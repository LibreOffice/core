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
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com>
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

$(eval $(call gb_Package_Package,cppu_odk_headers,$(SRCDIR)/cppu/inc))

$(eval $(call gb_Package_set_outdir,cppu_odk_headers,$(INSTDIR)))

$(eval $(call gb_Package_add_files,cppu_odk_headers,$(gb_Package_SDKDIRNAME)/include/com/sun/star/uno,\
	com/sun/star/uno/Any.h \
	com/sun/star/uno/Any.hxx \
	com/sun/star/uno/genfunc.h \
	com/sun/star/uno/genfunc.hxx \
	com/sun/star/uno/Reference.h \
	com/sun/star/uno/Reference.hxx \
	com/sun/star/uno/Sequence.h \
	com/sun/star/uno/Sequence.hxx \
	com/sun/star/uno/Type.h \
	com/sun/star/uno/Type.hxx \
))

$(eval $(call gb_Package_add_files,cppu_odk_headers,$(gb_Package_SDKDIRNAME)/include/cppu,\
	cppu/cppudllapi.h \
	cppu/EnvDcp.hxx \
	cppu/Enterable.hxx \
	cppu/EnvGuards.hxx \
	cppu/macros.hxx \
	cppu/Map.hxx \
	cppu/unotype.hxx \
))

$(eval $(call gb_Package_add_files,cppu_odk_headers,$(gb_Package_SDKDIRNAME)/include/cppu/helper/purpenv,\
	cppu/helper/purpenv/Environment.hxx \
	cppu/helper/purpenv/Mapping.hxx \
))

$(eval $(call gb_Package_add_files,cppu_odk_headers,$(gb_Package_SDKDIRNAME)/include/typelib,\
	typelib/typeclass.h \
	typelib/typedescription.h \
	typelib/typedescription.hxx \
	typelib/uik.h \
))

$(eval $(call gb_Package_add_files,cppu_odk_headers,$(gb_Package_SDKDIRNAME)/include/uno,\
	uno/any2.h \
	uno/current_context.h \
	uno/current_context.hxx \
	uno/data.h \
	uno/dispatcher.h \
	uno/dispatcher.hxx \
	uno/Enterable.h \
	uno/EnvDcp.h \
	uno/environment.h \
	uno/environment.hxx \
	uno/lbnames.h \
	uno/mapping.h \
	uno/mapping.hxx \
	uno/sequence2.h \
	uno/threadpool.h \
))

# vim: set noet sw=4 ts=4:
