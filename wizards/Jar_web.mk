# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Jar_Jar,web))

$(eval $(call gb_Jar_use_jars,web,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_packageroot,web,com))

$(eval $(call gb_Jar_set_manifest,web,$(SRCDIR)/wizards/com/sun/star/wizards/web/MANIFEST.MF))

$(eval $(call gb_Jar_add_sourcefiles,web,\
	wizards/com/sun/star/wizards/web/AbstractErrorHandler \
	wizards/com/sun/star/wizards/web/BackgroundsDialog \
	wizards/com/sun/star/wizards/web/CallWizard \
	wizards/com/sun/star/wizards/web/ErrorHandler \
	wizards/com/sun/star/wizards/web/ExtensionVerifier \
	wizards/com/sun/star/wizards/web/FTPDialog \
	wizards/com/sun/star/wizards/web/FTPDialogResources \
	wizards/com/sun/star/wizards/web/IconsDialog \
	wizards/com/sun/star/wizards/web/ImageListDialog \
	wizards/com/sun/star/wizards/web/LogTaskListener \
	wizards/com/sun/star/wizards/web/Process \
	wizards/com/sun/star/wizards/web/ProcessErrorHandler \
	wizards/com/sun/star/wizards/web/ProcessErrors \
	wizards/com/sun/star/wizards/web/ProcessStatusRenderer \
	wizards/com/sun/star/wizards/web/StatusDialog \
	wizards/com/sun/star/wizards/web/StylePreview \
	wizards/com/sun/star/wizards/web/TOCPreview \
	wizards/com/sun/star/wizards/web/WebWizard \
	wizards/com/sun/star/wizards/web/WebWizardConst \
	wizards/com/sun/star/wizards/web/WebWizardDialog \
	wizards/com/sun/star/wizards/web/WebWizardDialogResources \
	wizards/com/sun/star/wizards/web/WWD_Events \
	wizards/com/sun/star/wizards/web/WWD_General \
	wizards/com/sun/star/wizards/web/WWD_Startup \
	wizards/com/sun/star/wizards/web/WWHID \
	wizards/com/sun/star/wizards/web/data/CGArgument \
	wizards/com/sun/star/wizards/web/data/CGContent \
	wizards/com/sun/star/wizards/web/data/CGDesign \
	wizards/com/sun/star/wizards/web/data/CGDocument \
	wizards/com/sun/star/wizards/web/data/CGExporter \
	wizards/com/sun/star/wizards/web/data/CGFilter \
	wizards/com/sun/star/wizards/web/data/CGGeneralInfo \
	wizards/com/sun/star/wizards/web/data/CGIconSet \
	wizards/com/sun/star/wizards/web/data/CGImage \
	wizards/com/sun/star/wizards/web/data/CGLayout \
	wizards/com/sun/star/wizards/web/data/CGPublish \
	wizards/com/sun/star/wizards/web/data/CGSession \
	wizards/com/sun/star/wizards/web/data/CGSessionName \
	wizards/com/sun/star/wizards/web/data/CGSettings \
	wizards/com/sun/star/wizards/web/data/CGStyle \
	wizards/com/sun/star/wizards/web/data/ConfigSetItem \
	wizards/com/sun/star/wizards/web/data/TypeDetection \
	wizards/com/sun/star/wizards/web/export/AbstractExporter \
	wizards/com/sun/star/wizards/web/export/ConfiguredExporter \
	wizards/com/sun/star/wizards/web/export/CopyExporter \
	wizards/com/sun/star/wizards/web/export/Exporter \
	wizards/com/sun/star/wizards/web/export/FilterExporter \
	wizards/com/sun/star/wizards/web/export/ImpressHTMLExporter \
	wizards/com/sun/star/wizards/web/status/ErrorLog \
	wizards/com/sun/star/wizards/web/status/ErrorReporter \
	wizards/com/sun/star/wizards/web/status/LogTaskListener \
	wizards/com/sun/star/wizards/web/status/Task \
	wizards/com/sun/star/wizards/web/status/TaskEvent \
	wizards/com/sun/star/wizards/web/status/TaskListener \
))

$(eval $(call gb_Jar_set_componentfile,web,wizards/com/sun/star/wizards/web/web,OOO))

# vim: set noet sw=4 ts=4:
