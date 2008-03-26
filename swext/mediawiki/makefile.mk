#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: mav $ $Date: 2008-03-26 16:30:07 $
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

PRJ=..
PRJNAME=swext
TARGET=mediawiki

.IF "$(ENABLE_MEDIAWIKI)" == "YES"
.INCLUDE : ant.mk

.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
COMMONS_CODEC_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-codec-1.3.jar 
COMMONS_LANG_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-lang-2.3.jar
COMMONS_HTTPCLIENT_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-httpclient-3.1.jar
COMMONS_LOGGING_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-logging-1.1.1.jar
.ENDIF

ANT_FLAGS+=-Dcommons-codec-jar=$(COMMONS_CODEC_JAR) -Dcommons-lang-jar=$(COMMONS_LANG_JAR) -Dcommons-httpclient-jar=$(COMMONS_HTTPCLIENT_JAR) -Dcommons-logging-jar=$(COMMONS_LOGGING_JAR)

ALLTAR: ANTBUILD

.ELSE
@all:
    @echo "MediaWiki Publisher extension disabled."
.ENDIF

