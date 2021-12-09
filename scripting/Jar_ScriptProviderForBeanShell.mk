# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,ScriptProviderForBeanShell))

$(eval $(call gb_Jar_use_jars,ScriptProviderForBeanShell,\
	libreoffice \
	ScriptFramework \
))

$(eval $(call gb_Jar_use_externals,ScriptProviderForBeanShell,\
	bsh \
))

$(eval $(call gb_Jar_set_manifest,ScriptProviderForBeanShell,$(SRCDIR)/scripting/java/com/sun/star/script/framework/provider/beanshell/MANIFEST.MF))

$(eval $(call gb_Jar_set_componentfile,ScriptProviderForBeanShell,scripting/java/ScriptProviderForBeanShell,OOO,scriptproviderforbeanshell))

$(eval $(call gb_Jar_set_packageroot,ScriptProviderForBeanShell,com))

$(eval $(call gb_Jar_add_sourcefiles,ScriptProviderForBeanShell,\
	scripting/java/com/sun/star/script/framework/provider/beanshell/PlainSourceView \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptEditorForBeanShell \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptProviderForBeanShell \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptSourceModel \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptSourceView \
	scripting/java/com/sun/star/script/framework/provider/beanshell/UnsavedChangesListener \
))

$(eval $(call gb_Jar_add_packagefile,ScriptProviderForBeanShell,\
	com/sun/star/script/framework/provider/beanshell/template.bsh,\
		$(SRCDIR)/scripting/java/com/sun/star/script/framework/provider/beanshell/template.bsh \
))

# vim: set noet sw=4 ts=4:
