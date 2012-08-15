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

$(eval $(call gb_Jar_Jar,table))

$(eval $(call gb_Jar_use_jars,table,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_packageroot,table,com))

$(eval $(call gb_Jar_set_manifest,table,$(SRCDIR)/wizards/com/sun/star/wizards/table/MANIFEST.MF))

$(eval $(call gb_Jar_add_sourcefiles,table,\
	wizards/com/sun/star/wizards/table/CallTableWizard \
	wizards/com/sun/star/wizards/table/CGCategory \
	wizards/com/sun/star/wizards/table/CGTable \
	wizards/com/sun/star/wizards/table/FieldDescription \
	wizards/com/sun/star/wizards/table/FieldFormatter \
	wizards/com/sun/star/wizards/table/Finalizer \
	wizards/com/sun/star/wizards/table/PrimaryKeyHandler \
	wizards/com/sun/star/wizards/table/ScenarioSelector \
	wizards/com/sun/star/wizards/table/TableWizard \
))

$(eval $(call gb_Jar_set_componentfile,table,wizards/com/sun/star/wizards/table/table,OOO))

# vim: set noet sw=4 ts=4:
