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
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_InternalUnoApi_InternalUnoApi,bootstrap))

$(eval $(call gb_InternalUnoApi_set_types,bootstrap, \
    com.sun.star.beans.PropertyAttribute \
    com.sun.star.beans.XPropertySet \
    com.sun.star.connection.SocketPermission \
    com.sun.star.container.NoSuchElementException \
    com.sun.star.container.XContentEnumerationAccess \
    com.sun.star.container.XEnumeration \
    com.sun.star.container.XEnumerationAccess \
    com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.container.XSet \
    com.sun.star.io.FilePermission \
    com.sun.star.lang.DisposedException \
    com.sun.star.lang.IllegalArgumentException \
    com.sun.star.lang.XComponent \
    com.sun.star.lang.XEventListener \
    com.sun.star.lang.XInitialization \
    com.sun.star.lang.XMultiComponentFactory \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.lang.XServiceInfo \
    com.sun.star.lang.XSingleComponentFactory \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.lang.XTypeProvider \
    com.sun.star.lang.XUnoTunnel \
    com.sun.star.loader.XImplementationLoader \
    com.sun.star.reflection.XArrayTypeDescription \
    com.sun.star.reflection.XCompoundTypeDescription \
    com.sun.star.reflection.XConstantsTypeDescription \
    com.sun.star.reflection.XConstantTypeDescription \
    com.sun.star.reflection.XEnumTypeDescription \
    com.sun.star.reflection.XIndirectTypeDescription \
    com.sun.star.reflection.XInterfaceAttributeTypeDescription \
    com.sun.star.reflection.XInterfaceAttributeTypeDescription2 \
    com.sun.star.reflection.XInterfaceMemberTypeDescription \
    com.sun.star.reflection.XInterfaceMethodTypeDescription \
    com.sun.star.reflection.XInterfaceTypeDescription \
    com.sun.star.reflection.XInterfaceTypeDescription2 \
    com.sun.star.reflection.XMethodParameter \
    com.sun.star.reflection.XModuleTypeDescription \
    com.sun.star.reflection.XParameter \
    com.sun.star.reflection.XPropertyTypeDescription \
    com.sun.star.reflection.XPublished \
    com.sun.star.reflection.XServiceConstructorDescription \
    com.sun.star.reflection.XServiceTypeDescription \
    com.sun.star.reflection.XServiceTypeDescription2 \
    com.sun.star.reflection.XSingletonTypeDescription \
    com.sun.star.reflection.XSingletonTypeDescription2 \
    com.sun.star.reflection.XStructTypeDescription \
    com.sun.star.reflection.XTypeDescription \
    com.sun.star.reflection.XTypeDescriptionEnumerationAccess \
    com.sun.star.reflection.XUnionTypeDescription \
    com.sun.star.registry.XImplementationRegistration2 \
    com.sun.star.registry.XRegistryKey \
    com.sun.star.registry.XSimpleRegistry \
    com.sun.star.security.AllPermission \
    com.sun.star.security.RuntimePermission \
    com.sun.star.security.XAccessController \
    com.sun.star.security.XPolicy \
    com.sun.star.uno.DeploymentException \
    com.sun.star.uno.RuntimeException \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XAggregation \
    com.sun.star.uno.XComponentContext \
    com.sun.star.uno.XCurrentContext \
    com.sun.star.uno.XInterface \
    com.sun.star.uno.XUnloadingPreference \
    com.sun.star.uno.XWeak \
    com.sun.star.util.XMacroExpander \
))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
