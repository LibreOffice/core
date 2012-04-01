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

gb_PARTIALBUILD:=T
include $(GBUILDDIR)/gbuild.mk

all: private_types.flag unotypes.flag

# private_types
private_types.flag: private_types.rdb $(gb_UnoApiTarget_CPPUMAKERTARGET)
	$(call gb_Helper_abbreviate_dirs_native, \
	    $(call gb_Helper_execute,cppumaker) -O. -BUCR -C $< \
			-Tcppuhelper.detail.XExceptionThrower \
			-X$(OUTDIR_FOR_BUILD)/bin/types.rdb)
	touch $@

private_types.rdb: XExceptionThrower.urd
	$(call gb_Helper_abbreviate_dirs_native, \
	    $(call gb_Helper_execute,regmerge) $@ /UCR $< \
			$(OUTDIR_FOR_BUILD)/bin/types.rdb)

XExceptionThrower.urd: $(SRCDIR)/cppuhelper/unotypes/cppuhelper/detail/XExceptionThrower.idl
	$(call gb_Helper_abbreviate_dirs_native, \
		$(call gb_Helper_execute,idlc) -I. -I$(OUTDIR_FOR_BUILD)/idl -O. $<)

# unotypes
unotypes.flag: $(gb_UnoApiTarget_CPPUMAKERTARGET)
	$(call gb_Helper_abbreviate_dirs_native, \
		$(call gb_Helper_execute,cppumaker) -O. -BUCR -C \
			$(foreach type,$(UNOTYPES),-T$(type)) $(OUTDIR_FOR_BUILD)/bin/udkapi.rdb)
	touch $@

UNOTYPES := \
	com.sun.star.beans.PropertyAttribute \
	com.sun.star.beans.PropertyValue \
	com.sun.star.beans.XFastPropertySet \
	com.sun.star.beans.XMultiPropertySet \
	com.sun.star.beans.XPropertyAccess \
	com.sun.star.beans.XPropertySet \
	com.sun.star.beans.XPropertySetOption \
	com.sun.star.bridge.UnoUrlResolver \
	com.sun.star.bridge.XUnoUrlResolver \
	com.sun.star.connection.SocketPermission \
	com.sun.star.container.XElementAccess \
	com.sun.star.container.XEnumerationAccess \
	com.sun.star.container.XHierarchicalNameAccess \
	com.sun.star.container.XNameAccess \
	com.sun.star.container.XNameContainer \
	com.sun.star.container.XSet \
	com.sun.star.io.FilePermission \
	com.sun.star.io.IOException \
	com.sun.star.lang.DisposedException \
	com.sun.star.lang.WrappedTargetRuntimeException \
	com.sun.star.lang.XComponent \
	com.sun.star.lang.XEventListener \
	com.sun.star.lang.XInitialization \
	com.sun.star.lang.XMultiComponentFactory \
	com.sun.star.lang.XMultiServiceFactory \
	com.sun.star.lang.XServiceInfo \
	com.sun.star.lang.XSingleComponentFactory \
	com.sun.star.lang.XSingleServiceFactory \
	com.sun.star.lang.XTypeProvider \
	com.sun.star.loader.XImplementationLoader \
	com.sun.star.reflection.XArrayTypeDescription \
	com.sun.star.reflection.XCompoundTypeDescription \
	com.sun.star.reflection.XEnumTypeDescription \
	com.sun.star.reflection.XIdlClass \
	com.sun.star.reflection.XIdlField2 \
	com.sun.star.reflection.XIdlReflection \
	com.sun.star.reflection.XIndirectTypeDescription \
	com.sun.star.reflection.XInterfaceAttributeTypeDescription \
	com.sun.star.reflection.XInterfaceAttributeTypeDescription2 \
	com.sun.star.reflection.XInterfaceMemberTypeDescription \
	com.sun.star.reflection.XInterfaceMethodTypeDescription \
	com.sun.star.reflection.XInterfaceTypeDescription2 \
	com.sun.star.reflection.XMethodParameter \
	com.sun.star.reflection.XStructTypeDescription \
	com.sun.star.reflection.XTypeDescription \
	com.sun.star.reflection.XUnionTypeDescription \
	com.sun.star.registry.XImplementationRegistration \
	com.sun.star.registry.XRegistryKey \
	com.sun.star.registry.XSimpleRegistry \
	com.sun.star.security.RuntimePermission \
	com.sun.star.security.XAccessController \
	com.sun.star.uno.DeploymentException \
	com.sun.star.uno.RuntimeException \
	com.sun.star.uno.XAggregation \
	com.sun.star.uno.XComponentContext \
	com.sun.star.uno.XCurrentContext \
	com.sun.star.uno.XUnloadingPreference \
	com.sun.star.uno.XWeak \
	com.sun.star.util.XMacroExpander

.DEFAULT_GOAL:=all
.PHONY: all
# vim: set noet sw=4 ts=4:
