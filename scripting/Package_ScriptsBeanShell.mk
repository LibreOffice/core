# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,scripting_ScriptsBeanShell,$(SRCDIR)/scripting/examples))

$(eval $(call gb_Package_set_outdir,scripting_ScriptsBeanShell,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,scripting_ScriptsBeanShell,$(LIBO_SHARE_FOLDER)/Scripts,\
	beanshell/Capitalise/capitalise.bsh \
	beanshell/Capitalise/parcel-descriptor.xml \
	beanshell/HelloWorld/helloworld.bsh \
	beanshell/HelloWorld/parcel-descriptor.xml \
	beanshell/Highlight/ButtonPressHandler.bsh \
	beanshell/Highlight/highlighter.bsh \
	beanshell/Highlight/parcel-descriptor.xml \
	beanshell/Highlight/ShowDialog.bsh \
	beanshell/MemoryUsage/memusage.bsh \
	beanshell/MemoryUsage/parcel-descriptor.xml \
	beanshell/WordCount/parcel-descriptor.xml \
	beanshell/WordCount/wordcount.bsh \
))

# vim: set noet sw=4 ts=4:
