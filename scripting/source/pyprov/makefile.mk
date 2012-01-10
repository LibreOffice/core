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

PRJNAME=scripting
TARGET=pyprov

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------


# --- Targets ------------------------------------------------------
ALL : ALLTAR \
        $(DLLDEST)$/pythonscript.py	\
    $(DLLDEST)$/officehelper.py	\
        $(DLLDEST)$/mailmerge.py

$(DLLDEST)$/%.py: %.py
    cp $? $@

.INCLUDE :  target.mk

ALLTAR : $(MISC)/mailmerge.component $(MISC)/pythonscript.component

$(MISC)/mailmerge.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        mailmerge.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_PYTHON)mailmerge' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt mailmerge.component

$(MISC)/pythonscript.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt pythonscript.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_PYTHON)pythonscript' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt pythonscript.component
