#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

# creates two files wiki-publisher.oxt and mediawiki_develop.zip, the second one might be used in further build process
ALLTAR: ANTBUILD

.ELSE
@all:
    @echo "MediaWiki Publisher extension disabled."
.ENDIF

