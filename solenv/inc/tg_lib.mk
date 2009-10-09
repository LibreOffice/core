#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_lib.mk,v $
#
# $Revision: 1.24 $
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

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(LIB$(TNR)TARGET)" != ""
.IF "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)ARCHIV) :	$(LIB$(TNR)TARGET)
    @echo Making: $@
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)" =="HPUX_FRAG_HR"
    @-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @echo $(LINK) +inst_close -c `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
.ENDIF
    @echo $(LIBMGR) $(LIB$(TNR)FLAGS) $(LIBFLAGS) $(LIB$(TNR)ARCHIV) `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
     @echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB$(TNR)FLAGS) $(LIBFLAGS) $(LIB$(TNR)ARCHIV) `cat $(LIB$(TNR)TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)TARGET) :	$(LIB$(TNR)FILES) \
                        $(LIB$(TNR)OBJFILES) \
                        $(LIB$(TNR)DEPN)
    @echo using: $(LIB$(TNR)FILES)
    @echo using: $(LIB$(TNR)TARGET)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB$(TNR)OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB$(TNR)FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(LIBMGR) $(LIBFLAGS) $@ $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB$(TNR)OBJFILES)) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB$(TNR)OBJFILES)"!=""    
.IF "$(LIB$(TNR)FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB$(TNR)FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB$(TNR)OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB$(TNR)FILES) | xargs -n1 >> $@
.ELSE
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB$(TNR)OBJFILES)) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB$(TNR)OBJFILES)"!=""    
.IF "$(LIB$(TNR)FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB$(TNR)FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
    echo $(LIBMGR) r $@ $(LIB$(TNR)OBJFILES)
    $(LIBMGR) r $@ $(LIB$(TNR)OBJFILES) $(LIB$(TNR)FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB$(TNR)TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll end
#######################################################

