#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.5 $
#
#  last change: $Author: vg $ $Date: 2004-12-23 09:48:22 $
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
# Builds the UCB examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=UCBExamples
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	ChildrenRetriever.java \
	DataStreamComposer.java \
	DataStreamRetriever.java \
	Helper.java \
	MyActiveDataSink.java \
	PropertiesComposer.java \
	PropertiesRetriever.java \
	ResourceCreator.java \
	ResourceManager.java \
	ResourceRemover.java \
	MyInputStream.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
	UCBExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES)

UCBExamples : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make ChildrenRetriever.run
	@echo make DataStreamComposer.run
	@echo make DataStreamRetriever.run
	@echo make PropertiesComposer.run
	@echo make PropertiesRetriever.run
	@echo make ResourceCreator.run
	@echo make ResourceManager.run
	@echo make ResourceRemover.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) resource-*
	-$(DEL) transfered-resource-*
	-$(DEL) created-resource-*
	-$(DEL) renamed-resource-*
