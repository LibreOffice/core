#*************************************************************************
#
#   $RCSfile: tg_lib.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 18:20:32 $
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

MKFILENAME:=tg_lib.mk

#######################################################
# lines needed for rekursion

.IF "$(MULTI_LIB_FLAG)" == ""
$(LIB1TARGETN) .NULL : LIB1

$(LIB2TARGETN) .NULL : LIB2

$(LIB3TARGETN) .NULL : LIB3

$(LIB4TARGETN) .NULL : LIB4

$(LIB5TARGETN) .NULL : LIB5

$(LIB6TARGETN) .NULL : LIB6

$(LIB7TARGETN) .NULL : LIB7

$(LIB8TARGETN) .NULL : LIB8

$(LIB9TARGETN) .NULL : LIB9
.ENDIF



.IF "$(MULTI_LIB_FLAG)"==""
LIB1 LIB2 LIB3 LIB4 LIB5 LIB6 LIB7 LIB8 LIB9:
    @dmake $(LIB$(TNR)TARGETN) MULTI_LIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE

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
.IF "$(GUI)"=="MAC"
    @+$(RM) $@
    @+echo $(LIB$(TNR)OBJFILES) | sed s\#$(PRJ:s/./\./)$/$(ROUT)\#$(ROUT)\#g | xargs -n 1 > $@
    @+cat /dev/null $(LIB$(TNR)FILES) | xargs -n 1 >> $@
.ELSE                   # "$(GUI)"=="MAC"
.IF "$(GUI)"=="WNT"
    $(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES))
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @-+echo $(LIB$(TNR)OBJFILES) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
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
.ENDIF          # "$(GUI)"=="MAC"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB$(TNR)TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll end
#######################################################

.ENDIF			# "$(MULTI_LIB_FLAG)" == ""
