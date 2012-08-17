# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
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
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Jar_Jar,ScriptFramework))

$(eval $(call gb_Jar_use_jars,ScriptFramework,\
	juh \
	jurt \
	ridl \
	unoil \
))

$(eval $(call gb_Jar_set_componentfile,ScriptFramework,scripting/java/ScriptFramework,OOO))

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
	scripting/java/com/sun/star/script/framework/provider/NoSuitableClassLoaderException \
	scripting/java/com/sun/star/script/framework/provider/PathUtils \
	scripting/java/com/sun/star/script/framework/provider/ScriptContext \
	scripting/java/com/sun/star/script/framework/provider/ScriptEditor \
	scripting/java/com/sun/star/script/framework/provider/ScriptProvider \
	scripting/java/com/sun/star/script/framework/provider/SwingInvocation \
	scripting/java/Framework/com/sun/star/script/framework/security/SecurityDialog \
))

# vim: set noet sw=4 ts=4:
