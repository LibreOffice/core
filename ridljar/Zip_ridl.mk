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

$(eval $(call gb_Zip_Zip,ridl_src,$(SRCDIR)/ridljar))

$(eval $(call gb_Zip_add_files,ridl_src,\
    com/sun/star/lib/uno/typedesc/FieldDescription.java \
    com/sun/star/lib/uno/typedesc/MemberDescriptionHelper.java \
    com/sun/star/lib/uno/typedesc/MethodDescription.java \
    com/sun/star/lib/uno/typedesc/TypeDescription.java \
    com/sun/star/lib/uno/typeinfo/AttributeTypeInfo.java \
    com/sun/star/lib/uno/typeinfo/ConstantTypeInfo.java \
    com/sun/star/lib/uno/typeinfo/MemberTypeInfo.java \
    com/sun/star/lib/uno/typeinfo/MethodTypeInfo.java \
    com/sun/star/lib/uno/typeinfo/ParameterTypeInfo.java \
    com/sun/star/lib/uno/typeinfo/TypeInfo.java \
    com/sun/star/lib/util/DisposeListener.java \
    com/sun/star/lib/util/DisposeNotifier.java \
    com/sun/star/lib/util/WeakMap.java \
    com/sun/star/uno/Any.java \
    com/sun/star/uno/Enum.java \
    com/sun/star/uno/IBridge.java \
    com/sun/star/uno/IEnvironment.java \
    com/sun/star/uno/IFieldDescription.java \
    com/sun/star/uno/IMapping.java \
    com/sun/star/uno/IMemberDescription.java \
    com/sun/star/uno/IMethodDescription.java \
    com/sun/star/uno/IQueryInterface.java \
    com/sun/star/uno/ITypeDescription.java \
    com/sun/star/uno/Type.java \
    com/sun/star/uno/Union.java \
    com/sun/star/uno/UnoRuntime.java \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
