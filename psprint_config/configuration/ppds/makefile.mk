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


PRJ=..$/..
PRJNAME=psp_config
TARGET=unxfontsxp3ppds

.INCLUDE : settings.mk

.IF "$(WITHOUT_PPDS)"=="YES"
all:
    @echo "Building without OOo postscript printer definition files."
.ENDIF

ZIP1TARGET      = fontunxppds
.IF "$(WITH_SYSTEM_PPD_DIR)" != ""
ZIP1LIST = SGENPRT.PS SGENT42.PS GENERIC.PS
.ELSE
ZIP1LIST        = * -x makefile.* -x delzip
.ENDIF

.INCLUDE : target.mk

