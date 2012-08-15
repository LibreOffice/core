# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Jar_Jar,ridl))

$(eval $(call gb_Jar_use_customtargets,ridl,\
    ridljar/javamaker \
))

$(eval $(call gb_Jar_set_packageroot,ridl,com))

$(eval $(call gb_Jar_add_packagedirs,ridl,\
    $(call gb_CustomTarget_get_workdir,ridljar/javamaker)/com \
))

$(eval $(call gb_Jar_add_sourcefiles,ridl,\
    ridljar/com/sun/star/lib/uno/typedesc/FieldDescription \
    ridljar/com/sun/star/lib/uno/typedesc/MemberDescriptionHelper \
    ridljar/com/sun/star/lib/uno/typedesc/MethodDescription \
    ridljar/com/sun/star/lib/uno/typedesc/TypeDescription \
    ridljar/com/sun/star/lib/uno/typeinfo/AttributeTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/ConstantTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/MemberTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/MethodTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/ParameterTypeInfo \
    ridljar/com/sun/star/lib/uno/typeinfo/TypeInfo \
    ridljar/com/sun/star/lib/util/DisposeListener \
    ridljar/com/sun/star/lib/util/DisposeNotifier \
    ridljar/com/sun/star/lib/util/WeakMap \
    ridljar/com/sun/star/uno/Any \
    ridljar/com/sun/star/uno/Enum \
    ridljar/com/sun/star/uno/IBridge \
    ridljar/com/sun/star/uno/IEnvironment \
    ridljar/com/sun/star/uno/IFieldDescription \
    ridljar/com/sun/star/uno/IMapping \
    ridljar/com/sun/star/uno/IMemberDescription \
    ridljar/com/sun/star/uno/IMethodDescription \
    ridljar/com/sun/star/uno/IQueryInterface \
    ridljar/com/sun/star/uno/ITypeDescription \
    ridljar/com/sun/star/uno/Type \
    ridljar/com/sun/star/uno/Union \
    ridljar/com/sun/star/uno/UnoRuntime \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
