# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplateseducate,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/educate/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplateseducate,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplateseducate,$(LIBO_SHARE_FOLDER)/template,educate,\
	cs/dummy_templates.txt \
	de/Jahres_Schulnoten.ots \
	de/Klassen-Schulnoten-Verwaltung_Noten_1-6.ots \
	de/Kursnoten_15_Punkte.ots \
	de/Meine_Schulnoten.ots \
	de/Schulnoten.ots \
	de/Stundenplan.ots \
	de/stundenplan_und_vorlesungsplan.ots \
	en-US/apa_4thEd.ott \
	en-US/bookmark_1.ott \
	en-US/bookmark_2.ott \
	en-US/classroom_schedule.ott \
	en-US/Crystal.ots \
	en-US/disciplinary_assignment.ott \
	en-US/disciplinary_referral.ott \
	en-US/grades.ots \
	en-US/lesson_plan_1.ott \
	en-US/lesson_plan_2.ott \
	en-US/name_badge_1.ott \
	en-US/student_schedule.ott \
	en-US/table_tent_1.ott \
	en-US/Thesis.ott \
	fr/AgendaSemaine1.1.ots \
	fr/carnet_de_notes.ots \
	fr/documentfinalTP-1.1.ott \
	fr/note_moyenne.ots \
	hu/essze.ott \
	hu/Meresi_jegyzokonyv_-_Iskolai.ott \
	hu/osztalynevsor.ott \
	hu/thesis_k.ott \
	ja/timetable_for_school.ots \
))

# vim: set noet sw=4 ts=4:
