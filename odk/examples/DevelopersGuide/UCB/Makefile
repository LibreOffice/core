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

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
	UCBExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

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
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
