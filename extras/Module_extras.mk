# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,extras))

$(eval $(call gb_Module_add_targets,extras,\
	CustomTarget_autocorr \
	CustomTarget_autotextuser \
	CustomTarget_glade \
	Package_autocorr \
	Package_autotextuser \
	Package_cfgsrvnolang \
	Package_cfgusr \
	Package_database \
	Package_databasebiblio \
	Package_fonts \
	Package_gallbullets \
	Package_gallhtmlexpo \
	Package_gallmytheme \
	Package_gallroot \
	Package_gallsystem \
	Package_gallwwwgraf \
	Package_glade \
	Package_labels \
	$(if $(filter WNT,$(OS)),Package_newfiles) \
	Package_palettes \
	Package_tplofficorr \
	Package_tploffimisc \
	Package_tplpresnt \
	Package_tpl_styles \
	Package_tplpersonal \
	Package_tplwizagenda \
	Package_tplwizbitmap \
	Package_tplwizdesktop \
	Package_tplwizfax \
	Package_tplwizletter \
	Package_tplwizreport \
	Package_tplwizstyles \
	Package_wordbook \
))

$(eval $(call gb_Module_add_l10n_targets,extras,\
	CustomTarget_autotextshare \
	AllLangPackage_autotextshare \
))

ifneq ($(WITH_GALLERY_BUILD),)
$(eval $(call gb_Module_add_targets,extras,\
	Gallery_arrows \
	Gallery_backgrounds \
	Gallery_computers \
	Gallery_diagrams \
	Gallery_education \
	Gallery_environment \
	Gallery_finance \
	Gallery_people \
	Gallery_symbols \
	Gallery_sound \
	Gallery_txtshapes \
	Gallery_transportation \
))
endif

# vim: set noet sw=4 ts=4:
