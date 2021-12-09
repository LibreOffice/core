# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,ScriptProviderForJava))

$(eval $(call gb_Jar_use_jars,ScriptProviderForJava,\
	libreoffice \
	ScriptFramework \
))

$(eval $(call gb_Jar_set_componentfile,ScriptProviderForJava,scripting/java/ScriptProviderForJava,OOO,services))

$(eval $(call gb_Jar_set_manifest,ScriptProviderForJava,$(SRCDIR)/scripting/java/com/sun/star/script/framework/provider/java/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,ScriptProviderForJava,com))

$(eval $(call gb_Jar_add_sourcefiles,ScriptProviderForJava,\
	scripting/java/com/sun/star/script/framework/provider/java/Resolver \
	scripting/java/com/sun/star/script/framework/provider/java/ScriptDescriptor \
	scripting/java/com/sun/star/script/framework/provider/java/ScriptProviderForJava \
	scripting/java/com/sun/star/script/framework/provider/java/ScriptProxy \
	scripting/java/com/sun/star/script/framework/provider/java/StrictResolver \
))

# vim: set noet sw=4 ts=4:
