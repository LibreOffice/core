#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.1 $
#*
#*  last change: $Author: bustamam $ $Date: 2002-03-26 06:32:21 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************/
PRJ=..$/..

PRJNAME=i18npool
TARGET=collator

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
        $(SLO)$/collatorImpl.obj \
        $(SLO)$/chaptercollator.obj \
        $(SLO)$/collator_unicode.obj \
        $(SLO)$/collator_icu.obj \
        $(SLO)$/collator_simple.obj \
        $(SLO)$/collator_cjk.obj \
        $(SLO)$/collator_zh_pinyin.obj \
        $(SLO)$/collator_zh_radical.obj \
        $(SLO)$/collator_zh_stroke.obj \
        $(SLO)$/collator_zh_charset.obj \
        $(SLO)$/collator_zh_zhuyin.obj \
        $(SLO)$/collator_zh_TW_radical.obj \
        $(SLO)$/collator_zh_TW_stroke.obj \
        $(SLO)$/collator_zh_TW_charset.obj \
        $(SLO)$/collator_ko_dict.obj \
        $(SLO)$/collator_ko_charset.obj \
        $(SLO)$/collator_ja_charset.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk



