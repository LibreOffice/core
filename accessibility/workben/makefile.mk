# *************************************************************
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
# *************************************************************
# copied from settings.mk
SOLARBINDIR=$(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)

# Please modify the following lines to match your environment:
#   If you use the run: target at the end of the file, then adapt pipe name
PIPE_NAME = $(USER)

# The following variables probably don't need to be changed.
JAVA = java
#   The JAR_PATH points to the jar files of your local office installation.
JAR_PATH = $(SOLARBINDIR)$/


# The rest of this makefile should not need to be touched.

JAR_FILES =		\
    unoil.jar	\
    ridl.jar	\
    jurt.jar	\
    juh.jar		\
    java_uno.jar


JAVA_CLASSPATHS := 			\
    ..$/$(INPATH)$/class						\
    $(foreach,i,$(JAR_FILES) $(JAR_PATH)$i) \
    $(CLASSPATH)

CLASSPATH !:=$(JAVA_CLASSPATHS:t$(PATH_SEPERATOR))

all:
    build

# Example of how to run the work bench.
run:
    +$(JAVA) -classpath "$(CLASSPATH)" org/openoffice/accessibility/awb/AccessibilityWorkBench -p $(PIPE_NAME)

runjar:
    +$(JAVA) -classpath "$(CLASSPATH)" -jar AccessibilityWorkBench.jar -p $(PIPE_NAME)
