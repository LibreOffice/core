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

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaTextExamples

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)

$(CLASSFILES) : $(OUT_COMP_CLASS) $(JAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

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
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

GraphicsInserter.run: $(OUT_COMP_CLASS)/GraphicsInserter.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) oo_smiley.gif

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
