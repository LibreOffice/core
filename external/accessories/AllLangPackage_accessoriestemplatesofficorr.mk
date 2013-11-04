# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesofficorr,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/officorr/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesofficorr,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesofficorr,$(LIBO_SHARE_FOLDER)/template,officorr,\
	cs/dummy_templates.txt \
	de/01_business_fax.ott \
	de/01_business_letter.ott \
	de/02_business_fax.ott \
	de/02_business_letter.ott \
	de/03_business_fax.ott \
	de/03_business_letter.ott \
	de/04_business_fax.ott \
	de/04_business_letter.ott \
	de/05_business_fax.ott \
	de/05_business_letter.ott \
	de/06_business_fax.ott \
	de/06_business_letter.ott \
	de/74011-mygbrief.odt \
	de/basis_briefvorlage.ott \
	de/begleitanschreiben.ott \
	de/bewerbung_vorlage.ott \
	de/Brief.ott \
	de/briefumschlag_c4_kuvert_querformat.ott \
	de/briefvorlage.ott \
	de/din_brief_geschaeftlich.ott \
	de/dinbrief.ott \
	de/eleganter_tabellarischer_lebenslauf.ott \
	de/emailformular.ott \
	de/entschuldigung.ott \
	de/foermliches_privatfax_dringend.ott \
	de/foermliches_privatfax.ott \
	de/geschaeftsbrief_ch.ott \
	de/klassischer_brief.ott \
	de/klassischer_tabellarischer_lebenslauf.ott \
	de/klassisch_moderner_brief.ott \
	de/lebenslauf2.ott \
	de/lebenslauf3.ott \
	de/lebenslauf.ott \
	de/maengelanzeige_und_mietminderung.ott \
	de/memo1.ott \
	de/moderner_brief.ott \
	de/moderner_tabellarischer_lebenslauf.ott \
	de/notiz.ott \
	de/postkarte.ott \
	de/privatbrief_ch.ott \
	de/privatbrief_mit_feldern.ott \
	de/privatfax.ott \
	de/Standard_Geschaeftlich_Neutral.ott \
	de/stellenbewerbung.ott \
	de/zahlungsaufforderung.ott \
	en-US/01_business_fax.ott \
	en-US/01_business_letter.ott \
	en-US/02_business_fax.ott \
	en-US/02_business_letter.ott \
	en-US/03_business_fax.ott \
	en-US/03_business_letter.ott \
	en-US/04_business_fax.ott \
	en-US/04_business_letter.ott \
	en-US/05_business_fax.ott \
	en-US/05_business_letter.ott \
	en-US/06_business_fax.ott \
	en-US/06_business_letter.ott \
	en-US/bar_bc.ott \
	en-US/bar_env.ott \
	en-US/bar_label.ott \
	en-US/bar_lh.ott \
	en-US/bar_memo.ott \
	en-US/buscard_1.ott \
	en-US/buscard_2.ott \
	en-US/Business_Card.otg \
	en-US/businesscards.ott \
	en-US/business_cards_sandy.ott \
	en-US/CGInvoice.ott \
	en-US/env_1.ott \
	en-US/env_2.ott \
	en-US/Envelope_fancy.ott \
	en-US/FaxCoverLetter2.ott \
	en-US/Fax_Cover.ott \
	en-US/fax_cover_sandy.ott \
	en-US/faxCoverSheet.ott \
	en-US/Globe_-_Business_Card_1.ott \
	en-US/Globe_-_Business_Card_2.ott \
	en-US/id_1_10_envelope.ott \
	en-US/id_1_business_card.ott \
	en-US/id_1_fax_cover.ott \
	en-US/id_1_Invoice_form.ott \
	en-US/id_1_letterhead.ott \
	en-US/id_1_press_release.ott \
	en-US/id_1_purchase_order.ott \
	en-US/id_1_statement.ott \
	en-US/id_2_10_envelope.ott \
	en-US/id_2_business_card.ott \
	en-US/id_2_fax_cover.ott \
	en-US/id_2_Invoice_form.ott \
	en-US/id_2_letterhead.ott \
	en-US/id_2_press_release.ott \
	en-US/id_2_purchase_order.ott \
	en-US/invoice.billing.ots \
	en-US/invoice.ots \
	en-US/InvoiceTemplate.ots \
	en-US/Job_Invoice.ott \
	en-US/label_1.ott \
	en-US/label_2.ott \
	en-US/Labels_fancy.ott \
	en-US/LAFBusinessLetter.ott \
	en-US/letterhead_sandy.ott \
	en-US/lh_1.ott \
	en-US/lh_2.ott \
	en-US/memo_1.ott \
	en-US/memo1.ott \
	en-US/memo_2.ott \
	en-US/organica_business_cards.ott \
	en-US/organica_fax_form.ott \
	en-US/organica_file_folder_label.ott \
	en-US/organica_invoice_form.ott \
	en-US/organica_letter.ott \
	en-US/organica_mailing_labels.ott \
	en-US/organica_return_address_labels.ott \
	en-US/organica_shipping_labels.ott \
	en-US/PastDueNotice1.ott \
	en-US/PastDueNotice2.ott \
	en-US/Personal_Business_Cards_A4.ott \
	en-US/salesinvoice.ots \
	en-US/ServiceTermination.ott \
	en-US/simplefax.ott \
	en-US/Single_Page_Fax.ott \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/cartvprofr.ott \
	fr/entetelettrefr.ott \
	fr/facturefr.ott \
	fr/newsletter_2fr.ott \
	hu/23755-level_sablon.ott \
	hu/Buscard_1.ott \
	hu/Hivatali_dokumentum_2_o_oldalszamozva.ott \
	hu/Hivatali_dokumentum_elolabban_oldalszamozva_vonallal.ott \
	hu/hivataloslevelartdeco.ott \
	hu/hivataloslevelformazott.ott \
	hu/hivataloslevelirogep.ott \
	hu/hivataloslevelklasszikus.ott \
	hu/memo.ott \
	hu/Meresi_jegyzokonyv_-_Ceges.ott \
	hu/nemetuzletilev.ott \
	hu/Nevjegykartya_alap.ott \
	hu/normalfax.ott \
	it/Lettera_Formale_Stile_Macchina_Da_Scrivere1.ott \
	ja/fax_001.ott \
	ja/fax.ott \
	ja/invoice.ots \
	ja/stock_sheet.ots \
	ka/dummy_templates.txt \
	nl/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
