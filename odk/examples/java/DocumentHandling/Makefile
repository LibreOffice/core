#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: hr $ $Date: 2003-06-30 15:55:01 $
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

# Builds the Java DocumentHandling examples of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DocumentHandlingExamples
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	DocumentConverter.java \
	DocumentLoader.java \
	DocumentPrinter.java \
	DocumentSaver.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaDocumentHandlingExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

JavaDocumentHandlingExamples : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo The DocumentConverter search the "$(QM)./test$(QM)" directory for documents, convert
	@echo them using using the "$(QM)MS Word 97$(QM)" filter and the extension "$(QM).doc$(QM)".
	@echo The list of possible filter names can changed but a normally uptodate list can be found 
	@echo on "$(QM)http://www.openoffice.org/files/documents/25/111/filter_description.html$(QM)".
	@echo -
	@echo The DocumentLoader loads the document "$(QM)./test/test1.sxw$(QM)".
	@echo -
	@echo The DocumentPrinter prints the document "$(QM)./test/test1.sxw$(QM)" using the
	@echo the specified printer. If the printer is unknown the defualt printer is used.
	@echo -
	@echo The DocumentSaver loads the document "$(QM)./test/test1.sxw$(QM)" and save it
	@echo under "$(QM)./test/testsave.sxw$(QM)".
	@echo -
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make DocumentConverter.run
	@echo make DocumentLoader.run
	@echo make DocumentPrinter.run
	@echo make DocumentSaver.run
	@echo --------
	@echo The examples need parameters. Please use one the following commands to 
	@echo start the demo if you do not want the default parameters specified in the 
	@echo this makefile. Starting without parameters print a command line help:
	@echo --- DocumentConverter ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DocumentConverter "$(QM)directory$(QM)" "$(QM)filter name$(QM)" "$(QM)extension$(QM)"
	@echo --- DocumentLoader ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DocumentLoader "$(QM)Url|path$(QM)"
	@echo --- DocumentPrinter ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DocumentPrinter "$(QM)printername$(QM)" "$(QM)filename$(QM)" "$(QM)pages$(QM)"
	@echo --- DocumentSaver ---
	@echo java -classpath "$(QM)$(SDK_CLASSPATH)$(QM)" DocumentSaver "$(QM)load Url|path$(QM)" "$(QM)save Url|path$(QM)"
	@echo --------------------------------------------------------------------------------

DocumentConverter.run: $(OUT_COMP_CLASS)/DocumentConverter.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "./test" "MS Word 97" "doc"

DocumentLoader.run: $(OUT_COMP_CLASS)/DocumentLoader.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "./test/test1.sxw"

DocumentPrinter.run: $(OUT_COMP_CLASS)/DocumentPrinter.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "my_printer" "./test/test1.sxw" 1

DocumentSaver.run: $(OUT_COMP_CLASS)/DocumentSaver.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "./test/test1.sxw" "./test/testsave.sxw"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
