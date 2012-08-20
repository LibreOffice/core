# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangZip_AllLangZip,accessoriestemplateslabels,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/labels/lang))

$(eval $(call gb_AllLangZip_add_dependencies,accessoriestemplateslabels,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_AllLangZip_add_files,accessoriestemplateslabels,\
	cs/dummy_templates.txt \
	de/dummy_templates.txt \
	en-US/address12-WL-OL875.ott \
	en-US/address15-festival-WL-OL875.ott \
	en-US/address21-animals-WL-OL875.ott \
	en-US/address22-animals-WL-OL875.ott \
	en-US/address23-animals-WL-OL875.ott \
	en-US/Address-label10.ott \
	en-US/Address_label7.ott \
	en-US/Alphabetical-1-filling-labels.ott \
	en-US/Alphabetical-2-Filling-labels.ott \
	en-US/Alphabetical-3-Filling-labels.ott \
	en-US/BasicAddress3sameAvery5160.ott \
	en-US/BasicAddressSameAvery5160.ott \
	en-US/BasicMailing3sameAvery5162.ott \
	en-US/BasicMailing4sameAvery5162.ott \
	en-US/BasicMailingSame2Avery5161.ott \
	en-US/BasicMailingSameAvery5161.ott \
	en-US/BasicReturnAddress2SameAvery5167.ott \
	en-US/BasicreturnaddresssameAvery5167.ott \
	en-US/BasicShippingsame2Avery5164.ott \
	en-US/BasicShippingsameAvery5164.ott \
	en-US/Basictext2sameAvery5160.ott \
	en-US/blue-shpWL-OL150.ott \
	en-US/ClasicMailingsameAvery5163.ott \
	en-US/color-bars-2-filling-labels.ott \
	en-US/Fragile-Handle-with-Care-red-background.ott \
	en-US/Fragrance_02_Address_Label.ott \
	en-US/frame-filling-labels.ott \
	en-US/Gemini_label_01.ott \
	en-US/Mailing-Label-01.ott \
	en-US/Mailing_Label-Buss_with-Logo-011.ott \
	en-US/MailingLabelFlowerSameAvery5163.ott \
	en-US/Mailing_Label-General_Theme_01-WL-OL125.ott \
	en-US/miniaddapple-adrsWL-OL875.ott \
	en-US/miniaddflower-adrsWL-OL875.ott \
	en-US/miniaddfruits-adrsWL-OL875.ott \
	en-US/miniaddressbasket-adrsWL-OL875.ott \
	en-US/miniaddrose-adrsWL-OL875.ott \
	en-US/miniaddyellow-adrsWL-OL875.ott \
	en-US/SeasonsShippingLabel.ott \
	en-US/ShippinglabelSameAvery5164.ott \
	en-US/Shipping-std-design3.ott \
	en-US/Shipping-std-design9.ott \
	es/dummy_templates.txt \
	fi/dummy_templates.txt \
	fr/dummy_templates.txt \
	hu/dummy_templates.txt \
	it/dummy_templates.txt \
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
