#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2003-04-08 16:15:06 $
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

PRJ=..$/..
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=i18n

ENABLE_EXCEPTIONS=TRUE

# --- Settings common for the whole project -----

#UNOTYPES=
#.INCLUDE : $(PRJINC)$/unotools$/unotools.mk

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Types -------------------------------------


UNOTYPES+= \
    com.sun.star.i18n.CalendarDisplayIndex  \
    com.sun.star.i18n.CalendarFieldIndex    \
    com.sun.star.i18n.CollatorOptions       \
    com.sun.star.i18n.KCharacterType        \
    com.sun.star.i18n.KNumberFormatType     \
    com.sun.star.i18n.KNumberFormatUsage    \
    com.sun.star.i18n.KParseTokens          \
    com.sun.star.i18n.KParseType            \
    com.sun.star.i18n.LocaleItem            \
    com.sun.star.i18n.reservedWords         \
    com.sun.star.i18n.XCalendar             \
    com.sun.star.i18n.XCharacterClassification \
    com.sun.star.i18n.XCollator             \
    com.sun.star.i18n.XLocaleData           \
    com.sun.star.i18n.XNumberFormatCode     \
    com.sun.star.i18n.XTransliteration      \
    com.sun.star.util.SearchFlags           \
    com.sun.star.util.XTextSearch			\
    drafts.com.sun.star.i18n.XExtendedTransliteration      \
    drafts.com.sun.star.i18n.XNativeNumberSupplier	


# --- Files -------------------------------------

SLOFILES=	\
    $(SLO)$/charclass.obj	\
    $(SLO)$/calendarwrapper.obj	\
    $(SLO)$/collatorwrapper.obj	\
    $(SLO)$/intlwrapper.obj \
    $(SLO)$/localedatawrapper.obj	\
    $(SLO)$/nativenumberwrapper.obj	\
    $(SLO)$/numberformatcodewrapper.obj \
    $(SLO)$/readwritemutexguard.obj \
    $(SLO)$/transliterationwrapper.obj \
    $(SLO)$/textsearch.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

