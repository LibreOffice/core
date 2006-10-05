#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_slo.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2006-10-05 10:41:50 $
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


MKFILENAME:=TG_SLO.MK

.IF "$(SLOTARGET)"!=""
$(SLOTARGET): $(SLOFILES) $(IDLSLOFILES)
.IF "$(MDB)" != ""
    @echo $(SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
#	@+$(RM) $@
.IF "$(GUI)"=="WNT"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.IF "$(OS)"=="MACOSX"
    @-+nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @+nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(SLOTARGET)"!=""


.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)SLOTARGET)"!=""
$($(SECOND_BUILD)SLOTARGET): $(REAL_$(SECOND_BUILD)_SLOFILES)
.IF "$(MDB)" != ""
    @echo $(REAL_$(SECOND_BUILD)SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)SLOTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

