# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,scripting))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_targets,scripting,\
	$(if $(ENABLE_JAVA),\
		Jar_HelloWorld \
		Jar_Highlight \
		Jar_MemoryUsage \
		Jar_ScriptFramework \
		$(if $(ENABLE_SCRIPTING_BEANSHELL),\
			Jar_ScriptProviderForBeanShell \
			Rdb_scriptproviderforbeanshell \
		) \
		Jar_ScriptProviderForJava \
		$(if $(ENABLE_SCRIPTING_JAVASCRIPT),\
			Jar_ScriptProviderForJavaScript \
			Rdb_scriptproviderforjavascript \
		) \
		Package_java \
		Package_java_jars \
		$(if $(ENABLE_SCRIPTING_BEANSHELL),Package_ScriptsBeanShell) \
		$(if $(ENABLE_SCRIPTING_JAVASCRIPT),Package_ScriptsJavaScript) \
	) \
	$(if $(DISABLE_SCRIPTING),,Package_scriptbindinglib) \
	$(if $(DISABLE_PYTHON),,\
		Package_scriptproviderforpython \
		Package_ScriptsPython \
		Pyuno_mailmerge \
	) \
	Library_basprov \
	Library_dlgprov \
	Library_protocolhandler \
	Library_scriptframe \
	Library_stringresource \
	Library_vbaevents \
))

endif

# vim: set noet sw=4 ts=4:
