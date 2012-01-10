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
PACKAGE = util
TARGET = runner_util

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

JARFILES = ridl.jar jurt.jar unoil.jar

JAVAFILES =	AccessibilityTools.java	\
            BasicMacroTools.java    \
            BookmarkDsc.java		\
            ControlDsc.java			\
            dbg.java				\
            DBTools.java			\
            DefaultDsc.java			\
            DesktopTools.java		\
            DrawTools.java          \
            DynamicClassLoader.java \
            FootnoteDsc.java		\
            FormTools.java			\
            FrameDsc.java			\
            InstCreator.java		\
            InstDescr.java			\
            ParagraphDsc.java       \
            ReferenceMarkDsc.java	\
            RegistryTools.java		\
            ShapeDsc.java			\
            SOfficeFactory.java		\
            StyleFamilyDsc.java		\
            PropertyName.java		\
            SysUtils.java           \
            TableDsc.java			\
            TextSectionDsc.java		\
            XLayerImpl.java 		\
            XLayerHandlerImpl.java 	\
            XSchemaHandlerImpl.java \
            UITools.java            \
            utils.java              \
            ValueChanger.java		\
            ValueComparer.java		\
            WaitUnreachable.java    \
            WriterTools.java		\
            XInstCreator.java       \
            XMLTools.java

JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

TST:
        @echo $(JAVACLASSFILES)
