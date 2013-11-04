# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesfinance,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/finance/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesfinance,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesfinance,$(LIBO_SHARE_FOLDER)/template,finance,\
	cs/podaci_kniha.ots \
	de/arbeitszeiterfassung.ots \
	de/az.ots \
	de/BudgetAufDeutsch.ots \
	de/business_indebtedness_schedule.ots \
	de/business_trip_budget.ots \
	de/discount_zertifikate_rechner.ots \
	de/energie_verbrauchstabelle.ots \
	de/fahrtenbuch.ots \
	de/Finanzuebersicht.ots \
	de/Haushaltsbuch.ots \
	de/Haushaltsbuch_vordruck.ots \
	de/hypothekenberechnung.ots \
	de/Invoice-Rechnung.ots \
	de/Kassenbuch_2006.ots \
	de/Kassenbuch_2007.ots \
	de/kassenbuch2.ots \
	de/kassenbuch.ots \
	de/Kassenbuch_Vordruck.ots \
	de/Kassenbuch_vorlage_Netto.ots \
	de/Kassenbuch_vorlage.ots \
	de/Konto.ots \
	de/konto.ott \
	de/Kraftstoff_Kilometer_Kostenrechner_2006.ots \
	de/Kraftstoff_Kilometer_Kostenrechner_2007.ots \
	de/Kraftstoff_Kilometer_Kostenrechner_2008.ots \
	de/Kraftstoff_Kilometer_Kostenrechner_2009.ots \
	de/Kraftstoff_Kilometer_Kostenrechner_2010.ots \
	de/Lastschrift.ott \
	de/marketing_budget_plan.ots \
	de/Mwstbruttonetto.ots \
	de/OOoReise.ots \
	de/project_planning.ots \
	de/quittung.ott \
	de/Rechnung.ots \
	de/Rechnungs_Vorlage.ots \
	de/start-up_capital_estimate.ots \
	de/taetigkeitsbericht.ots \
	de/Ueberweisungsformular.ott \
	en-US/advloan.ots \
	en-US/Amortization-Schedule.ots \
	en-US/armcalculation.ots \
	en-US/BalanceSheet1.ots \
	en-US/Bank_Account.ots \
	en-US/Budget.ots \
	en-US/business_indebtedness_schedule.ots \
	en-US/business_plan.ott \
	en-US/business_trip_budget.ots \
	en-US/CheckBookRegister.ots \
	en-US/Compare_Loan_Conditions.ots \
	en-US/Compare_Two_Years_with_Graphs.ots \
	en-US/Deposit_Template_-_US_DOLLAR.ots \
	en-US/DialogSheetEg.ots \
	en-US/Giro_en.ott \
	en-US/job_bid.ott \
	en-US/loan.ots \
	en-US/marketing_budget_plan.ots \
	en-US/marketing_plan.ott \
	en-US/mortgage_buster.ots \
	en-US/PettyCash.ots \
	en-US/portfolio_manager.ots \
	en-US/pos.ots \
	en-US/project_planning.ots \
	en-US/rental_returns.ots \
	en-US/RunningBudget.ots \
	en-US/SalesTargets.ots \
	en-US/start-up_capital_estimate.ots \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/dummy_templates.txt \
	hu/afas_szamla.ott \
	hu/Banki_atutalas.ots \
	hu/Csekkminta.ott \
	hu/eves_beszamolo_eredmenykimutatas_egyszerusitett.ots \
	hu/Hazi_koltsegvetes.ots \
	hu/msz16124.ots \
	hu/msz16177.ots \
	hu/Napi_kiadas.ots \
	hu/Olvass.el \
	it/notaspese.ots \
	ja/cost_statement.ots \
	ja/journal_book.ots \
	ka/dummy_templates.txt \
	nl/dummy_templates.txt \
	pl/dummy_templates.txt \
	pt-BR/dummy_templates.txt \
	sv/dummy_templates.txt \
	tr/dummy_templates.txt \
	zh-CN/dummy_templates.txt \
))

# vim: set noet sw=4 ts=4:
