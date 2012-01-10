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



.IF "$(GUI)"=="WNT"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/perl$/lib$/core
PERL_LIB=$(BUILD_TOOLS)$/..$/perl$/lib$/core$/perl58.lib
.ENDIF 

.IF "$(OS)$(CPU)" == "LINUXI"
    .IF "$(OUTPATH)" == "unxubti8" # Hack for Ububtu x86 builds in SO environment 
        SOLARINC+=-I$/usr$/lib$/perl$/5.8.8$/CORE
        PERL_LIB=	-lcrypt \
            $/usr$/lib$/libperl.a \
            $/usr$/lib$/perl$/5.8.8$/auto/DynaLoader/DynaLoader.a
    .ELSE
        SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE
        PERL_LIB=	-lcrypt \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE$/libperl.a \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/auto/DynaLoader/DynaLoader.a
    .ENDIF
.ENDIF

.IF "$(OS)$(CPU)" == "SOLARISS"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE
PERL_LIB=	-lsocket \
            -lnsl \
            -ldl \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE$/shared$/libperl.so \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE$/shared$/DynaLoader.a
.ENDIF

.IF "$(OS)$(CPU)" == "SOLARISI"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE
PERL_LIB=	-lsocket \
            -lnsl \
            -ldl \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE$/shared$/libperl.so \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE$/shared$/DynaLoader.a 
.ENDIF 

.IF "$(OS)$(CPU)" == "MACOSXI"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/CORE
PERL_LIB=	\
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/CORE$/libperl.a \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/auto/DynaLoader/DynaLoader.a
.ENDIF

