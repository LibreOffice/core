#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:23:26 $
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

# Builds the Drawing examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DrawingExamples
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	ChangeOrderDemo.java \
	ControlAndSelectDemo.java \
	CustomShowDemo.java \
	DrawingDemo.java \
	DrawViewDemo.java \
	FillAndLineStyleDemo.java \
	GluePointDemo.java \
	GraphicExportDemo.java \
	Helper.java \
	LayerDemo.java \
	ObjectTransformationDemo.java \
	Organigram.java \
	PageHelper.java \
	PresentationDemo.java \
	ShapeHelper.java \
	StyleDemo.java \
	TextDemo.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    DrawingExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

DrawingExamples : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make ChangeOrderDemo.run
	@echo make ControlAndSelectDemo.run
	@echo make CustomShowDemo.run
	@echo make DrawingDemo.run
	@echo make DrawViewDemo.run
	@echo make FillAndLineStyleDemo.run
	@echo make GluePointDemo.run
	@echo make GraphicExportDemo.run
	@echo make LayerDemo.run
	@echo make ObjectTransformationDemo.run
	@echo make Organigram.run
	@echo make PresentationDemo.run
	@echo make StyleDemo.run
	@echo make TextDemo.run
	@echo --------
	@echo The DrawViewDemo and the GraphicExportDemo needs parameters. Please use the
	@echo following command to start the demo if you do not want the default parameters
	@echo specified in the this makefile:
	@echo --- DrawViewDemo ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DrawViewDemo "$(QM)SourceUrl$(QM)"
	@echo --- GraphicExportDemo ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DrawViewDemo "$(QM)SourceUrl$(QM)" "$(QM)DestinationUrl$(QM)" "$(QM)index$(QM)"
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

DrawViewDemo.run :
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" DrawViewDemo SimplePresentation.sxi 

GraphicExportDemo.run :
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" GraphicExportDemo SimplePresentation.sxi MyTestExport.gif 2

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
