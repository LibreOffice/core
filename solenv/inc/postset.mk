#*************************************************************************
#
#   $RCSfile: postset.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: nf $ $Date: 2001-08-07 11:12:12 $
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


defaultlang=deut
defaultlangext=49

alllangext=49
deut$(LANG_GUI)*=$(default$(LANG_GUI))
lang_49=deut
longlang_49=german
langext_49=49
rsclang_49=-lgGERMAN $(IBM_PC_437)
rescharset_49=$(deut$(LANG_GUI))

.IF "$(RES_ENUS)"!=""
alllangext+=01
enus$(LANG_GUI)*=$(default$(LANG_GUI))
lang_01=enus
longlang_01=english_us
langext_01=01
rsclang_01=-lgENGLISH_US $(IBM_PC_437)
rescharset_01=$(enus$(LANG_GUI))
RCLANGFLAGS_01+= -d ENGLISH
.ENDIF

.IF "$(UPDATER)"!="" || "$(give_me_all_languages)"!=""

.IF "$(RES_ITAL)"!="" || "$(give_me_all_languages)"!=""
alllangext+=39
ital$(LANG_GUI)*=$(default$(LANG_GUI))
lang_39=ital
longlang_39=italian
langext_39=39
rsclang_39=-lgITALIAN $(IBM_PC_850)
rescharset_39=$(ital$(LANG_GUI))
RCLANGFLAGS_39+= -d ITALY
.ENDIF

.IF "$(RES_SPAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=34
span$(LANG_GUI)*=$(default$(LANG_GUI))
lang_34=span
longlang_34=spanish
langext_34=34
rsclang_34=-lgSPANISH $(IBM_PC_850)
rescharset_34=$(span$(LANG_GUI))
RCLANGFLAGS_34+= -d SPAIN
.ENDIF

.IF "$(RES_FREN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=33
fren$(LANG_GUI)*=$(default$(LANG_GUI))
lang_33=fren
longlang_33=french
langext_33=33
rsclang_33=-lgFRENCH $(IBM_PC_850)
rescharset_33=$(enus$(LANG_GUI))
RCLANGFLAGS_33+= -d FRENCH
.ENDIF

.IF "$(RES_DTCH)"!="" || "$(give_me_all_languages)"!=""
alllangext+=31
dtch$(LANG_GUI)*=$(default$(LANG_GUI))
lang_31=dtch
longlang_31=dutch
langext_31=31
rsclang_31=-lgDUTCH $(IBM_PC_850)
rescharset_31=$(dtch$(LANG_GUI))
RCLANGFLAGS_31+= -d DTCH
.ENDIF

.IF "$(RES_DAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=45
dan$(LANG_GUI)*=$(default$(LANG_GUI))
lang_45=dan
longlang_45=danish
langext_45=45
rsclang_45=-lgDANISH $(IBM_PC_850)
rescharset_45=$(dan$(LANG_GUI))
.ENDIF

.IF "$(RES_SWED)"!="" || "$(give_me_all_languages)"!=""
alllangext+=46
swed$(LANG_GUI)*=$(default$(LANG_GUI))
lang_46=swed
longlang_46=swedish
langext_46=46
rsclang_46=-lgSWEDISH $(IBM_PC_850)
rescharset_46=$(swed$(LANG_GUI))
RCLANGFLAGS_46+= -d SWED
.ENDIF

.IF "$(RES_PORT)"!="" || "$(give_me_all_languages)"!=""
alllangext+=03
port$(LANG_GUI)*=$(default$(LANG_GUI))
lang_03=port
longlang_03=portuguese
langext_03=03
rsclang_03=-lgPORTUGUESE $(IBM_PC_850)
rescharset_03=$(port$(LANG_GUI))
RCLANGFLAGS_03+= -d PORT
.ENDIF 

.IF "$(RES_JAPN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=81
japn$(LANG_GUI)*=$(default$(LANG_GUI))
lang_81=japn
longlang_81=japanese
langext_81=81
rsclang_81=-lgJAPANESE $(UTF8)
rescharset_81=$(japn$(LANG_GUI))
RCLANGFLAGS_81+= -d JAPAN
.ENDIF

.IF "$(RES_CHINSIM)"!="" || "$(give_me_all_languages)"!=""
alllangext+=86
chinsim$(LANG_GUI)*=$(default$(LANG_GUI))
lang_86=chinsim
longlang_86=chinese_simplified
langext_86=86
rsclang_86=-lgCHINESE_SIMPLIFIED $(UTF8)
rescharset_86=$(chinsim$(LANG_GUI))
RCLANGFLAGS_86+= -d CHINA
.ENDIF 

.IF "$(RES_CHINTRAD)"!="" || "$(give_me_all_languages)"!=""
alllangext+=88
chintrad$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_chintrad:=chintrad 88 -lgCHINESE_TRADITIONAL $(chintrad$(LANG_GUI))
lang_88=chintrad
longlang_88=chinese_traditional
langext_88=88
rsclang_88=-lgCHINESE_TRADITIONAL $(UTF8)
rescharset_88=$(chintrad$(LANG_GUI))
.ENDIF 

.IF "$(RES_RUSS)"!="" || "$(give_me_all_languages)"!=""
alllangext+=07
russ$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_russ:=russ 07 -lgRUSSIAN $(IBM_PC_852) $(russ$(LANG_GUI))
lang_07=russ
longlang_07=russian
langext_07=07
rsclang_07=-lgRUSSIAN $(IBM_PC_852)
rescharset_07=$(russ$(LANG_GUI))
RCLANGFLAGS_07+= -d RUSS
.ENDIF

.IF "$(RES_POL)"!="" || "$(give_me_all_languages)"!=""
alllangext+=48
pol$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_pol:=pol 48 -lgPOLISH $(IBM_PC_852) $(pol$(LANG_GUI))
lang_48=pol
longlang_48=polish
langext_48=48
rsclang_48=-lgPOLISH $(IBM_PC_852)
rescharset_48=$(pol$(LANG_GUI))
RCLANGFLAGS_48+= -d POLISH
.ENDIF 

.IF "$(RES_ARAB)"!="" || "$(give_me_all_languages)"!=""
alllangext+=96
arab$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_arab:=arab 96 -lgARABIC $(IBM_PC_852) $(arab$(LANG_GUI))
lang_96=arab
longlang_96=arabic
langext_96=96
rsclang_96=-lgARABIC $(IBM_PC_852)
rescharset_96=$(arab$(LANG_GUI))
RCLANGFLAGS_96+= -d ARABIC
.ENDIF 

.IF "$(RES_GREEK)"!="" || "$(give_me_all_languages)"!=""
alllangext+=30
greek$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_greek:=greek 30 -lgGREEK $(IBM_PC_852) $(greek$(LANG_GUI))
lang_30=greek
longlang_30=greek
langext_30=30
rsclang_30=-lgGREEK $(IBM_PC_852)
rescharset_30=$(greek$(LANG_GUI))
RCLANGFLAGS_30+= -d GREEK
.ENDIF 

.IF "$(RES_TURK)"!="" || "$(give_me_all_languages)"!=""
alllangext+=90
turk$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_turk:=turk 90 -lgTURKISH $(IBM_PC_857) $(turk$(LANG_GUI))
lang_90=turk
longlang_90=turkish
langext_90=90
rsclang_90=-lgTURKISH $(IBM_PC_857)
rescharset_90=$(turk$(LANG_GUI))
RCLANGFLAGS_90+= -d TURKISH
.ENDIF 

.IF "$(RES_KOREAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=82
korean$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_korean:=korean 82 -lgKOREAN $(IBM_PC_852) $(korean$(LANG_GUI))
lang_82=korean
longlang_82=korean
langext_82=82
rsclang_82=-lgKOREAN $(UTF8)
rescharset_82=$(korean$(LANG_GUI))
RCLANGFLAGS_82+= -d KOREAN
.ENDIF 

.IF "$(RES_CAT)"!="" || "$(give_me_all_languages)"!=""
alllangext+=37
catalan$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_catalan:=catalan 37 -lgCATALAN $(IBM_PC_850) $(catalan$(LANG_GUI))
lang_37=catalan
longlang_37=catalan
langext_37=37
rsclang_37=-lgCATALAN $(IBM_PC_850)
rescharset_37=$(catalan$(LANG_GUI))
RCLANGFLAGS_37+= -d CATALAN
.ENDIF 

.ENDIF			#  "$(UPDATER)"!="" || "$(give_me_all_languages)"!=""

.IF "$(L10N-framework)"!=""
alllangext+=99
extern$(LANG_GUI)*=$(default$(LANG_GUI))
cmd_extern:=extern 99 -lgEXTERN $(UTF8) $(extern$(LANG_GUI))
lang_99=extern
longlang_99=extern
langext_99=99
rsclang_99=-lgEXTERN $(UTF8)
rescharset_99=$(extern$(LANG_GUI))
RCLANGFLAGS_99+= -d EXTERN
.ENDIF 

