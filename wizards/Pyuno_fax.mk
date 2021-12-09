# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pyuno_Pyuno,fax,$(SRCDIR)/wizards/com/sun/star/wizards/fax))

$(eval $(call gb_Pyuno_add_files,fax,wizards/fax,\
    __init__.py \
    CallWizard.py \
    CGFax.py \
    CGFaxWizard.py \
    FaxDocument.py \
    FaxWizardDialogConst.py \
    FaxWizardDialogImpl.py \
    FaxWizardDialog.py \
    FaxWizardDialogResources.py \
))
$(eval $(call gb_Pyuno_set_componentfile_full,fax,wizards/com/sun/star/wizards/fax/fax,vnd.openoffice.pymodule:wizards.fax,.CallWizard,services))

# vim:set noet sw=4 ts=4:
