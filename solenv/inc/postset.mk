#*************************************************************************
#
#   $RCSfile: postset.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 18:20:03 $
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



#.IF "$(RES_GER)"!="" || "$(give_me_all_languages)"!=""
alllangext+=49
completelangext+=49
deut$(LANG_GUI)*=$(default$(LANG_GUI))
lang_49=deut
longlang_49=german
langext_49=49
rsclang_49=-lgGERMAN 
rescharset_49=$(deut$(LANG_GUI))
RCLANGFLAGS_49+= -d GERMAN
iso_49=de
defaultlangext=49
#.ENDIF

.IF "$(RES_ENUS)"!="" || "$(give_me_all_languages)"!=""
alllangext+=01
.ENDIF
completelangext+=01
enus$(LANG_GUI)*=$(default$(LANG_GUI))
lang_01=enus
longlang_01=english_us
langext_01=01
rsclang_01=-lgENGLISH_US 
rescharset_01=$(enus$(LANG_GUI))
RCLANGFLAGS_01+= -d ENGLISH
iso_01=en-US
#defaultlangext=01

.IF "$(RES_ITAL)"!="" || "$(give_me_all_languages)"!=""
alllangext+=39
.ENDIF
completelangext+=39
ital$(LANG_GUI)*=$(default$(LANG_GUI))
lang_39=ital
longlang_39=italian
langext_39=39
rsclang_39=-lgITALIAN 
rescharset_39=$(ital$(LANG_GUI))
RCLANGFLAGS_39+= -d ITALY
iso_39=it

.IF "$(RES_SPAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=34
.ENDIF
completelangext+=34
span$(LANG_GUI)*=$(default$(LANG_GUI))
lang_34=span
longlang_34=spanish
langext_34=34
rsclang_34=-lgSPANISH 
rescharset_34=$(span$(LANG_GUI))
RCLANGFLAGS_34+= -d SPAIN
iso_34=es

.IF "$(RES_FREN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=33
.ENDIF
completelangext+=33
fren$(LANG_GUI)*=$(default$(LANG_GUI))
lang_33=fren
longlang_33=french
langext_33=33
rsclang_33=-lgFRENCH 
rescharset_33=$(fren$(LANG_GUI))
RCLANGFLAGS_33+= -d FRENCH
iso_33=fr

.IF "$(RES_DTCH)"!="" || "$(give_me_all_languages)"!=""
alllangext+=31
.ENDIF
completelangext+=31
dtch$(LANG_GUI)*=$(default$(LANG_GUI))
lang_31=dtch
longlang_31=dutch
langext_31=31
rsclang_31=-lgDUTCH 
rescharset_31=$(dtch$(LANG_GUI))
RCLANGFLAGS_31+= -d DTCH
iso_31=nl

.IF "$(RES_DAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=45
.ENDIF
completelangext+=45
dan$(LANG_GUI)*=$(default$(LANG_GUI))
lang_45=dan
longlang_45=danish
langext_45=45
rsclang_45=-lgDANISH 
rescharset_45=$(dan$(LANG_GUI))
iso_45=da

.IF "$(RES_SWED)"!="" || "$(give_me_all_languages)"!=""
alllangext+=46
.ENDIF
completelangext+=46
swed$(LANG_GUI)*=$(default$(LANG_GUI))
lang_46=swed
longlang_46=swedish
langext_46=46
rsclang_46=-lgSWEDISH 
rescharset_46=$(swed$(LANG_GUI))
RCLANGFLAGS_46+= -d SWED
iso_46=sv

.IF "$(RES_PORT)"!="" || "$(give_me_all_languages)"!=""
alllangext+=03
.ENDIF 
completelangext+=03
port$(LANG_GUI)*=$(default$(LANG_GUI))
lang_03=port
longlang_03=portuguese
langext_03=03
rsclang_03=-lgPORTUGUESE 
rescharset_03=$(port$(LANG_GUI))
RCLANGFLAGS_03+= -d PORT
iso_03=pt

.IF "$(RES_JAPN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=81
.ENDIF
completelangext+=81
japn$(LANG_GUI)*=$(default$(LANG_GUI))
lang_81=japn
longlang_81=japanese
langext_81=81
rsclang_81=-lgJAPANESE $(UTF8)
rescharset_81=$(japn$(LANG_GUI))
RCLANGFLAGS_81+= -d JAPAN
iso_81=ja

.IF "$(RES_CHINSIM)"!="" || "$(give_me_all_languages)"!=""
alllangext+=86
.ENDIF 
completelangext+=86
chinsim$(LANG_GUI)*=$(default$(LANG_GUI))
lang_86=chinsim
longlang_86=chinese_simplified
langext_86=86
rsclang_86=-lgCHINESE_SIMPLIFIED $(UTF8)
rescharset_86=$(chinsim$(LANG_GUI))
RCLANGFLAGS_86+= -d CHINA
iso_86=zh-CN

.IF "$(RES_CHINTRAD)"!="" || "$(give_me_all_languages)"!=""
alllangext+=88
.ENDIF 
completelangext+=88
chintrad$(LANG_GUI)*=$(default$(LANG_GUI))
lang_88=chintrad
longlang_88=chinese_traditional
langext_88=88
rsclang_88=-lgCHINESE_TRADITIONAL $(UTF8)
rescharset_88=$(chintrad$(LANG_GUI))
iso_88=zh-TW

.IF "$(RES_RUSS)"!="" || "$(give_me_all_languages)"!=""
alllangext+=07
.ENDIF
completelangext+=07
russ$(LANG_GUI)*=$(default$(LANG_GUI))
lang_07=russ
longlang_07=russian
langext_07=07
rsclang_07=-lgRUSSIAN 
rescharset_07=$(russ$(LANG_GUI))
RCLANGFLAGS_07+= -d RUSS
iso_07=ru

.IF "$(RES_POL)"!="" || "$(give_me_all_languages)"!=""
alllangext+=48
.ENDIF 
completelangext+=48
pol$(LANG_GUI)*=$(default$(LANG_GUI))
lang_48=pol
longlang_48=polish
langext_48=48
rsclang_48=-lgPOLISH 
rescharset_48=$(pol$(LANG_GUI))
RCLANGFLAGS_48+= -d POLISH
iso_48=pl

.IF "$(RES_ARAB)"!="" || "$(give_me_all_languages)"!=""
alllangext+=96
.ENDIF 
completelangext+=96
arab$(LANG_GUI)*=$(default$(LANG_GUI))
lang_96=arab
longlang_96=arabic
langext_96=96
rsclang_96=-lgARABIC 
rescharset_96=$(arab$(LANG_GUI))
RCLANGFLAGS_96+= -d ARABIC
iso_96=ar

.IF "$(RES_GREEK)"!="" || "$(give_me_all_languages)"!=""
alllangext+=30
.ENDIF 
completelangext+=30
greek$(LANG_GUI)*=$(default$(LANG_GUI))
lang_30=greek
longlang_30=greek
langext_30=30
rsclang_30=-lgGREEK 
rescharset_30=$(greek$(LANG_GUI))
RCLANGFLAGS_30+= -d GREEK
iso_30=el

.IF "$(RES_TURK)"!="" || "$(give_me_all_languages)"!=""
alllangext+=90
.ENDIF 
completelangext+=90
turk$(LANG_GUI)*=$(default$(LANG_GUI))
lang_90=turk
longlang_90=turkish
langext_90=90
rsclang_90=-lgTURKISH 
rescharset_90=$(turk$(LANG_GUI))
RCLANGFLAGS_90+= -d TURKISH
iso_90=tr

.IF "$(RES_KOREAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=82
.ENDIF 
completelangext+=82
korean$(LANG_GUI)*=$(default$(LANG_GUI))
lang_82=korean
longlang_82=korean
langext_82=82
rsclang_82=-lgKOREAN $(UTF8)
rescharset_82=$(korean$(LANG_GUI))
RCLANGFLAGS_82+= -d KOREAN
iso_82=ko

.IF "$(RES_CAT)"!="" || "$(give_me_all_languages)"!=""
alllangext+=37
.ENDIF 
completelangext+=37
catalan$(LANG_GUI)*=$(default$(LANG_GUI))
lang_37=catalan
longlang_37=catalan
langext_37=37
rsclang_37=-lgCATALAN 
rescharset_37=$(catalan$(LANG_GUI))
RCLANGFLAGS_37+= -d CATALAN
iso_37=ca

.IF "$(RES_FINN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=35
.ENDIF          # "$(RES_FINN)"!="" || "$(give_me_all_languages)"!=""
completelangext+=35
finn$(LANG_GUI)*=$(default$(LANG_GUI))
lang_35=finn
longlang_35=finnish
langext_35=35
rsclang_35=-lgFINNISH 
rescharset_35=$(finn$(LANG_GUI))
RCLANGFLAGS_35+= -d FINNISH
iso_35=fi

.IF "$(RES_THAI)"!="" || "$(give_me_all_languages)"!=""
alllangext+=66
.ENDIF          # "$(RES_THAI)"!="" || "$(give_me_all_languages)"!=""
completelangext+=66
thai$(LANG_GUI)*=$(default$(LANG_GUI))
lang_66=thai
longlang_66=thai
langext_66=66
rsclang_66=-lgTHAI $(UTF8)
rescharset_66=$(thai$(LANG_GUI))
RCLANGFLAGS_66+= -d THAI
iso_66=th

.IF "$(RES_HINDI)"!="" || "$(give_me_all_languages)"!=""
alllangext+=91
.ENDIF          # "$(RES_HINDI)"!="" || "$(give_me_all_languages)"!=""
completelangext+=91
hindi$(LANG_GUI)*=$(default$(LANG_GUI))
lang_91=hindi
longlang_91=hindi
langext_91=91
rsclang_91=-lgHINDI $(UTF8)
rescharset_91=$(hindi$(LANG_GUI))
RCLANGFLAGS_91+= -d HINDI
iso_91=hi-IN

.IF "$(RES_PORTBR)"!="" || "$(give_me_all_languages)"!=""
alllangext+=55
.ENDIF 
completelangext+=55
portbr$(LANG_GUI)*=$(default$(LANG_GUI))
lang_55=portbr
longlang_55=portuguese_brazilian
langext_55=55
rsclang_55=-lgPORTUGUESE_BRAZILIAN 
rescharset_55=$(portbr$(LANG_GUI))
RCLANGFLAGS_55+= -d PORTBR
iso_55=pt-BR

.IF "$(RES_CZECH)"!="" || "$(give_me_all_languages)"!=""
alllangext+=42
.ENDIF 
completelangext+=42
czech$(LANG_GUI)*=$(default$(LANG_GUI))
lang_42=czech
longlang_42=czech
langext_42=42
rsclang_42=-lgCZECH 
rescharset_42=$(czech$(LANG_GUI))
RCLANGFLAGS_42+= -d CZECH
iso_42=cs

.IF "$(RES_SLOVAK)"!="" || "$(give_me_all_languages)"!=""
alllangext+=43
.ENDIF 
completelangext+=43
slovak$(LANG_GUI)*=$(default$(LANG_GUI))
lang_43=slovak
longlang_43=slovak
langext_43=43
rsclang_43=-lgSLOVAK 
rescharset_43=$(slovak$(LANG_GUI))
RCLANGFLAGS_43+= -d SLOVAK
iso_43=sk

.IF "$(RES_ESTONIAN)"!="" || "$(give_me_all_languages)"!=""
alllangext+=77
.ENDIF 
completelangext+=77
estonian$(LANG_GUI)*=$(default$(LANG_GUI))
lang_77=estonian
longlang_77=estonian
langext_77=77
rsclang_77=-lgESTONIAN 
rescharset_77=$(estonian$(LANG_GUI))
RCLANGFLAGS_77+= -d ESTONIAN
iso_77=et

.IF "$(RES_HEBREW)"!="" || "$(give_me_all_languages)"!=""
alllangext+=97
.ENDIF 
completelangext+=97
hebrew$(LANG_GUI)*=$(default$(LANG_GUI))
lang_97=hebrew
longlang_97=hebrew
langext_97=97
rsclang_97=-lgHEBREW 
rescharset_97=$(hebrew$(LANG_GUI))
RCLANGFLAGS_97+= -d HEBREW
iso_97=he

.IF "$(L10N_framework)"!=""
.IF "$(native_lang_list)"!=""
helplist=$(uniq $(alllangext) $(native_lang_list))
helplist2=$(uniq $(completelangext) $(native_lang_list))
alllangext!:=$(helplist)
completelangext!:=$(helplist2)
.ENDIF          # "$(native_lang_list)"!=""
completelangext+=99
alllangext+=99
extern$(LANG_GUI)*=$(default$(LANG_GUI))
lang_99=extern
longlang_99=extern
langext_99=99
rsclang_99=-lgEXTERN
rescharset_99=$(extern$(LANG_GUI))
RCLANGFLAGS_99+= -d EXTERN
iso_99=$(L10N_framework)
.ENDIF 

