# Builds the Java ConverterServlet example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ConverterServletExample
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)

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

$(OUT_COMP_CLASS) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)

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

$(CLASSFILES) : $(OUT_COMP_CLASS) $(JAVAFILES) $(OFFICE_CLASSES_DIR)/cos.jar $(OFFICE_CLASSES_DIR)/servlet.jar
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

ConverterServletExample : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please see the README file for more information how to use the converter 
	@echo servlet.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
