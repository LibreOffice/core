#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..
PRJNAME=swext
TARGET=$(PRJNAME)_help

.INCLUDE : settings.mk

.IF "$(WITH_LANG)"!=""
# workaround for the problem in help, the help uses en instead of en-US
MEDIAWIKI_LANG=$(uniq en $(alllangiso))
aux_alllangiso:=$(MEDIAWIKI_LANG)
.ELSE          # "$(WITH_LANG)"!=""
aux_alllangiso:=$(defaultlangiso)
MEDIAWIKI_LANG=$(uniq en $(alllangiso))
.ENDIF          # "$(WITH_LANG)"!=""

.IF "$(ENABLE_MEDIAWIKI)" != "YES"
all:
    @echo Building mediawiki disabled...
.ELSE           # "$(ENABLE_MEDIAWIKI)" != "YES"

PACKAGE=com.sun.wiki-publisher

OUT_MEDIAWIKI:=$(MISC)$/mediawiki

LINKNAME:=help
XHPLINKSRC:=$(OUT_MEDIAWIKI)/help

XHPDEST=$(OUT_MEDIAWIKI)_merge/help

XHPFILES= \
    wiki.xhp\
    wikiaccount.xhp\
    wikiformats.xhp\
    wikisend.xhp\
    wikisettings.xhp

LINKLINKFILES= \
    $(PACKAGE)/{$(XHPFILES)}

# define with own language set
HLANGXHPFILES=$(foreach,i,$(XHPFILES) $(foreach,j,$(MEDIAWIKI_LANG) $(XHPDEST)$/$j$/$(PACKAGE)$/$(i:f)))

.INCLUDE : target.mk
.INCLUDE : tg_help.mk
.INCLUDE : extension_helplink.mk

ALLTAR : $(OUT_MEDIAWIKI)/help/component.txt

$(OUT_MEDIAWIKI)/help/component.txt : component.txt
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)$(COPY) component.txt $@

$(OUT_MEDIAWIKI)/help/%.xhp : $(OUT_MEDIAWIKI)_merge/help/%.xhp
    @-$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)cat $< | sed -e 's/@WIKIEXTENSIONPRODUCTNAME@/Wiki Publisher/g' | \
        sed  's/@WIKIEXTENSIONID@/com.sun.wiki-publisher/g' | \
        sed 's/@WIKIEXTENSIONFILENAME@/wiki-publisher/g' > $@

.ENDIF          # "$(ENABLE_MEDIAWIKI)" != "YES"

