# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesnonfreeeducate,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates-nonfree/educate/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesnonfreeeducate,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesnonfreeeducate,$(LIBO_SHARE_FOLDER)/template,educate,\
	cs/dummy_templates-nonfree.txt \
	de/dummy_templates-nonfree.txt \
	en-US/attendance_sheet.ott \
	en-US/class_list.ott \
	en-US/classroom_seating_chart.ott \
	en-US/College_Budget_-_Semester.ots \
	en-US/College_Monthly_Budget.ots \
	en-US/diploma_landscape.ott \
	en-US/diploma_portrait.ott \
	en-US/elementary_record_cards.ott \
	en-US/GPA_Calculator.ots \
	en-US/Grading_Sheet.ots \
	en-US/homework_tracker.ott \
	en-US/honor_student_award.ott \
	en-US/medical_history.ott \
	en-US/permission_slip.ott \
	en-US/School_Budget.ots \
	en-US/student_of_the_month.ott \
	en-US/Student_Reading_Record.ots \
	en-US/substitute_Information.ott \
	en-US/syllabus_1.ott \
	en-US/term_paper.ott \
	en-US/Weekly_Class_Schedule.ots \
	es/dummy_templates-nonfree.txt \
	fi/dummy_templates-nonfree.txt \
	fr/dummy_templates-nonfree.txt \
	hu/dummy_templates-nonfree.txt \
	it/dummy_templates-nonfree.txt \
	ja/dummy_templates-nonfree.txt \
	ka/dummy_templates-nonfree.txt \
	nl/dummy_templates-nonfree.txt \
	pl/dummy_templates-nonfree.txt \
	pt-BR/dummy_templates-nonfree.txt \
	sv/dummy_templates-nonfree.txt \
	tr/dummy_templates-nonfree.txt \
	zh-CN/dummy_templates-nonfree.txt \
))

# vim: set noet sw=4 ts=4:
