# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesnonfreefinance,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates-nonfree/finance/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesnonfreefinance,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesnonfreefinance,$(LIBO_SHARE_FOLDER)/template,finance,\
	cs/dummy_templates-nonfree.txt \
	de/dummy_templates-nonfree.txt \
	en-US/24_Month_Sales_Forecast.ots \
	en-US/Balance_Sheet.ots \
	en-US/Break-Even_Analysis.ots \
	en-US/business_plan_calc.ots \
	en-US/Capital_Budget_Analysis.ots \
	en-US/Corporate_Earnings_Analysis.ots \
	en-US/Five_Year_Projections.ots \
	en-US/Income_Statement.ots \
	en-US/job_bid_calc.ots \
	en-US/marketing_plan_calc.ots \
	en-US/Pro-Forma_Balance_Sheet.ots \
	en-US/Pro-Forma_Income_Statement.ots \
	en-US/Quarterly_Marketing_Budget.ots \
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
