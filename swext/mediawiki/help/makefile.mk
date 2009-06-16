#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.3 $
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
PRJNAME=mediawiki
TARGET=$(PRJNAME)_help

.INCLUDE : settings.mk
.INCLUDE : target.mk

.IF "$(ENABLE_MEDIAWIKI)" == "YES"
PACKAGE=com.sun.sun-mediawiki

# workaround for the problem in help, the help uses en instead of en-US
MEDIAWIKI_LANG= en $(alllangiso)

OUT_MEDIAWIKI=$(MISC)$/$(PRJNAME)
OUT_HELP=$(OUT_MEDIAWIKI)$/help

XHPFILES= \
    wiki.xhp\
    wikiaccount.xhp\
    wikiformats.xhp\
    wikisend.xhp\
    wikisettings.xhp

HLANGXHPFILES:=$(foreach,i,$(XHPFILES) $(foreach,j,$(MEDIAWIKI_LANG) $(OUT_HELP)$/$j$/$(PACKAGE)$/$(i:f)))

ALLTAR : $(OUT_MEDIAWIKI)$/$(TARGET).done $(OUT_HELP)$/component.txt

#$(OUT_MEDIAWIKI)$/xhp_changed.flag optix

$(OUT_HELP)$/component.txt : component.txt
    $(COPY) component.txt $(OUT_HELP)$/component.txt

$(HLANGXHPFILES) : #$$(@:d)thisdir.created
    -$(MKDIRHIER) $(@:d)
    $(TOUCH) $(@:d)thisdir.created

$(OUT_HELP)$/{$(MEDIAWIKI_LANG)}$/$(PACKAGE)$/%.xhp :| %.xhp
    @$(TOUCH) $@
# internal dependencies not sufficient to trigger merge?
#    @$(NULL)


$(OUT_MEDIAWIKI)$/$(TARGET).done : localize.sdf $(XHPFILES) $(HLANGXHPFILES)
    @$(HELPEX) -QQ -p $(PRJNAME) -r $(PRJ) -i @$(mktmp $(uniq $(foreach,i,$? $(!eq,$(i:f),$(i:f:s/.xhp//) $(i:f) $(XHPFILES))))) -x $(OUT_HELP) -y $(PACKAGE) -l all -lf $(MEDIAWIKI_LANG:t",") -m localize.sdf && $(TOUCH) $@
.ENDIF

