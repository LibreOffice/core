#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:25:00 $
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

# Builds the OfficeDevDestopEnvironment example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/OfficeDevDesktopEnv

JAR1_NAME=DesktopExample
JAR1_JAR_NAME = $(JAR1_NAME).jar
JAR1_JAR = $(OUT_CLASS)/$(JAR1_JAR_NAME)
JAR1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(JAR1_NAME).Manifest

# often the java files are structured in a hierarchy similar to the package, 
# for the example we know the package
PACKAGE = OfficeDev/samples/DesktopEnvironment

JAVAFILES  = \
		   CustomizeView.java \
		   Desk.java \
		   DocumentView.java \
		   FunctionHelper.java \
		   Interceptor.java \
		   IOnewayLink.java \
		   IShutdownListener.java \
		   JavaWindowPeerFake.java \
		   NativeView.java \
		   OfficeConnect.java \
		   OnewayExecutor.java \
		   StatusListener.java \
		   StatusView.java \
		   ViewContainer.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/$(PACKAGE)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

ifeq "$(OS)" "WIN"
SUBDIR= nativelib/windows 
else
SUBDIR= nativelib/unix 
endif

# Targets
.PHONY: ALL
ALL : \
	$(SUBDIR) \
	OfficeDevDesktopEnvExample

include $(SETTINGS)/stdtarget.mk

.PHONY : $(SUBDIR)
$(SUBDIR) :
	$(MAKE) -C $@	  

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: OfficeDev.samples.DesktopEnvironment.Desk> $@
	@echo Class-Path: sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(JAR1_JAR) : $(JAR1_MANIFESTFILE) $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvfm $@ $< -C $(OUT_COMP_CLASS) .

OfficeDevDesktopEnvExample : $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make DesktopExample.run
	@echo ------
	@echo If you want to run the $(JAR1_JAR) file please set your
	@echo CLASSPATH = $(SDK_CLASSPATH)
	@echo Start the example with jar -jar $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------

%.run: $(JAR1_JAR)
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" OfficeDev.samples.DesktopEnvironment.Desk

.PHONY: clean
clean :
	$(MAKE) -C $(SUBDIR) clean
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(JAR1_JAR)))
