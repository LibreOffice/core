#*******************************************************************
#*/*****************************************************************/
#*
#*    $Workfile:   lang.mk  $
#*
#*    Beschreibung      Compiler und Systemabhaengige
#*                      Entwicklungtools-Definitionen
#*
#*    Ersterstellung    HJS 25.11.98
#*    Letzte Aenderung  $Author: nf $ $Date: 2001-04-05 08:49:37 $
#*    $Revision: 1.3 $
#*
#*    $Logfile:   T:/solar/inc/lang.mkv  $
#*
#*    Copyright (c) 1990-1995, STAR DIVISION
#*
#*******************************************************************

.IF "$(GUI)"=="UNX" || "$(GUIBASE)"=="WIN"
LANG_GUI=WIN
.ELSE
LANG_GUI=$(GUI)
.ENDIF

###################################################
#
# eintragungen fuer charsets
# variablenname entspricht $(solarlang)$(LANG_GUI)
#
###################################################

.IF "$(USE_NEW_RSC)" != ""

pol$(LANG_GUI)=-CHARSET_microsoft-cp1250
slov$(LANG_GUI)=-CHARSET_microsoft-cp1250
russ$(LANG_GUI)=-CHARSET_microsoft-cp1251
cz$(LANG_GUI)=-CHARSET_microsoft-cp1250
hung$(LANG_GUI)=-CHARSET_microsoft-cp1250
japn$(LANG_GUI)=-CHARSET_microsoft-cp932
chinsim$(LANG_GUI)=-CHARSET_microsoft-cp936
chintrad$(LANG_GUI)=-CHARSET_microsoft-cp950
arab$(LANG_GUI)=-CHARSET_microsoft-cp1256
greek$(LANG_GUI)=-CHARSET_microsoft-cp1253
turk$(LANG_GUI)=-CHARSET_microsoft-cp1254
korean$(LANG_GUI)=-CHARSET_microsoft-cp949

.ELSE			# "$(USE_NEW_RSC)" != ""
# os2
danOS2=-CHARSET_ANSI-SOURCE -CHARSET_IBMPC_865
norgOS2=-CHARSET_ANSI-SOURCE -CHARSET_IBMPC_865

polOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
slovOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
russOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
czOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
hungOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
japnOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
chinsimOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
chintradOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
arabOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW

# win, wnt, unx

polWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
slovWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
russWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
czWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
hungWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
japnOS2=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
chinsimWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
chintradWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
arabWIN=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW
.ENDIF			# "$(USE_NEW_RSC)" != ""


###################################################
#
# defaults setzen
#
###################################################

.IF "$(solarlang)" == ""
solarlang=deut
.ENDIF

LANGEXT=49
RSCLANG=GERMAN

.IF "$(NEWCHARSET)"!=""

.IF "$(USE_NEW_RSC)" != ""

RSC_SRS_CHARSET=-CHARSET_DONTKNOW
default$(LANG_GUI)*=-CHARSET_microsoft-cp1252

.ELSE			# "$(USE_NEW_RSC)" != ""
RSC_SRS_CHARSET=-CHARSET_DONTKNOW-SOURCE -CHARSET_DONTKNOW

.IF "$(LANG_GUI)"=="OS2"
$(solarlang)$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_IBMPC_850
default$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_IBMPC_850
.ENDIF
.IF "$(LANG_GUI)"=="MAC"
$(solarlang)$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_MAC
default$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_MAC
.ENDIF
.IF "$(LANG_GUI)"=="WIN"
$(solarlang)$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_ANSI
default$(LANG_GUI)*=-CHARSET_ANSI-SOURCE -CHARSET_ANSI
.ENDIF
RSC_RES_CHARSET=$($(solarlang)$(LANG_GUI))

.ENDIF			# "$(USE_NEW_RSC)" != ""

.ELSE			# "$(NEWCHARSET)"!=""
IBM_PC_437=-CHARSET_IBMPC_437-SOURCE
IBM_PC_850=-CHARSET_IBMPC_850-SOURCE
IBM_PC_852=-CHARSET_IBMPC_852-SOURCE
.ENDIF			# "$(NEWCHARSET)"!=""

####################################################
.IF "$(NO_REC_RES)"==""
.IF "$(solarlang)" == "deut"
RSCLANG=GERMAN $(IBM_PC_437)
IDLLANG=german
LANGEXT=49
.ENDIF

.IF "$(solarlang)" == "enuk"
RSCLANG=ENGLISH_UK $(IBM_PC_437)
IDLLANG=english_uk
LANGEXT=44
.ENDIF
.IF "$(solarlang)" == "endp"
RSCLANG=ENGLISH $(IBM_PC_437)
IDLLANG=english_us
LANGEXT=01
.ENDIF
.IF "$(solarlang)" == "enus"
RSCLANG=ENGLISH_US $(IBM_PC_437)
IDLLANG=english_us
LANGEXT=01
.ENDIF
.IF "$(solarlang)" == "fren"
RSCLANG=FRENCH $(IBM_PC_850)
LANGEXT=33
.ENDIF
.IF "$(solarlang)" == "ital"
RSCLANG=ITALIAN $(IBM_PC_850)
LANGEXT=39
.ENDIF
.IF "$(solarlang)" == "span"
RSCLANG=SPANISH $(IBM_PC_850)
LANGEXT=34
.ENDIF
.IF "$(solarlang)" == "dtch"
RSCLANG=DUTCH $(IBM_PC_850)
LANGEXT=31
.ENDIF
.IF "$(solarlang)" == "dan"
RSCLANG=DANISH $(IBM_PC_850)
LANGEXT=45
.ENDIF
.IF "$(solarlang)" == "swed"
RSCLANG=SWEDISH $(IBM_PC_850)
LANGEXT=46
.ENDIF
.IF "$(solarlang)" == "ptbr"
RSCLANG=PORTUGUESE_BRAZILIAN $(IBM_PC_850)
LANGEXT=55
.ENDIF
.IF "$(solarlang)" == "port"
RSCLANG=PORTUGUESE $(IBM_PC_850)
LANGEXT=03
.ENDIF
.IF "$(solarlang)" == "norg"
RSCLANG=NORWEGIAN $(IBM_PC_850)
LANGEXT=47
.ENDIF
.IF "$(solarlang)" == "suom"
RSCLANG=FINNISH $(IBM_PC_850)
LANGEXT=05
.ENDIF
.IF "$(solarlang)" == "hung"
RSCLANG=HUNGARIAN $(IBM_PC_852)
LANGEXT=36
.ENDIF
.IF "$(solarlang)" == "pol"
RSCLANG=POLISH $(IBM_PC_852)
LANGEXT=48
.ENDIF
.IF "$(solarlang)" == "cz"
RSCLANG=CZECH $(IBM_PC_852)
LANGEXT=42
.ENDIF
.IF "$(solarlang)" == "russ"
RSCLANG=RUSSIAN $(IBM_PC_852)
LANGEXT=07
.ENDIF
.IF "$(solarlang)" == "slov"
RSCLANG=SLOVAK $(IBM_PC_852)
LANGEXT=04
.ENDIF
.IF "$(solarlang)" == "turk"
RSCLANG=TURKISH $(IBM_PC_857)
LANGEXT=90
.ENDIF

#	sprachen nur fuer neues rsc verhalten
.IF "$(solarlang)" == "chinsim"
RSCLANG=CHINESE_SIMPLIFIED
LANGEXT=86
.ENDIF
.IF "$(solarlang)" == "chintrad"
RSCLANG=CHINESE_TRADITIONAL
LANGEXT=88
.ENDIF
.IF "$(solarlang)" == "japn"
RSCLANG=JAPANESE
LANGEXT=81
.ENDIF
.IF "$(solarlang)" == "arab"
RSCLANG=ARABIC
LANGEXT=96
.ENDIF
.IF "$(solarlang)" == "korean"
RSCLANG=KOREAN
LANGEXT=82
.ENDIF
.IF "$(solarlang)" == "cat"
RSCLANG=CATALAN ($IBM_PC_850)
LANGEXT=37
.ENDIF

.ENDIF			# "$(NO_REC_RES)"==""

