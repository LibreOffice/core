# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Package_Package,wizards_basicsrvscriptforge,$(SRCDIR)/wizards/source/scriptforge))

$(eval $(call gb_Package_add_files,wizards_basicsrvscriptforge,$(LIBO_SHARE_FOLDER)/basic/ScriptForge,\
	SF_Array.xba \
	SF_Dictionary.xba \
	SF_Exception.xba \
	SF_FileSystem.xba \
	SF_L10N.xba \
	SF_Platform.xba \
	SF_PythonHelper.xba \
	SF_Region.xba \
	SF_Root.xba \
	SF_Services.xba \
	SF_Session.xba \
	SF_String.xba \
	SF_TextStream.xba \
	SF_Timer.xba \
	SF_UI.xba \
	SF_Utils.xba \
	_CodingConventions.xba \
	_ModuleModel.xba \
	__License.xba \
	dialog.xlb \
	dlgConsole.xdl \
	dlgProgress.xdl \
	script.xlb \
))

$(eval $(call gb_Package_add_files,wizards_basicsrvscriptforge,$(LIBO_SHARE_FOLDER)/Scripts/python,\
	python/ScriptForgeHelper.py \
))

$(eval $(call gb_Package_add_files,wizards_basicsrvscriptforge,$(LIBO_SHARE_FOLDER)/basic/ScriptForge/po,\
	po/ScriptForge.pot \
	po/en.po \
	po/pt.po \
))

$(eval $(call gb_Package_add_files,wizards_basicsrvscriptforge,$(LIBO_LIB_PYUNO_FOLDER),\
	python/scriptforge.py \
	python/scriptforge.pyi \
))
# vim: set noet sw=4 ts=4:
