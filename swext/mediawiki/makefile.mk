#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rene $ $Date: 2008-02-01 10:21:44 $
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
ALLTAR: ANTBUILD

.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
COMMONS_CODEC_JAR=jars/commons-codec-1.3.jar 
COMMONS_LANG_JAR=jars/commons-lang-2.3.jar
COMMONS_HTTPCLIENT_JAR=jars/commons-httpclient-3.0.1.jar
COMMONS_LOGGING_JAR=jars/commons-logging-1.1.jar
.ENDIF

.IF "$(SYSTEM_XML_APIS)" != "YES"
XML_APIS_JAR = $(SOLARVER)$/$(INPATH)$/lib/xml-apis.jar
.ENDIF

ANT_FLAGS+=-Dcommons-codec-jar=$(COMMONS_CODEC_JAR) -Dcommons-lang-jar=$(COMMONS_LANG_JAR) -Dcommons-httpclient-jar=$(COMMONS_HTTPCLIENT_JAR) -Dcommons-logging-jar=$(COMMONS_LOGGING_JAR) -Dxml-apis-jar=$(XML_APIS_JAR)

.ELSE
@all:
    @echo "MediaWiki Editor extension disabled."
.ENDIF

