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

# Builds the ProfUNO interprocess example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=ProfUNOInterprocessExamples
OUT_APP_CLASS=$(OUT_CLASS)/$(EXAMPLE_NAME)

JAVAFILES  = \
	ConnectionAwareClient.java \
	UrlResolver.java

CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_APP_CLASS))


# Targets
.PHONY: ALL
ALL : $(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(OUT_APP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $<

$(EXAMPLE_NAME) : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Note: For these examples you need a running office listening on port 2083.
	@echo $(SQM)      $(SQM)Before you run these examples you have to start your office with
	@echo -
	@echo $(SQM)  $(SQM)soffice "$(QM)-accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
	@echo -----
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) ConnectionAwareClient.run
	@echo $(MAKE) UrlResolver.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_APP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@) "uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_CLASS))
