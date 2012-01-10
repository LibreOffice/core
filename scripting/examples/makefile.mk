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
PRJNAME=scriptinginstaller
TARGET=scriptinginstaller
no_common_build_zip=true

.INCLUDE : settings.mk
.IF "$(L10N_framework)"==""
.IF "$(SOLAR_JAVA)"!=""
ZIP1FLAGS=-r
ZIP1TARGET=ScriptsJava
ZIP1DIR=$(OUT)$/class$/examples
ZIP1LIST=java
.ENDIF

ZIP2FLAGS=-r
ZIP2TARGET=ScriptsBeanShell
ZIP2LIST=beanshell/HelloWorld beanshell/Highlight beanshell/WordCount beanshell/Capitalise beanshell/MemoryUsage 

ZIP3FLAGS=-r
ZIP3TARGET=ScriptsJavaScript
ZIP3LIST=javascript

ZIP4FLAGS=-r
ZIP4TARGET=ScriptsPython
ZIP4LIST=python/HelloWorld.py python/Capitalise.py python/pythonSamples

ZIP5FLAGS=-r
ZIP5TARGET=scriptbindinglib
ZIP5DIR=$(PRJ)$/workben$/bindings
ZIP5LIST=*.xlb Highlight.xdl
.ENDIF
.INCLUDE : target.mk
