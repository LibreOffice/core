# Builds the Java NotesAccess example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=NotesAccessExample
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	NotesAccess.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/Notes.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaNotesAccessExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)

$(OFFICE_CLASSES_DIR)/Notes.jar : 
	@echo --------------------------------------------------------------------------------
	@echo You need the "$(QM)Notes.jar$(QM)" from Notes/Domino for this example.
	@echo You can download it from "$(QM)http://www.servlets.com/cos/index.html$(QM)"
	@echo and then please copy it into your "$(QM)$(OFFICE_CLASSES_DIR)$(QM)" directory.
	@echo --------------------------------------------------------------------------------


$(CLASSFILES) : $(OFFICE_CLASSES_DIR)/Notes.jar $(OUT_COMP_CLASS) $(JAVAFILES) 
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

JavaNotesAccessExample : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make NotesAccess.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) "" "" "" "Stocks.nsf"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
