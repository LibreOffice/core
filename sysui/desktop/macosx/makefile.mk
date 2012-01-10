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
TARGET=desktopmacosx

# --- Settings -------------------------------------------------

.INCLUDE :	settings.mk

# --- Files ----------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
        @echo "Nothing to build for OS $(OS)"

.ELSE           # "$(OS)"!="MACOSX"

ZIPFLAGS = -r
ZIP1TARGET = osxicons
ZIP1DIR = ../icons
ZIP1LIST := $(shell $(PERL) -w list_icons.pl < Info.plist)

.IF "$(GUIBASE)"!="aqua"
CREATOR_TYPE=OOo2
EXECUTABLE=droplet
.ELSE
CREATOR_TYPE=OOO2
EXECUTABLE=soffice
.ENDIF

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/desktopshare
.ELSE # "$(WITH_LANG)"!=""
ULFDIR:=..$/share
.ENDIF # "$(WITH_LANG)"!=""

# --- Targets --------------------------------------------------

.INCLUDE : target.mk

.IF "$(ZIP1TARGETN)"!=""
ZIP1TARGETN : Info.plist extract_icons_names.pl

.ENDIF          # "$(ZIP1TARGETN)"!=""

ALLTAR : $(COMMONMISC)$/{PkgInfo Info.plist} $(COMMONBIN)$/InfoPlist_{$(alllangiso)}.zip

$(COMMONMISC)$/PkgInfo :
    echo "APPL$(CREATOR_TYPE)" > $@


$(COMMONMISC)$/Info.plist : $$(@:f)
    sed -e "s|\%EXECUTABLE|${EXECUTABLE}|g" $< > $@

$(COMMONBIN)$/InfoPlist_{$(alllangiso)}.zip : $(COMMONMISC)$/$$(@:b)/InfoPlist.strings
    cd $(<:d) && zip ../$(@:f).$(INPATH) $(<:f)
    $(MV) -f $(COMMONMISC)$/$(@:f).$(INPATH) $@

$(COMMONMISC)$/InfoPlist_{$(alllangiso)}$/InfoPlist.strings : Info.plist $(ULFDIR)$/documents.ulf
    $(MKDIRHIER) $(@:d)
    $(PERL) -w gen_strings.pl -l $(@:d:d:b:s/InfoPlist_//) -p $< | iconv -f UTF-8 -t UTF-16 > $@.$(INPATH)
    $(MV) -f $@.$(INPATH) $@
    
.ENDIF		# "$(OS)"!="MACOSX"	

