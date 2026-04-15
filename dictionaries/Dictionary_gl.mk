# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Dictionary_Dictionary,dict-gl,dictionaries/gl))

$(eval $(call gb_Dictionary_add_root_files,dict-gl,\
	dictionaries/gl/Changelog.txt \
	dictionaries/gl/COPYING_th_gl \
	dictionaries/gl/gl_ES.aff \
	dictionaries/gl/gl_ES.dic \
	dictionaries/gl/GPLv3.txt \
	dictionaries/gl/hyph_gl.dic \
	dictionaries/gl/package-description.txt \
	dictionaries/gl/ProxectoTrasno.png \
	dictionaries/gl/README \
	dictionaries/gl/README_hyph-gl.txt \
	dictionaries/gl/README_th_gl.txt \
))

$(eval $(call gb_Dictionary_add_thesauri,dict-gl,\
	dictionaries/gl/thesaurus_gl.dat \
))

# vim: set noet sw=4 ts=4:
