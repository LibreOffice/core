# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,letter,$(SRCDIR)/wizards/com/sun/star/wizards/letter))

$(eval $(call gb_Pyuno_add_files,letter,wizards/letter,\
	CallWizard.py \
	CGLetter.py \
	CGLetterWizard.py \
	CGPaperElementLocation.py \
	__init__.py \
	LetterDocument.py \
	LetterWizardDialogConst.py \
	LetterWizardDialogImpl.py \
	LetterWizardDialog.py \
	LetterWizardDialogResources.py \
))
$(eval $(call gb_Pyuno_set_componentfile_full,letter,wizards/com/sun/star/wizards/letter/letter,vnd.openoffice.pymodule:wizards.letter,.CallWizard,services))

# vim:set noet sw=4 ts=4:
