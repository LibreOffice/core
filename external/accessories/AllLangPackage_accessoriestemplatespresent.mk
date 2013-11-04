# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatespresent,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/presnt/lang,$(true)))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatespresent,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatespresent,$(LIBO_SHARE_FOLDER)/template,presnt,\
	cs/dummy_templates.txt \
	de/compladients.otp \
	de/praktikumsbericht.otp \
	de/produktentwicklung.otp \
	de/Produktplanung-Verkauf.otp \
	de/prs-novelty-old-2.otp \
	de/prs-novelty-old.otp \
	de/prs-strategy-old-2.otp \
	de/prs-strategy-old.otp \
	de/team.otp \
	de/tech.otp \
	de/umweltschutz.otp \
	de/Unternehmensbericht.otp \
	en-US/compladients.otp \
	en-US/Focused_Discussion.otp \
	en-US/Introducing_New_Product.otp \
	en-US/Orangedric.otp \
	en-US/prs-novelty-old-2.otp \
	en-US/prs-novelty-old.otp \
	en-US/prs-strategy-old-2.otp \
	en-US/prs-strategy-old.otp \
	en-US/RedCircle.otp \
	en-US/Stylish.otp \
	en-US/team.otp \
	en-US/tech.otp \
	en-US/template1.otp \
	en-US/template2.otp \
	en-US/template3.otp \
	en-US/template4.otp \
	es/compladients.otp \
	fi/compladients.otp \
	fr/compladients.otp \
	fr/prs-novelty-old-2.otp \
	fr/prs-novelty-old.otp \
	fr/prs-strategy-old-2.otp \
	fr/prs-strategy-old.otp \
	fr/team.otp \
	fr/tech.otp \
	hu/Anytime_presentation.otp \
	hu/Bemutato_uzleti.otp \
	hu/compladients.otp \
	hu/prs-novelty-old.otp \
	hu/prs-strategy-old.otp \
	it/compladients.otp \
	it/prs-novelty-old-2.otp \
	it/prs-novelty-old.otp \
	it/prs-strategy-old-2.otp \
	it/prs-strategy-old.otp \
	it/team.otp \
	it/tech.otp \
	ja/business_plan.otp \
	ja/company-wide_meeting.otp \
	ja/compladients.otp \
	ja/creative_session.otp \
	ja/financial_reporting.otp \
	ja/general_2.otp \
	ja/general.otp \
	ja/marketing_plan.otp \
	ja/new_employee_orientation.otp \
	ja/problem_report_2.otp \
	ja/problem_report.otp \
	ja/progress_repost_2.otp \
	ja/progress_repost.otp \
	ja/proposition_of_strategic_planning_2.otp \
	ja/proposition_of_strategic_planning.otp \
	ja/sales_plan_of_product_and_idea_2.otp \
	ja/sales_plan_of_product_and_idea.otp \
	ja/top_10_list.otp \
	ja/training_2.otp \
	ja/training.otp \
	ka/compladients.otp \
	nl/compladients.otp \
	pl/compladients.otp \
	pt-BR/compladients.otp \
	sv/compladients.otp \
	tr/compladients.otp \
	zh-CN/compladients.otp \
))

# vim: set noet sw=4 ts=4:
