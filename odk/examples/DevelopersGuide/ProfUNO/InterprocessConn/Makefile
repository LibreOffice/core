# Builds the ProfUNO interprocess example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ProfUNOInterprocessExamples
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	ConnectionAwareClient.java \
	UrlResolver.java

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
	ProfUNOInterprocessExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

ProfUNOInterprocessExamples : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make ConnectionAwareClient.run
	@echo make UrlResolver.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
