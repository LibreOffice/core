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

$(eval $(call gb_Jar_Jar,ScriptProviderForBeanShell))

$(eval $(call gb_Jar_use_jars,ScriptProviderForBeanShell,\
	juh \
	jurt \
	ridl \
	unoil \
	ScriptFramework \
))

$(eval $(call gb_Jar_use_externals,ScriptProviderForBeanShell,\
	bsh \
))

$(eval $(call gb_Jar_set_manifest,ScriptProviderForBeanShell,$(SRCDIR)/scripting/java/com/sun/star/script/framework/provider/beanshell/MANIFEST.MF))

$(eval $(call gb_Jar_set_componentfile,ScriptProviderForBeanShell,scripting/java/ScriptProviderForBeanShell,OOO))

$(eval $(call gb_Jar_set_packageroot,ScriptProviderForBeanShell,com))

$(eval $(call gb_Jar_add_sourcefiles,ScriptProviderForBeanShell,\
	scripting/java/com/sun/star/script/framework/provider/beanshell/PlainSourceView \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptEditorForBeanShell \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptProviderForBeanShell \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptSourceModel \
	scripting/java/com/sun/star/script/framework/provider/beanshell/ScriptSourceView \
))

$(eval $(call gb_Jar_add_packagefile,ScriptProviderForBeanShell,\
	com/sun/star/script/framework/provider/beanshell/template.bsh,\
		$(SRCDIR)/scripting/java/com/sun/star/script/framework/provider/beanshell/template.bsh \
))

# vim: set noet sw=4 ts=4:
