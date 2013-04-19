#*************************************************************************
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright 2000, 2010 Oracle and/or its affiliates.
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

# Builds the ProfUNO interprocess example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

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
	@echo $(SQM)  $(SQM)soffice "$(QM)--accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
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
