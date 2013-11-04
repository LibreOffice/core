# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatespersonal,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/personal/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatespersonal,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatespersonal,$(LIBO_SHARE_FOLDER)/template,personal,\
	cs/dummy_templates.txt \
	de/faltkarte.otg \
	de/formalletteraquarius.ott \
	de/formalletteraries.ott \
	de/formalletterat.ott \
	de/formalletterblackwhite.ott \
	de/formallettercancer.ott \
	de/formallettercapricornus.ott \
	de/formallettereye.ott \
	de/formallettergame.ott \
	de/formallettergemini.ott \
	de/formalletterhearts.ott \
	de/formalletterhexagons.ott \
	de/formalletterinkpot.ott \
	de/formalletterleo.ott \
	de/formalletterlibra.ott \
	de/formalletterpisces.ott \
	de/formallettersagittarius.ott \
	de/formalletterscorpius.ott \
	de/formallettertaurus.ott \
	de/formallettervirgo.ott \
	de/formalletteryinyang.ott \
	de/merels_board_game_original.otg \
	de/mitarbeiterplanung_2006.ots \
	de/tabellarischer_lebenslauf.ott \
	de/tabellarischer_lebenslauf_popup.ott \
	de/vitalwerte.ots \
	en-US/48171-Resume-Template.odt \
	en-US/ad_resume1.ott \
	en-US/ad_resume2.ott \
	en-US/ad_resume3.ott \
	en-US/BookLabel_12_4x3-WL-OL150.ott \
	en-US/Country_book6-WL-OL125.ott \
	en-US/Curriculum_Vitae.ott \
	en-US/CV.ott \
	en-US/DailyCalendar.ots \
	en-US/dayreview.ott \
	en-US/diarypage.ott \
	en-US/eventlist.ott \
	en-US/example-resume.ott \
	en-US/faltkarte.otg \
	en-US/formalletteraquarius.ott \
	en-US/formalletteraries.ott \
	en-US/formalletterat.ott \
	en-US/formalletterblackwhite.ott \
	en-US/formallettercancer.ott \
	en-US/formallettercapricornus.ott \
	en-US/formallettereye.ott \
	en-US/formallettergame.ott \
	en-US/formallettergemini.ott \
	en-US/formalletterhearts.ott \
	en-US/formalletterhexagons.ott \
	en-US/formalletterinkpot.ott \
	en-US/formalletterleo.ott \
	en-US/formalletterlibra.ott \
	en-US/formalletterpisces.ott \
	en-US/formallettersagittarius.ott \
	en-US/formalletterscorpius.ott \
	en-US/formallettertaurus.ott \
	en-US/formallettervirgo.ott \
	en-US/formalletteryinyang.ott \
	en-US/greeting_card_kids.otg \
	en-US/home_buying_estimator.ots \
	en-US/Household_Budget.ots \
	en-US/humanist_resume_a4.ott \
	en-US/humanist_resume_letter.ott \
	en-US/Job_Application.ott \
	en-US/kaledeiscope-bk-OL150.ott \
	en-US/merels_board_game_original.otg \
	en-US/MonthlyBudget.ots \
	en-US/OpenOffice_Budget_Template.ots \
	en-US/packinglist.ott \
	en-US/personal-budget.ots \
	en-US/personalCoverLetter.ott \
	en-US/Personal_Expenses_Report_ct.ots \
	en-US/Personal_Money_Management_Template.ots \
	en-US/personalReferences.ott \
	en-US/personalResume.ott \
	en-US/Recipe.ott \
	en-US/resume1.1.ott \
	en-US/resume_4.ott \
	en-US/Resume_-_Fancy.ott \
	en-US/Resume.ott \
	en-US/shopnotes.ott \
	en-US/Shopping_List.ott \
	en-US/shopping_list_with_dropdown.ott \
	en-US/Sudoku_blank.ott \
	en-US/Todo.ots \
	en-US/visit.ott \
	en-US/wedding_planner.ots \
	en-US/wedgreen-wineWL-OL475.ott \
	en-US/Week_at_a_glance.ott \
	en-US/weekly-plan.otg \
	en-US/week_plan.otg \
	en-US/WeightTracker.ots \
	en-US/wine-label8.ott \
	es/75438-ShopingCart.ott \
	fi/dummy_templates.txt \
	fr/Affiche_vente_fr.ott \
	fr/CD.otg \
	fr/CVfr.ott \
	fr/FairePart-Vierge.otg \
	fr/menu-noel.otp \
	fr/trifold_brochurefr.ott \
	hu/gyermekmero.ots \
	hu/merels_board_game_original.otg \
	hu/napiterv.ott \
	hu/Oneletrajz-magyar.ott \
	hu/resume.ott \
	hu/Sudoku_megoldo.ots \
	it/Corsa_OOo.ots \
	it/curriculum.ott \
	it/faxPersonale.ott \
	ja/dummy_templates.txt \
	ka/dummy_templates.txt \
	nl/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
