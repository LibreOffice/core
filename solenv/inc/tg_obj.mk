#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_obj.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:56:07 $
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

MKFILENAME:=TG_OBJ.MK

.IF "$(OBJTARGET)"!=""
$(OBJTARGET): $(OBJFILES) $(IDLOBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    $(ECHONL) $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i)) >> $@
    @$(TYPE) $@ | tr -d ï\n\rï > $@
    @$(ECHONL) >> $@
.ELSE			# "$(COM)"=="GCC"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 > $@
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(OBJTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(OBJTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(OBJTARGET)"!=""


.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)OBJTARGET)"!=""
$($(SECOND_BUILD)OBJTARGET): $(REAL_$(SECOND_BUILD)_OBJFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
#     $(LIBMGR) $(LIBFLAGS) $@ $(OBJFILES)
    $(ECHONL) $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)$/$(i)) > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    echo $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)$/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)OBJTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

