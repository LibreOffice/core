#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.12 $
#
#  last change: $Author: obo $ $Date: 2003-10-20 13:12:48 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the Java ConverterServlet example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ConverterServletExample
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)

OUT_WAR=$(OUT_COMP_CLASS)/conversion
OUT_WAR_WEB_INF=$(OUT_WAR)/WEB-INF
OUT_WAR_CLASSES=$(OUT_WAR_WEB_INF)/classes
OUT_WAR_LIB=$(OUT_WAR_WEB_INF)/lib

JAVAFILES  = \
	ConverterServlet.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/cos.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/servlet.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : \
    ConverterServletExample

include $(SETTINGS)/stdtarget.mk

$(OFFICE_CLASSES_DIR)/cos.jar : 
	@echo --------------------------------------------------------------------------------
	@echo You need the servlet support Java archive "$(QM)cos.jar$(QM)" for this example.
	@echo You can download it from "$(QM)http://www.servlets.com/cos/index.html$(QM)"
	@echo and then please copy it into your "$(QM)$(OFFICE_CLASSES_DIR)$(QM)" directory.
	@echo --------------------------------------------------------------------------------

$(OFFICE_CLASSES_DIR)/servlet.jar : 
	@echo --------------------------------------------------------------------------------
	@echo You need the servlet API "$(QM)servlet.jar$(QM)" for this example.
	@echo You can download it from "$(QM)http://java.sun.com/products/servlet/index.html$(QM)"
	@echo and then please copy it into your "$(QM)$(OFFICE_CLASSES_DIR)$(QM)" directory.
	@echo --------------------------------------------------------------------------------

$(CLASSFILES) : $(JAVAFILES) $(OFFICE_CLASSES_DIR)/cos.jar $(OFFICE_CLASSES_DIR)/servlet.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(OUT_CLASS)/conversion.war : $(CLASSFILES) ConverterServlet.html web.xml
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_WAR))
	-$(MKDIR) $(subst /,$(PS),$(OUT_WAR_WEB_INF))
	-$(MKDIR) $(subst /,$(PS),$(OUT_WAR_CLASSES))
	-$(MKDIR) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OUT_COMP_CLASS)/ConverterServlet.class) $(subst /,$(PS),$(OUT_WAR_CLASSES))
	$(COPY) ConverterServlet.html $(subst /,$(PS),$(OUT_WAR))
	$(COPY) web.xml $(subst /,$(PS),$(OUT_WAR_WEB_INF))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/jurt.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/unoil.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/ridl.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/sandbox.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/juh.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	$(COPY) $(subst /,$(PS),$(OFFICE_CLASSES_DIR)/cos.jar) $(subst /,$(PS),$(OUT_WAR_LIB))
	cd $(subst /,$(PS),$(OUT_WAR)) && jar cvfM ../../$(@F) .

ConverterServletExample : $(OUT_CLASS)/conversion.war
	@echo --------------------------------------------------------------------------------
	@echo The "$(QM)$?$(QM)" file was created and can be used.
	@echo Please see the README file for more information how to use the converter 
	@echo servlet.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
