#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.5 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:32:37 $
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

# Builds the Java Text examples of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=JavaTextExamples
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	BookmarkInsertion.java \
	HardFormatting.java \
	StyleCreation.java \
	StyleInitialization.java \
	SWriter.java \
	TextDocumentStructure.java \
	TextReplace.java \
	WriterSelector.java \
	GraphicsInserter.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : \
    JavaTextExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

JavaTextExamples : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Before running the "$(QM)WriterSelector$(QM)" example, create a document, write 
	@echo something, select something and then run the example to see what you have selected.
	@echo -
	@echo The GraphicsInserter example loads the graphic "$(QM)./oo_smiley.gif$(QM)" into a 
	@echo new document.
	@echo -
	@echo Please use one of the following commands to execute the examples!
	@echo ------
	@echo make BookmarkInsertion.run
	@echo make HardFormatting.run
	@echo make StyleCreation.run
	@echo make StyleInitialization.run
	@echo make SWriter.run
	@echo make TextDocumentStructure.run
	@echo make TextReplace.run
	@echo make WriterSelector.run
	@echo make GraphicsInserter.run
	@echo --------
	@echo The GraphicsInserter needs parameters. Please use the following command to 
	@echo start the demo if you do not want the default parameters specified in the 
	@echo this makefile. Starting without parameters print a command line help:
	@echo --- GraphicsInserter ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" GraphicsInserter "$(QM)graphic Url|path$(QM)"
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

GraphicsInserter.run: $(OUT_COMP_CLASS)/GraphicsInserter.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@) oo_smiley.gif

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
