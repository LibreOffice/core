#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..
TARGET=desktopmacosx

# --- Settings -------------------------------------------------

.INCLUDE :	settings.mk

# --- Files ----------------------------------------------------

.IF "$(OS)"!="MACOSX"

dummy:
# nothing

.ELSE           # "$(OS)"!="MACOSX"

ZIPFLAGS = -r
ZIP1TARGET = osxicons
ZIP1DIR = ../icons
ZIP1LIST := $(shell $(PERL) -w list_icons.pl < Info.plist)

CREATOR_TYPE=LIBO
EXECUTABLE=soffice

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

