# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,scripting_ScriptsBeanShell,$(SRCDIR)/scripting/examples))

$(eval $(call gb_Package_add_files_with_dir,scripting_ScriptsBeanShell,$(LIBO_SHARE_FOLDER)/Scripts,\
	beanshell/Calc/CopyRange.bsh \
	beanshell/Calc/FixView.bsh \
	beanshell/Calc/InsertSheet.bsh \
	beanshell/Calc/parcel-descriptor.xml \
	beanshell/Calc/ProtectSheet.bsh \
	beanshell/Calc/SelectCell.bsh \
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
	beanshell/Writer/ChangeFont.bsh \
	beanshell/Writer/ChangeParaAdjust.bsh \
	beanshell/Writer/InsertTable.bsh \
	beanshell/Writer/InsertText.bsh \
	beanshell/Writer/parcel-descriptor.xml \
	beanshell/Writer/SetText.bsh \
))

# vim: set noet sw=4 ts=4:
