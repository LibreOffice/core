#*************************************************************************
#
#   $RCSfile: tg_zip.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: mh $ $Date: 2001-06-20 09:09:38 $
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


.IF "$(MULTI_ZIP_FLAG)" == ""
$(ZIP1TARGETN) .NULL : ZIP1

$(ZIP2TARGETN) .NULL : ZIP2

$(ZIP3TARGETN) .NULL : ZIP3

$(ZIP4TARGETN) .NULL : ZIP4

$(ZIP5TARGETN) .NULL : ZIP5

$(ZIP6TARGETN) .NULL : ZIP6

$(ZIP7TARGETN) .NULL : ZIP7

$(ZIP8TARGETN) .NULL : ZIP8

$(ZIP9TARGETN) .NULL : ZIP9
.ENDIF

.IF "$(MULTI_ZIP_FLAG)"==""
ZIP1 ZIP2 ZIP3 ZIP4 ZIP5 ZIP6 ZIP7 ZIP8 ZIP9:
    +@dmake $(ZIP$(TNR)TARGETN) MULTI_ZIP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(GUI)"=="WNT"
command_seperator=&&
#command_seperator=^
avoid_cvs_dir=-x "*CVS*"
.ELSE
command_seperator=;
avoid_cvs_dir=-x "*CVS*"
.ENDIF

.IF "$(ZIP$(TNR)TARGETN)"!=""

$(MISC)$/$(TARGET).$(PWD:f).$(ZIP$(TNR)TARGET).dpzz : $(ZIP$(TNR)TARGETN)

.IF "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP$(TNR)TARGET)}
.ELSE			# "$(common_build_zip)"!=""
ZIP$(TNR)HELPVAR=$(BIN)$/$(ZIP$(TNR)TARGET)
.ENDIF			# "$(common_build_zip)"!=""

.IF "$(make_zip_deps)" == ""
$(ZIP$(TNR)TARGETN) : delzip
.ELSE			# "$(make_zip_deps)" == ""
$(ZIP$(TNR)TARGETN) :
.ENDIF			# "$(make_zip_deps)" == ""
.IF "$(make_zip_deps)" == ""
    @+echo ------------------------------
    @+echo Making: $@
.IF "$(common_build_zip)"!=""
.IF "$(make_zip_deps)" == ""
.IF "$?"!="$(subst,delzip,zzz $?)"
    @+-$(RM) $@  $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
    @+echo rebuilding zipfiles
    @+echo ------------------------------
.ENDIF		# "$?"!="$(subst,delzip,zzz $?)"
.ENDIF			# "$(make_zip_deps)" == ""
    @+-$(GNUCOPY) -p $@ $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) >& $(NULLDEV)
.IF "$(ZIPDIR)" != ""
#	+-echo $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR))
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} $(ZIP$(TNR)LIST:s/LANGDIR/./) -x delzip $(avoid_cvs_dir) 
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
    @+-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(GUI)"=="UNX"
    @+if ( -r $@.new ) $(RENAME) $@.new $@
#	@+if ( -r $@ ) $(TOUCH) $@
.ELSE			# "$(GUI)"=="UNX"
    @+if exist $@.new $(RENAME) $@.new $@
#	@+if exist $@ $(TOUCH) $@
.ENDIF			# "$(GUI)"=="UNX"
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zip $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) -x delzip  $(avoid_cvs_dir)
.ELSE			# "$(ZIPDIR)" != ""
    +-zip $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET), $(@:db))}) $j )) -x delzip $(avoid_cvs_dir)
.ENDIF			# "$(ZIPDIR)" != ""
#	+$(TOUCH) $@
.ENDIF			# "$(common_build_zip)"!=""
.ELSE			# "$(make_zip_deps)" != ""
    @+echo $@ : makefile.mk >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.IF "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    @+-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    @+-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIPDIR)" != ""
    +-cd $(subst,LANGDIR,$(longlang_{$(subst,$(ZIP$(TNR)HELPVAR), $(@:db))}) $(ZIPDIR)) $(command_seperator) zipdep $(ZIPFLAGS) ..$/{$@} $(ZIP$(TNR)LIST:s/LANGDIR/./) >> ..$/$(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ELSE			# "$(ZIPDIR)" != ""
    +-zipdep $(ZIPFLAGS) $@ $(foreach,j,$(ZIP$(TNR)LIST) $(subst,LANGDIR,$(longlang_{$(subst,$(BIN)$/$(ZIP$(TNR)TARGET), $(@:db))}) $j )) >> $(MISC)$/$(TARGET).$(PWD:f).$(@:b).dpzz
.ENDIF			# "$(ZIPDIR)" != ""
.ENDIF			# "$(common_build_zip)"!=""
.ENDIF			# "$(make_zip_deps)" != ""
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF		# "$(MULTI_ZIP_FLAG)"==""
