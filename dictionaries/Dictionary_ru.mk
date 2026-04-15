# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-ru,dictionaries/ru_RU))

$(eval $(call gb_Dictionary_add_root_files,dict-ru,\
	dictionaries/ru_RU/hyph_ru_RU.dic \
	dictionaries/ru_RU/Lightproof.components \
	dictionaries/ru_RU/Lightproof.py \
	dictionaries/ru_RU/Linguistic.xcu \
	dictionaries/ru_RU/README_Lightproof_ru_RU.txt \
	dictionaries/ru_RU/README_ru_RU.txt \
	dictionaries/ru_RU/README_thes_ru_RU_M_aot_and_v2.txt \
	dictionaries/ru_RU/ru_RU.aff \
	dictionaries/ru_RU/ru_RU.dic \
))

$(eval $(call gb_Dictionary_add_files,dict-ru,dialog,\
	dictionaries/ru_RU/dialog/ru_RU.xdl \
	dictionaries/ru_RU/dialog/ru_RU_en_US.default \
	dictionaries/ru_RU/dialog/ru_RU_ru_RU.properties \
))

$(eval $(call gb_Dictionary_add_files,dict-ru,pythonpath,\
	dictionaries/ru_RU/pythonpath/lightproof_ru_RU.py \
	dictionaries/ru_RU/pythonpath/lightproof_handler_ru_RU.py \
	dictionaries/ru_RU/pythonpath/lightproof_impl_ru_RU.py \
	dictionaries/ru_RU/pythonpath/lightproof_opts_ru_RU.py \
))

$(eval $(call gb_Dictionary_add_propertyfiles,dict-ru,dialog,\
	dictionaries/ru_RU/dialog/ru_RU_en_US.properties \
))

$(eval $(call gb_Dictionary_add_generated_file,dict-ru,dialog/OptionsDialog.xcs,$(call gb_XcsTarget_get_target,org/openoffice/Lightproof_ru_RU.xcs)))

$(eval $(call gb_Dictionary_add_localized_xcu_file,dict-ru,dialog,\
	dictionaries/ru_RU/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-ru,\
	dictionaries/ru_RU/th_ru_RU_M_aot_and_v2.dat \
))

# vim: set noet sw=4 ts=4:
