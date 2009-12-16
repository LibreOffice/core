#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_slo.mk,v $
#
# $Revision: 1.15 $
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


MKFILENAME:=TG_SLO.MK

.IF "$(SLOTARGET)"!=""
$(SLOTARGET): $(SLOFILES) $(IDLSLOFILES)
.IF "$(MDB)" != ""
    @echo $(SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo ------------------------------
    @echo Making: $@
#	@$(RM) $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(foreach,i,$(SLOFILES:f) $(RSLO)/$(i)) | xargs -n1 > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(SLOFILES:f) $(RSLO)/$(i:s/.obj/.o/)) | xargs -n1 > $@
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
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
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)/$(i)) | xargs -n1 > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))

.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)SLOTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""
