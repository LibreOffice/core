# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,autotextshare,$(call gb_CustomTarget_get_workdir,extras/source/autotext)))

$(eval $(call gb_AllLangPackage_add_files,autotextshare,$(LIBO_SHARE_FOLDER)/autotext,\
	af-ZA/standard.bau \
	bg/crdbus50.bau \
	bg/standard.bau \
	bg/template.bau \
	ca/standard.bau \
	cs/crdbus50.bau \
	cs/crdbus54.bau \
	cs/standard.bau \
	cs/template.bau \
	da/crdbus50.bau \
	da/standard.bau \
	da/template.bau \
	de/crdbus50.bau \
	de/standard.bau \
	de/template.bau \
	en-AU/standard.bau \
	en-GB/crdbus50.bau \
	en-GB/standard.bau \
	en-GB/template.bau \
	en-US/crdbus50.bau \
	en-US/standard.bau \
	en-US/template.bau \
	en-ZA/standard.bau \
	es/crdbus50.bau \
	es/standard.bau \
	es/template.bau \
	fa/standard.bau \
	fi/standard.bau \
	fr/crdbus50.bau \
	fr/standard.bau \
	fr/template.bau \
	ga-IE/standard.bau \
	hr/standard.bau \
	hu/crdbus50.bau \
	hu/crdbus54.bau \
	hu/standard.bau \
	hu/template.bau \
	is/standard.bau \
	it/crdbus50.bau \
	it/standard.bau \
	it/template.bau \
	ja/crdbus50.bau \
	ja/standard.bau \
	ja/template.bau \
	km/crdbus50.bau \
	km/standard.bau \
	km/template.bau \
	ko/crdbus50.bau \
	ko/standard.bau \
	ko/template.bau \
	lb-LU/standard.bau \
	lt/standard.bau \
	mn/crdbus50.bau \
	mn/standard.bau \
	mn/template.bau \
	nl-BE/standard.bau \
	nl/crdbus50.bau \
	nl/standard.bau \
	nl/template.bau \
	pl/crdbus50.bau \
	pl/standard.bau \
	pl/template.bau \
	pt-BR/crdbus50.bau \
	pt-BR/standard.bau \
	pt-BR/template.bau \
	pt/crdbus50.bau \
	pt/standard.bau \
	pt/template.bau \
	ro/standard.bau \
	ru/crdbus50.bau \
	ru/standard.bau \
	ru/template.bau \
	sk/crdbus50.bau \
	sk/crdbus54.bau \
	sk/standard.bau \
	sk/template.bau \
	sl/crdbus50.bau \
	sl/crdbus54.bau \
	sl/standard.bau \
	sl/template.bau \
	sr-CS/standard.bau \
	sr-Latn-CS/standard.bau \
	sr-Latn-ME/standard.bau \
	sr-Latn-RS/standard.bau \
	sr-ME/standard.bau \
	sr-RS/standard.bau \
	sv/crdbus50.bau \
	sv/standard.bau \
	sv/template.bau \
	tr/crdbus50.bau \
	tr/standard.bau \
	tr/template.bau \
	vi/standard.bau \
	zh-CN/crdbus50.bau \
	zh-CN/standard.bau \
	zh-CN/template.bau \
	zh-TW/crdbus50.bau \
	zh-TW/standard.bau \
	zh-TW/template.bau \
))

# vim: set noet sw=4 ts=4:
