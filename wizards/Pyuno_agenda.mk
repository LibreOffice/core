# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,agenda,$(SRCDIR)/wizards/com/sun/star/wizards/agenda))

$(eval $(call gb_Pyuno_add_files,agenda,wizards/agenda,\
    AgendaDocument.py \
    AgendaWizardDialogConst.py \
    AgendaWizardDialogImpl.py \
    AgendaWizardDialog.py \
    AgendaWizardDialogResources.py \
    CallWizard.py \
    CGAgenda.py \
    CGTopic.py \
    __init__.py \
    TemplateConsts.py \
    TopicsControl.py \
))
$(eval $(call gb_Pyuno_set_componentfile_full,agenda,wizards/com/sun/star/wizards/agenda/agenda,vnd.openoffice.pymodule:wizards.agenda,.CallWizard,services))

# vim:set noet sw=4 ts=4:
