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

PRJNAME = OOoRunner
PACKAGE = helper
TARGET = runner_helper

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar jurt.jar juh.jar unoil.jar

JAVAFILES =	APIDescGetter.java      \
            ConfigurationRead.java  \
            StreamSimulator.java	\
            AppProvider.java        \
            URLHelper.java			\
            CfgParser.java          \
            SimpleMailSender.java          \
            WindowListener.java		\
            ClParser.java           \
            OfficeWatcher.java      \
            OfficeProvider.java		\
            ComplexDescGetter.java  \
            InetTools.java          \
            ProcessHandler.java	\
            ContextMenuInterceptor.java	\
            UnoProvider.java\
            PropertyHelper.java\
            FileTools.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
