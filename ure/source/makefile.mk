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



PRJ = ..
PRJNAME = ure
TARGET = source

ZIP1TARGET = uretest
ZIP1FLAGS = -r
ZIP1LIST = uretest

my_components = \
    acceptor \
    binaryurp \
    bootstrap \
    connector \
    introspection \
    invocadapt \
    invocation \
    javaloader \
    javavm \
    juh \
    namingservice \
    proxyfac \
    reflection \
    stocservices \
    streams \
    textinstream \
    textoutstream \
    uuresolver

.INCLUDE: settings.mk
.INCLUDE: target.mk

ALLTAR : $(MISC)/services.rdb

$(MISC)/services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/services.input $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/services.input

$(MISC)/services.input :
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@
