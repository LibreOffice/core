# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-gug,dictionaries/gug))

$(eval $(call gb_Dictionary_add_root_files,dict-gug,\
    dictionaries/gug/README_th_gug_PY.txt\
    dictionaries/gug/gug.aff \
    dictionaries/gug/gug.dic \
))


$(eval $(call gb_Dictionary_add_files,dict-gug,description,\
    dictionaries/gug/description/desc_en_US.txt \
    dictionaries/gug/description/desc_es_ES.txt \
    dictionaries/gug/description/desc_gug_PY.txt \
))

$(eval $(call gb_Dictionary_add_files,dict-gug,images,\
    dictionaries/gug/images/gug-PY.png \
))
$(eval $(call gb_Dictionary_add_thesauri,dict-gug,\
    dictionaries/gug/th_gug_PY.dat \
))


# vim: set noet sw=4 ts=4:
