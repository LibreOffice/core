# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,helplinker))

$(eval $(call gb_Library_set_include,helplinker,\
	-I$(SRCDIR)/helpcompiler/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,helplinker,\
    -DL10N_DLLIMPLEMENTATION \
    -DHELPLINKER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,helplinker,\
    sal \
    comphelper \
))

$(eval $(call gb_Library_use_internal_api,helplinker,\
    udkapi \
    offapi \
))


$(eval $(call gb_Library_use_externals,helplinker,\
    expat \
    libxslt \
    libxml2 \
    clucene \
))

$(eval $(call gb_Library_add_exception_objects,helplinker,\
    helpcompiler/source/HelpCompiler \
    helpcompiler/source/LuceneHelper \
    helpcompiler/source/HelpIndexer \
    helpcompiler/source/HelpSearch \
    helpcompiler/source/BasCodeTagger \
))

$(eval $(call gb_Library_add_exception_objects,helplinker,\
    helpcompiler/source/HelpLinker \
))

# vim: set noet sw=4 ts=4:
