# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-pt-BR,dictionaries/pt_BR))

$(eval $(call gb_Dictionary_add_root_files,dict-pt-BR,\
    dictionaries/pt_BR/hyph_pt_BR.dic \
    dictionaries/pt_BR/pt_BR.aff \
    dictionaries/pt_BR/pt_BR.dic \
    dictionaries/pt_BR/Lightproof.components \
    dictionaries/pt_BR/Lightproof.py \
    dictionaries/pt_BR/Linguistic.xcu \
    dictionaries/pt_BR/README_Lightproof_pt_BR.txt \
    dictionaries/pt_BR/README_en.txt \
    dictionaries/pt_BR/README_hyph_pt_BR.txt \
    dictionaries/pt_BR/README_pt_BR.txt \
    dictionaries/pt_BR/test_compile_rules.py \
    dictionaries/pt_BR/package-description.txt \
))

$(eval $(call gb_Dictionary_add_files,dict-pt-BR,dialog,\
    dictionaries/pt_BR/dialog/pt_BR.xdl \
    dictionaries/pt_BR/dialog/pt_BR_pt_BR.default \
))

ifeq (,$(findstring pt-BR,$(gb_WITH_LANG)))
$(eval $(call gb_Dictionary_add_files,dict-pt-BR,dialog,\
    dictionaries/pt_BR/dialog/pt_BR_pt_BR.properties \
))
endif

$(eval $(call gb_Dictionary_add_files,dict-pt-BR,pythonpath,\
    dictionaries/pt_BR/pythonpath/lightproof_pt_BR.py \
    dictionaries/pt_BR/pythonpath/lightproof_handler_pt_BR.py \
    dictionaries/pt_BR/pythonpath/lightproof_impl_pt_BR.py \
    dictionaries/pt_BR/pythonpath/lightproof_opts_pt_BR.py \
))

$(eval $(call gb_Dictionary_add_files,dict-pt-BR,icons,\
    dictionaries/pt_BR/icons/VERO-logo.png \
))

$(eval $(call gb_Dictionary_add_generated_file,dict-pt-BR,dialog/OptionsDialog.xcs,$(call gb_XcsTarget_get_target,org/openoffice/Lightproof_pt_BR.xcs)))

$(eval $(call gb_Dictionary_add_localized_xcu_file,dict-pt-BR,dialog,\
    dictionaries/pt_BR/dialog/registry/data/org/openoffice/Office/OptionsDialog.xcu \
))

$(eval $(call gb_Dictionary_add_propertyfiles,dict-pt-BR,dialog,\
    dictionaries/pt_BR/dialog/pt_BR_en_US.properties \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-pt-BR,\
        dictionaries/pt_BR/th_pt_BR.dat \
))


# vim: set noet sw=4 ts=4:
