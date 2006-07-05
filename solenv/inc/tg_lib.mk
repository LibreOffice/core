#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_lib.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:58:42 $
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

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(LIB$(TNR)TARGET)" != ""
.IF "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)ARCHIV) :	$(LIB$(TNR)TARGET)
    @echo Making: $@
    @-+$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+-$(RM) $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)" =="HPUX_FRAG_HR"
    @+-$(RM) $(MISC)$/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @+echo $(LINK) +inst_close -c `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(MISC)$/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @cat $(MISC)$/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @source $(MISC)$/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
.ENDIF
    @+echo $(LIBMGR) $(LIB$(TNR)FLAGS) $(LIBFLAGS) $(LIB$(TNR)ARCHIV) `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @+echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
     @+echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
    @cat $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
    @source $(MISC)$/$(LIB$(TNR)ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
    @echo just a dummy > $@
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)TARGET) :	$(LIB$(TNR)FILES) \
                        $(LIB$(TNR)OBJFILES) \
                        $(LIB$(TNR)DEPN)
    @echo using: $(LIB$(TNR)FILES)
    @echo using: $(LIB$(TNR)TARGET)
    @echo ------------------------------
    @echo Making: $@
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+echo $(LIB$(TNR)OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)$/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @+cat /dev/null $(LIB$(TNR)FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @+$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-+nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @+nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)$/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES))
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB$(TNR)OBJFILES)) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB$(TNR)OBJFILES)"!=""    
.IF "$(LIB$(TNR)FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB$(TNR)FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
    @+$(ECHONL)
.ELSE			# "$(GUI)"=="WNT"
    @+-$(RM) $@
    +echo $(LIBMGR) r $@ $(LIB$(TNR)OBJFILES)
    $(LIBMGR) r $@ $(LIB$(TNR)OBJFILES) $(LIB$(TNR)FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB$(TNR)TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll end
#######################################################

