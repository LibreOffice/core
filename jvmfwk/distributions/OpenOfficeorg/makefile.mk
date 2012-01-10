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



PRJ = ..$/..
PRJNAME = jvmfwk
TARGET = vendors_ooo

.INCLUDE: settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jvmfwk  because Java is disabled"
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
$(BIN)$/javavendors.xml: javavendors_unx.xml javavendors_wnt.xml javavendors_macosx.xml javavendors_linux.xml
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="FREEBSD"
    -$(COPY) javavendors_freebsd.xml $(BIN)$/javavendors.xml
.ELIF "$(OS)"=="MACOSX"
    -$(COPY) javavendors_macosx.xml $(BIN)$/javavendors.xml
.ELIF "$(OS)"=="LINUX"
    -$(COPY) javavendors_linux.xml $(BIN)$/javavendors.xml
.ELSE
    -$(COPY) javavendors_unx.xml $(BIN)$/javavendors.xml
.ENDIF
.ELIF "$(GUI)"=="WNT"
    -$(COPY) javavendors_wnt.xml $(BIN)$/javavendors.xml	
.ELIF "$(GUI)"=="OS2"
    -$(COPY) javavendors_os2.xml $(BIN)$/javavendors.xml	
.ELSE
    @echo Unsupported platform.
.ENDIF

.ENDIF          # "$(SOLAR_JAVA)"!=""




.INCLUDE: target.mk

