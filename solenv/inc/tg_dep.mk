#*************************************************************************
#
#   $RCSfile: tg_dep.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 14:47:01 $
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

.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(SRCFILES)$(SRC1FILES)$(SRC2FILES)$(SRC3FILES)$(RCFILES)$(HXX1TARGET)$(HDBDEPNTARGET)$(IDLFILES)$(PARFILES)$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"!=""
ALLDEP .PHONY: 
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @+if ( -e  "$(SRS)$/$(PWD:f).*.dpr" ) +-$(RM) "$(SRS)$/$(PWD:f).*.dpr" >& $(NULLDEV)
.ELSE
    @+-$(RM) $(SRS)$/$(PWD:f).*.dpr >& $(NULLDEV)
.ENDIF
    @+-$(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dprr >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpj >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpc >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(TARGET).dpz >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP1TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP2TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP3TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP4TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP5TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP6TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP7TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP8TYPELIST).mk >& $(NULLDEV)
    @+-$(RM) $(MISC)$/$(COMP9TYPELIST).mk >& $(NULLDEV)
.IF "$(DEPFILES)" != ""
#to keep win9x happy
.IF "$(GROUPSHELL:b:l)"=="4dos"
    @+-echo $(foreach,i,$(DEPFILES) $(shell +-del $i >& $(NULLDEV))) >& $(NULLDEV)
.ELSE			# "$(GROUPSHELL:b)"=="4dos"
.IF "$(USE_SHELL)"=="4nt"
    @+-echo $(foreach,i,$(DEPFILES) $(shell +-del $i >& $(NULLDEV))) >& $(NULLDEV)
.ELSE			# "$(USE_SHELL)"=="4nt"
    @+-$(RM) $(DEPFILES) >& $(NULLDEV)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(GROUPSHELL:b)"=="4dos"
.ENDIF			# "$(DEPFILES)" != ""
    +@echo ---
    +@echo      Old dependency files removed
    +@echo ---


ALLDPC: \
        $(DEPFILES) \
        $(CFILES) \
        $(CXXFILES) \
        $(RCFILES) \
        $(IDLFILES) \
        $(HDBDEPNTARGET)
.IF "$(nodep)"!=""
    @echo NOT-Making: Depend-Lists
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ELSE
    @echo Making : Dependencies 
.IF "$(CFILES)$(CXXFILES)"!=""
.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)"==""
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    @echo 		da stimmt was nicht!!
    @echo 		source files aber keine obj
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
#	@quit
.ENDIF
.ENDIF
.IF "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)$(PARFILES)"!=""
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
#	@+if ( -e  $(MISC)$/$(TARGET).dpr ) $(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+if ( -e  $(MISC)$/$(TARGET).dpj ) $(RM) $(MISC)$/$(TARGET).dpj >& $(NULLDEV)
    @+if ( -e  $(MISC)$/genjava.mk ) $(RM) $(MISC)$/genjava.mk >& $(NULLDEV)
.ELSE
#	@+-if exist $(MISC)$/$(TARGET).dpr $(RM) $(MISC)$/$(TARGET).dpr >& $(NULLDEV)
    @+-if exist $(MISC)$/$(TARGET).dpj $(RM) $(MISC)$/$(TARGET).dpj >& $(NULLDEV)
    @+-if exist $(MISC)$/genjava.mk $(RM) $(MISC)$/genjava.mk >& $(NULLDEV)
.ENDIF
    +$(TOUCH) $(MISC)$/$(TARGET).dpc
.IF "$(SVXLIGHT)"!=""
#.IF "$(SVXLIGHTSLOFILES)"!=""
#	@+$(TYPE) $(mktmp $(foreach,i,$(SVXLIGHTSLOFILES) $(i:d:^"\n")sxl_$(i:f) : $i )) >> $(MISC)$/$(TARGET).dpc
#.ENDIF
.IF "$(SVXLIGHTOBJFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$(SVXLIGHTOBJFILES) $(i:d:^"\n")sxl_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.ENDIF			# "$(SVXLIGHT)"!=""
.IF "$($(SECOND_BUILD)_SLOFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_SLOFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $i )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""
    @+$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_OBJFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.ELSE			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ENDIF			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
.ENDIF			# "$(nodep)"!=""
.ELSE		# irgendwas abhaengiges


ALLDPC:
    @echo ------------------------------
    @echo No Dependencies
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @echo "#" > $(MISC)$/$(TARGET).dpc
.ELSE			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @echo # > $(MISC)$/$(TARGET).dpc
.ENDIF			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"

ALLDEP:
    @echo ------------------------------
    @echo No Dependencies

.ENDIF

