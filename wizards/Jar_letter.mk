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

$(eval $(call gb_Jar_Jar,letter))

$(eval $(call gb_Jar_use_jars,letter,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_manifest,letter,$(SRCDIR)/wizards/com/sun/star/wizards/letter/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,letter,com))

$(eval $(call gb_Jar_add_sourcefiles,letter,\
	wizards/com/sun/star/wizards/letter/CallWizard \
	wizards/com/sun/star/wizards/letter/CGLetter \
	wizards/com/sun/star/wizards/letter/CGLetterWizard \
	wizards/com/sun/star/wizards/letter/CGPaperElementLocation \
	wizards/com/sun/star/wizards/letter/LetterDocument \
	wizards/com/sun/star/wizards/letter/LetterWizardDialog \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogConst \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogEvents \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogImpl \
	wizards/com/sun/star/wizards/letter/LetterWizardDialogResources \
	wizards/com/sun/star/wizards/letter/LocaleCodes \
))

$(eval $(call gb_Jar_set_componentfile,letter,wizards/com/sun/star/wizards/letter/letter,OOO))

# vim: set noet sw=4 ts=4:
