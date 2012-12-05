# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallfinance,$(SRCDIR)/extras/source/gallery/finance))

$(eval $(call gb_Package_set_outdir,extras_gallfinance,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallfinance,share/gallery/finance,\
	Agreement.png \
	ATM01.png \
	ATM02.png \
	Balance-Balanced1.png \
	Balance-Balanced2.png \
	Balance-Unbalanced1.png \
	Balance-Unbalanced2.png \
	Calculator.png \
	Chart-Decrease1.png \
	Chart-Decrease2.png \
	Chart-Increase1.png \
	Chart-Increase2.png \
	Chart-Plateau.png \
	Check.png \
	Contract.png \
	CreditCard-Black.png \
	CreditCard-Cut.png \
	CreditCard-Gold.png \
	Currency-Dollar.png \
	Currency-Dollars.png \
	Currency-StackCoins.png \
	Currency-Stack.png \
	GoldBar.png \
	GoldBar-Stack.png \
	MoneyBag01.png \
	MoneyBag02.png \
	PercentSign.png \
	PiggyBank01-Broken.png \
	PiggyBank02-Broken.png \
	PiggyBank-Coins.png \
	PiggyBank-Currency.png \
	Portfolio.png \
	PriceTag.png \
	Safe-Closed.png \
	Safe-Empty.png \
	Safe-Full.png \
	Seal-Gold.png \
	Seal-Red.png \
	Seal-Silver.png \
	Wallet-Empty.png \
	Wallet-Full1.png \
	Wallet-Full2.png \
))

# vim: set noet sw=4 ts=4:
