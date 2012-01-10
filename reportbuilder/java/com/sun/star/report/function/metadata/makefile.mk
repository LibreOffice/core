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



PRJ	= ..$/..$/..$/..$/..$/..$/..
PRJNAME = reportbuilder
TARGET= rpt_java_css_metadata
PACKAGE = com$/sun$/star$/report$/function$/metadata

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
#----- compile .java files -----------------------------------------

.IF "$(SYSTEM_JFREEREPORT)" == "YES"
EXTRAJARFILES = $(LIBBASE_JAR) $(LIBFORMULA_JAR)
.ELSE
.INCLUDE :  $(SOLARBINDIR)/jfreereport_version.mk
JARFILES += \
    libbase-$(LIBBASE_VERSION).jar                      \
    libformula-$(LIBFORMULA_VERSION).jar
.ENDIF

JAVAFILES       :=	AuthorFunction.java \
                    AuthorFunctionDescription.java \
                    TitleFunction.java \
                    TitleFunctionDescription.java \
                    MetaDataFunctionCategory.java

COPYFILES := $(CLASSDIR)$/$(PACKAGE)$/category.properties \
                 $(CLASSDIR)$/$(PACKAGE)$/Title-Function.properties\
                $(CLASSDIR)$/$(PACKAGE)$/category_en_US.properties \
                $(CLASSDIR)$/$(PACKAGE)$/Author-Function.properties \
                $(CLASSDIR)$/$(PACKAGE)$/Author-Function_en_US.properties \
                $(CLASSDIR)$/$(PACKAGE)$/Title-Function_en_US.properties
                 
ALLTAR : $(COPYFILES)
# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

$(COPYFILES): $$(@:f)
    +$(MKDIRHIER) $(CLASSDIR)$/$(PACKAGE)
    +$(COPY) $< $@
    
