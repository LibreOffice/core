#*************************************************************************
#
#   $RCSfile: tg_slo.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: mh $ $Date: 2001-06-20 08:58:57 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(IDLSLOFILES:f) $(RSLO)$/$(IDLPACKAGE)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
    +echo $(foreach,i,$(SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="MAC"
    +echo $(foreach,i,$(IDLSLOFILES:f) $(RSLO)$/$(IDLPACKAGE)$/$(i)) | xargs -n1 > $@
    +echo $(foreach,i,$(SLOFILES:f) $(RSLO)$/$(i)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="MAC"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(SLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
.IF "$(COM)"=="ICC"
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ELSE
    $(LIBMGR) r $@ $(SLOFILES)
.ENDIF			# "$(COM)"=="ICC"
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(SLOTARGET)"!=""


.IF "$(SMRSLOTARGET)"!=""
$(SMRSLOTARGET):  $(SMRSLOFILES)
.IF "$(MDB)" != ""
    @+echo $(SMRSLOTARGET:s/ttt/\/)
    @+echo $(SMRSLOFILES)
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n":s/ttt/\/)) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$(@:s/ttt/\/) @$(mktmp $(&:+"\n":s/ttt/\/))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(SMRSLOFILES:f:s/ttt/\/) $(RPACKAGESLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LIBMGR) $(@:s/ttt/\/) $(LIBFLAGS) +$(SMRSLOFILES:+"\n+":^"&":s/ttt/\/)
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $(@:s/ttt/\/)
    $(LIBMGR) $(LIBFLAGS) $(@:s/ttt/\/) @$(mktmp $(&:+"&\n":s/ttt/\/);)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(SMRSLOTARGET)"!=""


.IF "$(S2USLOTARGET)"!=""
$(S2USLOTARGET): $(S2USLOFILES:s/ttt/\/)
.IF "$(MDB)" != ""
    @echo $(S2USLOTARGET)
    @echo $(&:+"\n":s/ttt/\/)
.ENDIF
    @echo ------------------------------
    @echo Making: $(@:s/ttt/\/)
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n":s/ttt/\/)) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$(@:s/ttt/\/) @$(mktmp $(&:+"\n":s/ttt/\/))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(S2USLOFILES:f:s/ttt/\/) $(RPACKAGESLO)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    $(LIBMGR) $(@:s/ttt/\/) $(LIBFLAGS) +$(S2USLOFILES:+"\n+":^"&":s/ttt/\/)
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $(@:s/ttt/\/)
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n":s/ttt/\/);)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(S2USLOTARGET)"!=""


.IF "$(SVXLIGHTSLOTARGET)"!=""
$(SVXLIGHTSLOTARGET): $(REAL_SVXLIGHTSLOFILES)
.IF "$(MDB)" != ""
    @echo $(REAL_SVXLIGHTSLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_SVXLIGHTSLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_SVXLIGHTSLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(SVXLIGHTSLOTARGET)"!=""

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
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(MISC)$/$(TARGET).lin
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    +echo $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WIN"
.IF "$(COM)"=="BLC"
    @+-$(RM) $@ >& $(NULLDEV)
    $(LIBMGR) $@ $(LIBFLAGS) +$(REAL_$(SECOND_BUILD)_SLOFILES:+"\n+":^"&")
.ELSE			# "$(COM)"=="BLC"
.ENDIF			# "$(COM)"=="BLC"
.ENDIF			# "$(GUI)"=="WIN"
.IF "$(GUI)"=="OS2"
    @+-$(RM) $@
    $(LIBMGR) $(LIBFLAGS) $@ @$(mktmp $(&:+"&\n");)
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$($(SECOND_BUILD)SLOTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

