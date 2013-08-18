# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,finance,extras/source/gallery/finance))

$(eval $(call gb_Gallery_add_files,finance,$(LIBO_SHARE_FOLDER)/gallery/finance,\
	extras/source/gallery/finance/Agreement.png \
	extras/source/gallery/finance/ATM01.png \
	extras/source/gallery/finance/ATM02.png \
	extras/source/gallery/finance/Balance-Balanced1.png \
	extras/source/gallery/finance/Balance-Balanced2.png \
	extras/source/gallery/finance/Balance-Unbalanced1.png \
	extras/source/gallery/finance/Balance-Unbalanced2.png \
	extras/source/gallery/finance/Calculator.png \
	extras/source/gallery/finance/Chart-Decrease1.png \
	extras/source/gallery/finance/Chart-Decrease2.png \
	extras/source/gallery/finance/Chart-Increase1.png \
	extras/source/gallery/finance/Chart-Increase2.png \
	extras/source/gallery/finance/Chart-Plateau.png \
	extras/source/gallery/finance/Check.png \
	extras/source/gallery/finance/Contract.png \
	extras/source/gallery/finance/CreditCard-Black.png \
	extras/source/gallery/finance/CreditCard-Cut.png \
	extras/source/gallery/finance/CreditCard-Gold.png \
	extras/source/gallery/finance/Currency-Dollar.png \
	extras/source/gallery/finance/Currency-Dollars.png \
	extras/source/gallery/finance/Currency-StackCoins.png \
	extras/source/gallery/finance/Currency-Stack.png \
	extras/source/gallery/finance/GoldBar.png \
	extras/source/gallery/finance/GoldBar-Stack.png \
	extras/source/gallery/finance/MoneyBag01.png \
	extras/source/gallery/finance/MoneyBag02.png \
	extras/source/gallery/finance/PercentSign.png \
	extras/source/gallery/finance/PiggyBank01-Broken.png \
	extras/source/gallery/finance/PiggyBank02-Broken.png \
	extras/source/gallery/finance/PiggyBank-Coins.png \
	extras/source/gallery/finance/PiggyBank-Currency.png \
	extras/source/gallery/finance/Portfolio.png \
	extras/source/gallery/finance/PriceTag.png \
	extras/source/gallery/finance/Safe-Closed.png \
	extras/source/gallery/finance/Safe-Empty.png \
	extras/source/gallery/finance/Safe-Full.png \
	extras/source/gallery/finance/Seal-Gold.png \
	extras/source/gallery/finance/Seal-Red.png \
	extras/source/gallery/finance/Seal-Silver.png \
	extras/source/gallery/finance/Wallet-Empty.png \
	extras/source/gallery/finance/Wallet-Full1.png \
	extras/source/gallery/finance/Wallet-Full2.png \
))

# vim: set noet sw=4 ts=4:
