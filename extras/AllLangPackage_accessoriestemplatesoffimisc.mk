# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesoffimisc,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/offimisc/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesoffimisc,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesoffimisc,$(LIBO_SHARE_FOLDER)/template,offimisc,\
	cs/dummy_templates.txt \
	de/01_graph_paper.ott \
	de/01_invoice.ott \
	de/01_minutes.ott \
	de/01_notes.ott \
	de/01_presentation_background.otp \
	de/01_press_release.ott \
	de/02_graph_paper.ott \
	de/02_invoice.ott \
	de/02_minutes.ott \
	de/02_notes.ott \
	de/02_presentation_background.otp \
	de/02_press_release.ott \
	de/03_graph_paper.ott \
	de/03_invoice.ott \
	de/03_minutes.ott \
	de/03_notes.ott \
	de/03_presentation_background.otp \
	de/03_press_release.ott \
	de/04_graph_paper.ott \
	de/04_invoice.ott \
	de/04_minutes.ott \
	de/04_notes.ott \
	de/04_presentation_background.otp \
	de/04_press_release.ott \
	de/05_graph_paper.ott \
	de/05_invoice.ott \
	de/05_minutes.ott \
	de/05_notes.ott \
	de/05_presentation_background.otp \
	de/05_press_release.ott \
	de/06_graph_paper.ott \
	de/06_invoice.ott \
	de/06_minutes.ott \
	de/06_notes.ott \
	de/06_presentation_background.otp \
	de/06_press_release.ott \
	de/agenda.ott \
	de/benzinrechner.ots \
	de/ergebnisprotokoll_handschriftlich.ott \
	de/ergebnisprotokoll.ott \
	de/fahrtenbuch.ots \
	de/Projektarbeit.ott \
	de/projektauftrag.ott \
	de/projektplanung.ots \
	de/Stunden-_und_Vorlesungsplan.ots \
	de/Stundenzettel.ots \
	de/todo_liste_handschriftlich.ott \
	de/Zeiterfassungsbogen.ots \
	en-US/01_graph_paper.ott \
	en-US/01_invoice.ott \
	en-US/01_minutes.ott \
	en-US/01_notes.ott \
	en-US/01_presentation_background.otp \
	en-US/01_press_release.ott \
	en-US/02_graph_paper.ott \
	en-US/02_invoice.ott \
	en-US/02_minutes.ott \
	en-US/02_notes.ott \
	en-US/02_presentation_background.otp \
	en-US/02_press_release.ott \
	en-US/03_graph_paper.ott \
	en-US/03_invoice.ott \
	en-US/03_minutes.ott \
	en-US/03_notes.ott \
	en-US/03_presentation_background.otp \
	en-US/03_press_release.ott \
	en-US/04_graph_paper.ott \
	en-US/04_invoice.ott \
	en-US/04_minutes.ott \
	en-US/04_notes.ott \
	en-US/04_presentation_background.otp \
	en-US/04_press_release.ott \
	en-US/05_graph_paper.ott \
	en-US/05_invoice.ott \
	en-US/05_minutes.ott \
	en-US/05_notes.ott \
	en-US/05_presentation_background.otp \
	en-US/05_press_release.ott \
	en-US/06_graph_paper.ott \
	en-US/06_invoice.ott \
	en-US/06_minutes.ott \
	en-US/06_notes.ott \
	en-US/06_presentation_background.otp \
	en-US/06_press_release.ott \
	en-US/attendanceCalendar.ots \
	en-US/Blue_Report.ott \
	en-US/Business_Letter.ott \
	en-US/Checklist.ott \
	en-US/CorporateReport.ott \
	en-US/DailyToDo.ott \
	en-US/Humanist_Report_A4.ott \
	en-US/Humanist_Report_Letter.ott \
	en-US/MeetingFollowUp.ott \
	en-US/MeetingPreparation.ott \
	en-US/Membership_Fee_payment_status.ots \
	en-US/org_chart.ott \
	en-US/report_3.ott \
	en-US/Report_Or_Paper.ott \
	en-US/roster.ots \
	en-US/Weekly_Timesheet.ots \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/diagramfluxfr.otg \
	fr/FeuillemetreItalienne.ott \
	fr/FeuillemetrePaysage.ott \
	fr/Lettre-EnveloppeFenetreC65.ott \
	fr/logigramfr.otg \
	fr/logigramme.otg \
	fr/organigramfr.ott \
	hu/bizalmasjavaslat.odt \
	hu/Blue_Report.ott \
	hu/Checklist.ott \
	hu/Havi_jelenleti_iv.ots \
	hu/Irattarolo_16_3-1.ott \
	hu/Irattarolo_16_3.ott \
	hu/Irattarolo_16_5-1.ott \
	hu/Irattarolo_16_5.ott \
	hu/marginalishirujsag.ott \
	hu/Mobiltelefon-hasznalati_szabalyzat.ott \
	hu/moderncegeshirujsag.ott \
	hu/Posta_ajanlott.otg \
	hu/sajtokozlemeny.ott \
	hu/uzletihirlevel.ott \
	it/dummy_templates.txt \
	ja/company_directory.ots \
	ja/Giziroku.ott \
	ka/dummy_templates.txt \
	nl/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
