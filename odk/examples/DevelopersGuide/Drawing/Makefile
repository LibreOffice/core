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

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    DrawingExamples

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

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
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

DrawViewDemo.run :
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" DrawViewDemo SimplePresentation.sxi 

GraphicExportDemo.run :
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" GraphicExportDemo SimplePresentation.sxi MyTestExport.gif 2

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
