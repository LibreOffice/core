# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_AllLangResTarget_AllLangResTarget,pcr))

$(eval $(call gb_AllLangResTarget_set_reslocation,pcr,extensions))

$(eval $(call gb_AllLangResTarget_add_srs,pcr,\
	pcr/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,pcr/res))

$(eval $(call gb_SrsTarget_set_include,pcr/res,\
	-I$(SRCDIR)/extensions/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_SrsTarget_add_files,pcr/res,\
	extensions/source/propctrlr/propres.src \
	extensions/source/propctrlr/formres.src \
	extensions/source/propctrlr/pcrmiscres.src \
	extensions/source/propctrlr/fontdialog.src \
	extensions/source/propctrlr/selectlabeldialog.src \
	extensions/source/propctrlr/formlinkdialog.src \
	extensions/source/propctrlr/listselectiondlg.src \
	extensions/source/propctrlr/newdatatype.src \
))

# vim:set noet sw=4 ts=4:
