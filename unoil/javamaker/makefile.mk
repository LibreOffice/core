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



PRJ := ..
PRJNAME := unoil
TARGET := javamaker

.INCLUDE: settings.mk

RDB = $(SOLARBINDIR)$/offapi.rdb
EXTRA_RDB = $(SOLARBINDIR)$/udkapi.rdb

.INCLUDE: target.mk

.IF "$(SOLAR_JAVA)" != ""

ALLTAR: $(MISC)$/javamaker.done

$(MISC)$/javamaker.done .ERRREMOVE: $(RDB) $(EXTRA_RDB)
    $(JAVAMAKER) -BUCR -O$(CLASSDIR) $(RDB) -X$(EXTRA_RDB)
    $(TOUCH) $@

.ENDIF
