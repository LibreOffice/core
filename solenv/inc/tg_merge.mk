#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_merge.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2006-12-12 15:50:31 $
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

MKFILENAME:=tg_merge.mk

# look for the according rules in "rules.mk"

.IF "$(WITH_LANG)"!=""
.IF "$(ULFFILES)"!=""
$(foreach,i,$(ULFFILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrb merge
.IF "$(XMLPROPERTIES)"!=""
$(foreach,i,$(XMLPROPERTIES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrm merge
.IF "$(READMEFILES)"!=""
$(foreach,i,$(READMEFILES) $(COMMONMISC)$/$(TARGET)$/$(i:b).xrm) : $$(@:f) localize.sdf 
.ENDIF          # "$(ULFFILES)"!=""

# *.xrm merge
.IF "$(APOCHELPFILES)"!=""
$(foreach,i,$(APOCHELPFILES) $(COMMONMISC)$/$(MYPATH)$/$(i:b).xrm) : $$(@:f) localize.sdf 
.ENDIF          # "$(ULFFILES)"!=""

# *.xcu merge
.IF "$(LOCALIZEDFILES)"!=""
$(foreach,i,$(LOCALIZEDFILES) $(PROCESSOUT)$/merge$/{$(subst,.,$/ $(PACKAGE))}$/$(i:b).xcu) : $$(@:f) localize.sdf 
.ENDIF          # "$(ULFFILES)"!=""
.ENDIF			# "$(WITH_LANG)"!=""

# dependencies from *.ulf to par-files
.IF "$(ULFPARFILES)"!=""
$(ULFPARFILES) : $(COMMONMISC)$/$(TARGET)$/$$(@:b).$(LANGFILEEXT)
.ENDIF          # "$(PARFILES)"!=""

# *.xrm merge
.IF "$(READMEFILES)"!=""
$(uniq $(foreach,i,$(READMEFILES) $(COMMONMISC)$/$(TARGET)$/$(i:b).xrm)) : $$(@:f) localize.sdf
.ENDIF          # "$(ULFFILES)"!=""

