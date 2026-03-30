# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-hu,dictionaries/hu_HU))

$(eval $(call gb_Dictionary_add_root_files,dict-hu,\
	dictionaries/hu_HU/hu_HU.aff \
	dictionaries/hu_HU/hu_HU.dic \
	dictionaries/hu_HU/hyph_hu_HU.dic \
	dictionaries/hu_HU/Lightproof.components \
	dictionaries/hu_HU/Lightproof.py \
	dictionaries/hu_HU/Linguistic.xcu \
	dictionaries/hu_HU/README_hu_HU.txt \
	dictionaries/hu_HU/README_hyph_hu_HU.txt \
	dictionaries/hu_HU/README_lightproof_hu_HU.txt \
	dictionaries/hu_HU/README_th_hu_HU_v2.txt \
))

$(eval $(call gb_Dictionary_add_files,dict-hu,dialog,\
	dictionaries/hu_HU/dialog/hu_HU.xdl \
	dictionaries/hu_HU/dialog/hu_HU_en_US.default \
	dictionaries/hu_HU/dialog/hu_HU_hu_HU.properties \
))

$(eval $(call gb_Dictionary_add_files,dict-hu,help/en,\
	dictionaries/hu_HU/help/en/help.tree \
))

$(eval $(call gb_Dictionary_add_files,dict-hu,help/hu,\
	dictionaries/hu_HU/help/hu/help.tree \
))

$(eval $(call gb_Dictionary_add_files,dict-hu,help/hu/org.openoffice.hu.hunspell.dictionaries,\
	dictionaries/hu_HU/help/hu/org.openoffice.hu.hunspell.dictionaries/fsfhu.png \
	dictionaries/hu_HU/help/hu/org.openoffice.hu.hunspell.dictionaries/page1.xhp \
	dictionaries/hu_HU/help/hu/org.openoffice.hu.hunspell.dictionaries/szinonima.png \
))

$(eval $(call gb_Dictionary_add_files,dict-hu,pythonpath,\
	dictionaries/hu_HU/pythonpath/lightproof_hu_HU.py \
	dictionaries/hu_HU/pythonpath/lightproof_handler_hu_HU.py \
	dictionaries/hu_HU/pythonpath/lightproof_impl_hu_HU.py \
	dictionaries/hu_HU/pythonpath/lightproof_opts_hu_HU.py \
))

$(eval $(call gb_Dictionary_add_generated_file,dict-hu,dialog/OptionsDialog.xcs,$(call gb_XcsTarget_get_target,org/openoffice/Lightproof_hu_HU.xcs)))

$(eval $(call gb_Dictionary_add_localized_xcu_file,dict-hu,dialog,\
	dictionaries/hu_HU/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu \
))

$(eval $(call gb_Dictionary_add_propertyfiles,dict-hu,dialog,\
	dictionaries/hu_HU/dialog/hu_HU_en_US.properties \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-hu,\
	dictionaries/hu_HU/th_hu_HU_v2.dat \
))

# vim: set noet sw=4 ts=4:
