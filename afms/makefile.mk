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

PRJNAME=afms
TARGET=afms

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.IF "$(L10N_framework)"==""
# --- Files --------------------------------------------------------

TARFILE_NAME=Adobe-Core35_AFMs-314
TARFILE_MD5=1756c4fa6c616ae15973c104cd8cb256
TARFILE_ROOTDIR=Adobe-Core35_AFMs-314

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk

ZIP1DIR         = $(MISC)$/build$/$(TARFILE_NAME)
ZIP1TARGET      = fontunxafm
ZIP1LIST        = *.afm -x "*Helvetica-Narrow*"

.ENDIF # L10N_framework
.INCLUDE : target.mk

.INCLUDE : tg_ext.mk
.IF "$(L10N_framework)"==""
.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN):	$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)

.ENDIF          # "$(ZIP1TARGETN)"!=""

.ENDIF # L10N_framework
