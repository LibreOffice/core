#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 09:44:55 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

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
    com.sun.star.i18n.XCalendar             \
    com.sun.star.i18n.XCharacterClassification \
    com.sun.star.i18n.XCollator             \
    com.sun.star.i18n.XExtendedTransliteration      \
    com.sun.star.i18n.XLocaleData           \
    com.sun.star.i18n.XNativeNumberSupplier	\
    com.sun.star.i18n.XNumberFormatCode     \
    com.sun.star.i18n.XTransliteration      \
    com.sun.star.i18n.reservedWords         \
    com.sun.star.util.SearchFlags           \
    com.sun.star.util.XTextSearch


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

