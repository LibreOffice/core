#*************************************************************************
#
#   $RCSfile: lang.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: nf $ $Date: 2001-08-07 11:11:39 $
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


.IF "$(GUI)"=="UNX" || "$(GUIBASE)"=="WIN"
LANG_GUI=WIN
.ELSE
LANG_GUI=$(GUI)
.ENDIF

###################################################
#
# entries for charsets

# variable name equals $(solarlang)$(LANG_GUI)
#
###################################################

.IF "$(USE_NEW_RSC)" != ""

pol$(LANG_GUI)=-CHARSET_microsoft-cp1250
slov$(LANG_GUI)=-CHARSET_microsoft-cp1250
russ$(LANG_GUI)=-CHARSET_microsoft-cp1251
cz$(LANG_GUI)=-CHARSET_microsoft-cp1250
hung$(LANG_GUI)=-CHARSET_microsoft-cp1250
japn$(LANG_GUI)=-CHARSET_UTF8
chinsim$(LANG_GUI)=-CHARSET_UTF8
chintrad$(LANG_GUI)=-CHARSET_UTF8
arab$(LANG_GUI)=-CHARSET_microsoft-cp1256
greek$(LANG_GUI)=-CHARSET_microsoft-cp1253
turk$(LANG_GUI)=-CHARSET_microsoft-cp1254
korean$(LANG_GUI)=-CHARSET_UTF8
extern$(LANG_GUI)=-CHARSET_UTF8

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
# set defaults
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
UTF8=-CHARSET_UTF8
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
.IF "$(solarlang)" == "extern"
RSCLANG=EXTERN ($UTF8)
LANGEXT=99
.ENDIF

.ENDIF			# "$(NO_REC_RES)"==""

