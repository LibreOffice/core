# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_javadoc,$(call gb_CustomTarget_get_workdir,odk/docs/java/ref)))

$(eval $(call gb_Package_set_outdir,odk_javadoc,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,odk_javadoc,$(gb_Package_SDKDIRNAME)/docs/java/ref,\
	allclasses-frame.html \
	allclasses-noframe.html \
	com/sun/star/comp/helper/Bootstrap.html \
	com/sun/star/comp/helper/BootstrapException.html \
	com/sun/star/comp/helper/ComponentContext.html \
	com/sun/star/comp/helper/ComponentContextEntry.html \
	com/sun/star/comp/helper/SharedLibraryLoader.html \
	com/sun/star/comp/helper/class-use/Bootstrap.html \
	com/sun/star/comp/helper/class-use/BootstrapException.html \
	com/sun/star/comp/helper/class-use/ComponentContext.html \
	com/sun/star/comp/helper/class-use/ComponentContextEntry.html \
	com/sun/star/comp/helper/class-use/SharedLibraryLoader.html \
	com/sun/star/comp/helper/package-frame.html \
	com/sun/star/comp/helper/package-summary.html \
	com/sun/star/comp/helper/package-tree.html \
	com/sun/star/comp/helper/package-use.html \
	com/sun/star/lib/uno/helper/ComponentBase.html \
	com/sun/star/lib/uno/helper/Factory.html \
	com/sun/star/lib/uno/helper/InterfaceContainer.html \
	com/sun/star/lib/uno/helper/MultiTypeInterfaceContainer.html \
	com/sun/star/lib/uno/helper/PropertySet.html \
	com/sun/star/lib/uno/helper/PropertySetMixin.BoundListeners.html \
	com/sun/star/lib/uno/helper/PropertySetMixin.html \
	com/sun/star/lib/uno/helper/UnoUrl.html \
	com/sun/star/lib/uno/helper/WeakAdapter.html \
	com/sun/star/lib/uno/helper/WeakBase.html \
	com/sun/star/lib/uno/helper/class-use/ComponentBase.html \
	com/sun/star/lib/uno/helper/class-use/Factory.html \
	com/sun/star/lib/uno/helper/class-use/InterfaceContainer.html \
	com/sun/star/lib/uno/helper/class-use/MultiTypeInterfaceContainer.html \
	com/sun/star/lib/uno/helper/class-use/PropertySet.html \
	com/sun/star/lib/uno/helper/class-use/PropertySetMixin.BoundListeners.html \
	com/sun/star/lib/uno/helper/class-use/PropertySetMixin.html \
	com/sun/star/lib/uno/helper/class-use/UnoUrl.html \
	com/sun/star/lib/uno/helper/class-use/WeakAdapter.html \
	com/sun/star/lib/uno/helper/class-use/WeakBase.html \
	com/sun/star/lib/uno/helper/package-frame.html \
	com/sun/star/lib/uno/helper/package-summary.html \
	com/sun/star/lib/uno/helper/package-tree.html \
	com/sun/star/lib/uno/helper/package-use.html \
	com/sun/star/lib/unoloader/UnoClassLoader.html \
	com/sun/star/lib/unoloader/UnoLoader.html \
	com/sun/star/lib/unoloader/class-use/UnoClassLoader.html \
	com/sun/star/lib/unoloader/class-use/UnoLoader.html \
	com/sun/star/lib/unoloader/package-frame.html \
	com/sun/star/lib/unoloader/package-summary.html \
	com/sun/star/lib/unoloader/package-tree.html \
	com/sun/star/lib/unoloader/package-use.html \
	com/sun/star/lib/util/UrlToFileMapper.html \
	com/sun/star/lib/util/class-use/UrlToFileMapper.html \
	com/sun/star/lib/util/package-frame.html \
	com/sun/star/lib/util/package-summary.html \
	com/sun/star/lib/util/package-tree.html \
	com/sun/star/lib/util/package-use.html \
	com/sun/star/uno/Any.html \
	com/sun/star/uno/AnyConverter.html \
	com/sun/star/uno/Ascii.html \
	com/sun/star/uno/AsciiString.html \
	com/sun/star/uno/Enum.html \
	com/sun/star/uno/IBridge.html \
	com/sun/star/uno/IEnvironment.html \
	com/sun/star/uno/IFieldDescription.html \
	com/sun/star/uno/IMapping.html \
	com/sun/star/uno/IMemberDescription.html \
	com/sun/star/uno/IMethodDescription.html \
	com/sun/star/uno/IQueryInterface.html \
	com/sun/star/uno/ITypeDescription.html \
	com/sun/star/uno/MappingException.html \
	com/sun/star/uno/Type.html \
	com/sun/star/uno/Union.html \
	com/sun/star/uno/UnoRuntime.html \
	com/sun/star/uno/WeakReference.html \
	com/sun/star/uno/class-use/Any.html \
	com/sun/star/uno/class-use/AnyConverter.html \
	com/sun/star/uno/class-use/Ascii.html \
	com/sun/star/uno/class-use/AsciiString.html \
	com/sun/star/uno/class-use/Enum.html \
	com/sun/star/uno/class-use/IBridge.html \
	com/sun/star/uno/class-use/IEnvironment.html \
	com/sun/star/uno/class-use/IFieldDescription.html \
	com/sun/star/uno/class-use/IMapping.html \
	com/sun/star/uno/class-use/IMemberDescription.html \
	com/sun/star/uno/class-use/IMethodDescription.html \
	com/sun/star/uno/class-use/IQueryInterface.html \
	com/sun/star/uno/class-use/ITypeDescription.html \
	com/sun/star/uno/class-use/MappingException.html \
	com/sun/star/uno/class-use/Type.html \
	com/sun/star/uno/class-use/Union.html \
	com/sun/star/uno/class-use/UnoRuntime.html \
	com/sun/star/uno/class-use/WeakReference.html \
	com/sun/star/uno/package-frame.html \
	com/sun/star/uno/package-summary.html \
	com/sun/star/uno/package-tree.html \
	com/sun/star/uno/package-use.html \
	constant-values.html \
	deprecated-list.html \
	help-doc.html \
	index-files/index-1.html \
	index-files/index-10.html \
	index-files/index-11.html \
	index-files/index-12.html \
	index-files/index-13.html \
	index-files/index-14.html \
	index-files/index-15.html \
	index-files/index-16.html \
	index-files/index-17.html \
	index-files/index-18.html \
	index-files/index-19.html \
	index-files/index-2.html \
	index-files/index-20.html \
	index-files/index-21.html \
	index-files/index-3.html \
	index-files/index-4.html \
	index-files/index-5.html \
	index-files/index-6.html \
	index-files/index-7.html \
	index-files/index-8.html \
	index-files/index-9.html \
	index.html \
	overview-frame.html \
	overview-summary.html \
	overview-tree.html \
	package-list \
	resources/background.gif \
	resources/tab.gif \
	resources/titlebar.gif \
	resources/titlebar_end.gif \
	serialized-form.html \
	stylesheet.css \
))

# vim: set noet sw=4 ts=4:
