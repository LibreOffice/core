#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.2 $
#
#  last change: $Author: hr $ $Date: 2004-11-09 11:21:44 $
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

# Builds the OOoBean of the Developers Guide

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/OOoBeanViewer

JAR1_NAME=OOoBeanViewer
JAR1_JAR_NAME = $(JAR1_NAME).jar
JAR1_JAR = $(OUT_CLASS)/$(JAR1_JAR_NAME)
JAR1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(JAR1_NAME).Manifest

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = com/sun/star/comp/beans

JAVAFILES  = \
	OOoBeanViewer.java \

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/$(PACKAGE)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/officebean.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : \
    OOoBeanViewer

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: OOoBeanViewer> $@
	@echo Class-Path: sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar officebean.jar>> $@
	@echo Name: OOoBeanViewer.class>> $@
	@echo Java-Bean: True>> $@

$(CLASSFILES) : $(JAVAFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES)

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) OfficeIconColor16.jpg $(subst /,$(PS),$(OUT_COMP_CLASS))
	$(COPY) OfficeIconColor32.jpg $(subst /,$(PS),$(OUT_COMP_CLASS))
	$(COPY) OfficeIconMono16.jpg $(subst /,$(PS),$(OUT_COMP_CLASS))
	$(COPY) OfficeIconMono32.jpg $(subst /,$(PS),$(OUT_COMP_CLASS))
	jar cvfm $@ $< -C $(OUT_COMP_CLASS) .


OOoBeanViewer: $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make OOoBeanViewer.run
#	@echo ------
#	@echo If you want to run the $(JAR1_JAR) file please set your
#	@echo CLASSPATH = $(SDK_CLASSPATH)
#	@echo Start the example with java -jar $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------

%.run: $(JAR1_JAR)
	java -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

%.dbg: $(JAR1_JAR)
	jdb -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst /,$(PS),$(JAR1_JAR))

