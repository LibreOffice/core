#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: mav $ $Date: 2008-02-28 11:36:32 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..
PRJNAME=swext
TARGET=mediawiki
GEN_HID=FALSE

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

DESCRIPTION:=$(MISC)$/$(TARGET)$/description.xml

# .IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
# PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/$(TARGET)$/license$/license_$i)
# .ELSE
PACKLICS:=$(foreach,i,$(alllangiso) $(MISC)$/$(TARGET)$/license$/LICENSE_$i)
# .ENDIF

common_build_zip=

.INCLUDE : target.mk

.IF "$(GUI)" == "WIN" || "$(GUI)" == "WNT"
$(PACKLICS) : $(SOLARBINDIR)$/osl$/license$$(@:b:s/_/./:e:s/./_/)$$(@:e).txt
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ELSE
$(PACKLICS) : $(SOLARBINDIR)$/osl$/LICENSE$$(@:b:s/_/./:e:s/./_/)$$(@:e)
    @@-$(MKDIRHIER) $(@:d)
    $(GNUCOPY) $< $@
.ENDIF

ALLTAR: $(PACKLICS) $(DESCRIPTION)

.INCLUDE .IGNORE : $(MISC)$/$(TARGET)_lang_track.mk
.IF "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
PHONYDESC=.PHONY
.ENDIF			# "$(LAST_WITH_LANG)"!="$(WITH_LANG)"
$(DESCRIPTION) $(PHONYDESC) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(PERL) $(SOLARENV)$/bin$/licinserter.pl description.xml license/LICENSE_xxx $@
    @echo LAST_WITH_LANG=$(WITH_LANG) > $(MISC)$/$(TARGET)_lang_track.mk

