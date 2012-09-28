#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..
PRJNAME=swext
TARGET=mediawiki
.IF "$(L10N_framework)"==""
.IF "$(ENABLE_MEDIAWIKI)" == "YES"
.INCLUDE : ant.mk

.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
COMMONS_CODEC_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-codec-1.6.jar
COMMONS_LANG_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-lang-2.4.jar
COMMONS_HTTPCLIENT_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-httpclient-3.1.jar
COMMONS_LOGGING_JAR=$(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/commons-logging-1.1.1.jar
.ELSE
COMP=fix_system_commons
.ENDIF

.IF defined(debug) || defined(DEBUG)
ANTDEBUG=true
.ELSE
ANTDEBUG=off
.ENDIF

ANT_FLAGS+=-Dcommons-codec-jar=$(COMMONS_CODEC_JAR) -Dcommons-lang-jar=$(COMMONS_LANG_JAR) -Dcommons-httpclient-jar=$(COMMONS_HTTPCLIENT_JAR) -Dcommons-logging-jar=$(COMMONS_LOGGING_JAR) -Dantdebug=$(ANTDEBUG)

# creates two files wiki-publisher.oxt and mediawiki_develop.zip, the second one might be used in further build process
ALLTAR: $(COMP) ANTBUILD

fix_system_commons:
    @echo "Fix Java Class-Path entry for Apache Commons libraries from system."
    @$(SED) -i.bak -r -e "s#(name=\"Class-Path\" value=\").*\"#\1file://$(COMMONS_CODEC_JAR) file://$(COMMONS_LANG_JAR) \
file://$(COMMONS_HTTPCLIENT_JAR) file://$(COMMONS_LOGGING_JAR)\"#" build.xml
    @echo "Unbundle Apache Commons libraries from Mediawiki Presentation extension."
    @$(SED) -i.bak '/file="..commons/d' build.xml

.ELSE
@all:
    @echo "MediaWiki Publisher extension disabled."
.ENDIF

.ELSE
pesudo:
.ENDIF
