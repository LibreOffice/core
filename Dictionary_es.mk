# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-es,dictionaries/es))

$(eval $(call gb_Dictionary_add_root_files,dict-es,\
	dictionaries/es/es_AR.aff \
	dictionaries/es/es_AR.dic \
	dictionaries/es/es_BO.aff \
	dictionaries/es/es_BO.dic \
	dictionaries/es/es_CL.aff \
	dictionaries/es/es_CL.dic \
	dictionaries/es/es_CO.aff \
	dictionaries/es/es_CO.dic \
	dictionaries/es/es_CR.aff \
	dictionaries/es/es_CR.dic \
	dictionaries/es/es_CU.aff \
	dictionaries/es/es_CU.dic \
	dictionaries/es/es_DO.aff \
	dictionaries/es/es_DO.dic \
	dictionaries/es/es_EC.aff \
	dictionaries/es/es_EC.dic \
	dictionaries/es/es_ES.aff \
	dictionaries/es/es_ES.dic \
	dictionaries/es/es_GQ.aff \
	dictionaries/es/es_GQ.dic \
	dictionaries/es/es_GT.aff \
	dictionaries/es/es_GT.dic \
	dictionaries/es/es_HN.aff \
	dictionaries/es/es_HN.dic \
	dictionaries/es/es_MX.aff \
	dictionaries/es/es_MX.dic \
	dictionaries/es/es_NI.aff \
	dictionaries/es/es_NI.dic \
	dictionaries/es/es_PA.aff \
	dictionaries/es/es_PA.dic \
	dictionaries/es/es_PE.aff \
	dictionaries/es/es_PE.dic \
	dictionaries/es/es_PH.aff \
	dictionaries/es/es_PH.dic \
	dictionaries/es/es_PR.aff \
	dictionaries/es/es_PR.dic \
	dictionaries/es/es_PY.aff \
	dictionaries/es/es_PY.dic \
	dictionaries/es/es_SV.aff \
	dictionaries/es/es_SV.dic \
	dictionaries/es/es_US.aff \
	dictionaries/es/es_US.dic \
	dictionaries/es/es_UY.aff \
	dictionaries/es/es_UY.dic \
	dictionaries/es/es_VE.aff \
	dictionaries/es/es_VE.dic \
	dictionaries/es/hyph_es.dic \
	dictionaries/es/package-description.txt \
	dictionaries/es/README_hunspell_es.txt \
	dictionaries/es/README_hyph_es.txt \
	dictionaries/es/README_th_es.txt \
	dictionaries/es/LICENSE.md \
	dictionaries/es/RLA-ES.png \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-es,\
	dictionaries/es/th_es_v2.dat \
))

# vim: set noet sw=4 ts=4:
