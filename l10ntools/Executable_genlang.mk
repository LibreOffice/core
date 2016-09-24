# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,genlang))

$(eval $(call gb_Executable_set_include,genlang,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_externals,genlang,\
    boost_headers \
))


$(eval $(call gb_Executable_add_scanners,genlang,\
    l10ntools/source/gLexPo   \
    l10ntools/source/gLexUi   \
    l10ntools/source/gLexSrc  \
    l10ntools/source/gLexXcu  \
    l10ntools/source/gLexXcs  \
    l10ntools/source/gLexXrm  \
    l10ntools/source/gLexXml  \
    l10ntools/source/gLexXhp  \
    l10ntools/source/gLexUlf  \
    l10ntools/source/gLexTree \
))

$(eval $(call gb_Executable_add_exception_objects,genlang,\
    l10ntools/source/gLang      \
    l10ntools/source/gL10nMem   \
    l10ntools/source/gConvProp  \
    l10ntools/source/gConv      \
    l10ntools/source/gConvPo    \
    l10ntools/source/gConvSrc   \
    l10ntools/source/gConvUi    \
    l10ntools/source/gConvXrm   \
    l10ntools/source/gConvXml   \
    l10ntools/source/gConvXhp   \
    l10ntools/source/gConvXcs   \
    l10ntools/source/gConvXcu   \
    l10ntools/source/gConvUlf   \
    l10ntools/source/gConvTree  \
))

# vim:set noet sw=4 ts=4:




# localizer for new l10n framework
APP1TARGET=   genLang 
APP1OBJS=     $(OBJFILES)
APP1RPATH=    NONE
APP1LINKTYPE= STATIC

APP1LIBS= 
APP1STDLIBS= 
APP1LIBSALCPPRT=


# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(MISC)$/%_yy.c : %lex.l
	flex -l -w -8 -o$@ $<

# --- Files --------------------------------------------------------


genPO: ALLTAR
	+$(PERL) $(SOLARENV)/bin/deliver.pl

