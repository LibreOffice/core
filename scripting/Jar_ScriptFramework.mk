# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,ScriptFramework))

$(eval $(call gb_Jar_use_jars,ScriptFramework,\
	libreoffice \
))

$(eval $(call gb_Jar_set_componentfile,ScriptFramework,scripting/java/ScriptFramework,OOO,services))

$(eval $(call gb_Jar_set_manifest,ScriptFramework,$(SRCDIR)/scripting/java/Framework/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,ScriptFramework,com))

$(eval $(call gb_Jar_add_sourcefiles,ScriptFramework,\
	scripting/java/com/sun/star/script/framework/browse/DialogFactory \
	scripting/java/com/sun/star/script/framework/browse/ParcelBrowseNode \
	scripting/java/com/sun/star/script/framework/browse/PkgProviderBrowseNode \
	scripting/java/com/sun/star/script/framework/browse/ProviderBrowseNode \
	scripting/java/com/sun/star/script/framework/browse/ScriptBrowseNode \
	scripting/java/com/sun/star/script/framework/container/DeployedUnoPackagesDB \
	scripting/java/com/sun/star/script/framework/container/ParcelContainer \
	scripting/java/com/sun/star/script/framework/container/ParcelDescriptor \
	scripting/java/com/sun/star/script/framework/container/Parcel \
	scripting/java/com/sun/star/script/framework/container/ParsedScriptUri \
	scripting/java/com/sun/star/script/framework/container/ScriptEntry \
	scripting/java/com/sun/star/script/framework/container/ScriptMetaData \
	scripting/java/com/sun/star/script/framework/container/UnoPkgContainer \
	scripting/java/com/sun/star/script/framework/container/XMLParserFactory \
	scripting/java/com/sun/star/script/framework/container/XMLParser \
	scripting/java/com/sun/star/script/framework/io/UCBStreamHandler \
	scripting/java/com/sun/star/script/framework/io/XInputStreamImpl \
	scripting/java/com/sun/star/script/framework/io/XInputStreamWrapper \
	scripting/java/com/sun/star/script/framework/io/XOutputStreamWrapper \
	scripting/java/com/sun/star/script/framework/io/XStorageHelper \
	scripting/java/com/sun/star/script/framework/log/LogUtils \
	scripting/java/com/sun/star/script/framework/provider/ClassLoaderFactory \
	scripting/java/com/sun/star/script/framework/provider/EditorScriptContext \
	scripting/java/com/sun/star/script/framework/provider/PathUtils \
	scripting/java/com/sun/star/script/framework/provider/ScriptContext \
	scripting/java/com/sun/star/script/framework/provider/ScriptEditor \
	scripting/java/com/sun/star/script/framework/provider/ScriptEditorBase \
	scripting/java/com/sun/star/script/framework/provider/ScriptProvider \
	scripting/java/com/sun/star/script/framework/provider/SwingInvocation \
	scripting/java/Framework/com/sun/star/script/framework/security/SecurityDialog \
))

# vim: set noet sw=4 ts=4:
