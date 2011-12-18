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

$(eval $(call gb_Package_Package,cppu_inc,$(SRCDIR)/cppu/inc))

$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Any.h,com/sun/star/uno/Any.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Any.hxx,com/sun/star/uno/Any.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/genfunc.h,com/sun/star/uno/genfunc.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/genfunc.hxx,com/sun/star/uno/genfunc.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Reference.h,com/sun/star/uno/Reference.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Reference.hxx,com/sun/star/uno/Reference.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Sequence.h,com/sun/star/uno/Sequence.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Sequence.hxx,com/sun/star/uno/Sequence.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Type.h,com/sun/star/uno/Type.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/com/sun/star/uno/Type.hxx,com/sun/star/uno/Type.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/cppudllapi.h,cppu/cppudllapi.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/EnvDcp.hxx,cppu/EnvDcp.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/Enterable.hxx,cppu/Enterable.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/EnvGuards.hxx,cppu/EnvGuards.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/FreeReference.hxx,cppu/FreeReference.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/macros.hxx,cppu/macros.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/Map.hxx,cppu/Map.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/Shield.hxx,cppu/Shield.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/unotype.hxx,cppu/unotype.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/helper/purpenv/Environment.hxx,cppu/helper/purpenv/Environment.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/cppu/helper/purpenv/Mapping.hxx,cppu/helper/purpenv/Mapping.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/typelib/typeclass.h,typelib/typeclass.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/typelib/typedescription.h,typelib/typedescription.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/typelib/typedescription.hxx,typelib/typedescription.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/typelib/uik.h,typelib/uik.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/any2.h,uno/any2.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/cuno.h,uno/cuno.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/current_context.h,uno/current_context.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/current_context.hxx,uno/current_context.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/data.h,uno/data.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/dispatcher.h,uno/dispatcher.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/dispatcher.hxx,uno/dispatcher.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/Enterable.h,uno/Enterable.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/EnvDcp.h,uno/EnvDcp.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/environment.h,uno/environment.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/environment.hxx,uno/environment.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/lbnames.h,uno/lbnames.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/mapping.h,uno/mapping.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/mapping.hxx,uno/mapping.hxx))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/sequence2.h,uno/sequence2.h))
$(eval $(call gb_Package_add_file,cppu_inc,inc/uno/threadpool.h,uno/threadpool.h))

# vim: set noet sw=4 ts=4:
