# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
    ridljar/com/sun/star/uno/UnoRuntime \
))

# vim:set noet sw=4 ts=4:
