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



PRJ=.

PRJNAME=xsltml
TARGET=xsltml

.IF "$(ENABLE_MEDIAWIKI)" == "YES"

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=xsltml_2.1.2
TARFILE_MD5=a7983f859eafb2677d7ff386a023bc40
PATCH_FILES=$(TARFILE_NAME).patch
TARFILE_IS_FLAT:=TRUE

CONVERTFILES=\
    README \
    entities.xsl \
    glayout.xsl \
    mmltex.xsl \
    cmarkup.xsl \
    scripts.xsl \
    tables.xsl \
    tokens.xsl \

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.ELSE
@all:
    @echo "MediaWiki Publisher extension disabled."
.ENDIF

