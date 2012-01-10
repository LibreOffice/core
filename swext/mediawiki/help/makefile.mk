#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

