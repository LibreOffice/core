# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriestemplatesforms,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/forms/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriestemplatesforms,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriestemplatesforms,\
	cs/seznam_pohledavek.ots \
	de/Ausbildungsnachweis2.ott \
	de/Ausbildungsnachweis.ott \
	de/ausgabebelege.ott \
	de/Checkliste_Hauswartung.ots \
	de/dienstreiseantrag.ott \
	de/einnahmebelege.ott \
	de/Fahrtenbuch.ots \
	de/Fax_privat_formlich_dringend.ott \
	de/Fax_privat_formlich.ott \
	de/Fax_privat_personlich.ott \
	de/Gesprachsnotiz.ott \
	de/kariertes_blatt_a4_5mm.ots \
	de/kurz_begleitschreiben.ott \
	de/Kurz-Begleitschreiben.ott \
	de/kurzbrief.ott \
	de/lernkarteikarten_4x10_stueck.ott \
	de/pkw_kaufvertrag.ott \
	de/protokoll.ott \
	de/quittungen.ott \
	de/quittung.ots \
	de/rechnung.ots \
	de/reisekostenabrechnung.ott \
	de/smily_calender.ots \
	de/ueberweisungstraeger.ott \
	de/urlaubsplaner_2006.ots \
	de/zeichenblatt_din_en_iso_5457_einfach_ohne_din_schrift.ott \
	en-US/cap1107.ott \
	en-US/Confidentiality_Agreement.ott \
	en-US/contract.ott \
	en-US/Joint_venture.ott \
	en-US/maintenance_agreement.ott \
	en-US/Note_Template.ott \
	en-US/partnership.ott \
	en-US/PleadingGeneric.ott \
	en-US/Pleading.ott \
	en-US/System_maint_tmpl.ott \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/dummy_templates.txt \
	hu/Alairasi_cimpeldany.ott \
	hu/Auto_berbeadasi_megallapodas.ott \
	hu/Autokolcsonzes.ott \
	hu/Beszelgetesi_jegyzet.ott \
	hu/BT_tarsasagi_szerzodes_1.ott \
	hu/Ingatlan-adasveteli_eloszerzodes.ott \
	hu/Ingatlan-adasveteli_szerzodes_haszonelvezet.ott \
	hu/Ingatlan-adasveteli_szerzodes.ott \
	hu/Ingatlan-adasveteli_szerzodes_tarsashaz.ott \
	hu/Ingatlancsere_szerzodes.ott \
	hu/Kiadoi_szerzodes.ott \
	hu/Meghatalmazas_penzfelvetel.ott \
	hu/Reklammu_atruhazasi_szerzodes.ott \
	hu/Szoftver_felhasznalasi_szerzodes.ott \
	hu/Szoftverlegalizacio_alkalmazotti_nyilatkozat.ott \
	hu/Szoftverlegalizacio_etikai_kodex.ott \
	hu/Vegrendelet.ott \
	hu/Veszelyes_allat_tartasi_engedely_kerelem.ott \
	it/lettera_legale.ott \
	it/SchedaCarburante.ott \
	ja/resume.ots \
	ka/dummy_templates.txt \
	nl/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
