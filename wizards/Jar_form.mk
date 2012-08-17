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

$(eval $(call gb_Jar_Jar,form))

$(eval $(call gb_Jar_use_jars,form,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_manifest,form,$(SRCDIR)/wizards/com/sun/star/wizards/form/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,form,com))

$(eval $(call gb_Jar_add_sourcefiles,form,\
	wizards/com/sun/star/wizards/form/CallFormWizard \
	wizards/com/sun/star/wizards/form/DataEntrySetter \
	wizards/com/sun/star/wizards/form/FieldLinker \
	wizards/com/sun/star/wizards/form/Finalizer \
	wizards/com/sun/star/wizards/form/FormConfiguration \
	wizards/com/sun/star/wizards/form/FormControlArranger \
	wizards/com/sun/star/wizards/form/FormDocument \
	wizards/com/sun/star/wizards/form/FormWizard \
	wizards/com/sun/star/wizards/form/StyleApplier \
	wizards/com/sun/star/wizards/form/UIControlArranger \
))

$(eval $(call gb_Jar_set_componentfile,form,wizards/com/sun/star/wizards/form/form,OOO))

# vim: set noet sw=4 ts=4:
