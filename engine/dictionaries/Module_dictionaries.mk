# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,dictionaries))

$(eval $(call gb_Module_add_targets,dictionaries,\
	Configuration_en \
	Configuration_hu \
	Configuration_pt_BR \
	Configuration_ru \
))

$(eval $(call gb_Module_add_l10n_targets,dictionaries,\
	Dictionary_af \
	Dictionary_an \
	Dictionary_ar \
	Dictionary_as \
	Dictionary_be \
	Dictionary_bg \
	Dictionary_bn \
	Dictionary_bo \
	Dictionary_br \
	Dictionary_bs \
	Dictionary_ca \
	Dictionary_ckb \
	Dictionary_cs \
	Dictionary_da \
	$(if $(and $(MPL_SUBSET), $(filter-out LINUX,$(OS))),,Dictionary_de) \
	Dictionary_el \
	$(if $(and $(MPL_SUBSET), $(filter-out LINUX,$(OS))),,Dictionary_en) \
	Dictionary_eo \
	Dictionary_es \
	Dictionary_et \
	Dictionary_fa \
	Dictionary_fr \
	Dictionary_gd \
	Dictionary_gl \
	Dictionary_gu \
	Dictionary_gug \
	Dictionary_he \
	Dictionary_hi \
	Dictionary_hr \
	Dictionary_hu \
	Dictionary_id \
	Dictionary_is \
	$(if $(and $(MPL_SUBSET), $(filter-out LINUX,$(OS))),,Dictionary_it) \
	Dictionary_kmr-Latn \
	Dictionary_kn \
	Dictionary_ko-KR \
	Dictionary_lo \
	Dictionary_lt \
	Dictionary_lv \
	Dictionary_mn \
	Dictionary_mr \
	Dictionary_ne \
	Dictionary_nl \
	Dictionary_no \
	Dictionary_oc \
	Dictionary_or \
	Dictionary_pl \
	Dictionary_pt-BR \
	Dictionary_pt-PT \
	Dictionary_ro \
	Dictionary_ru \
	Dictionary_si \
	Dictionary_sk \
	Dictionary_sl \
	Dictionary_sq \
	Dictionary_sr \
	Dictionary_sv \
	Dictionary_sw \
	Dictionary_ta \
	Dictionary_te \
	Dictionary_th \
	Dictionary_tr \
	Dictionary_uk \
	Dictionary_vi \
	Dictionary_zu \
))

# vim: set noet sw=4 ts=4:
