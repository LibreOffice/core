# Builds the Charts examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/ChartExamples

COMPONENT1_NAME=JavaSampleChartAddIn
COMPONENT1_PACKAGE = $(OUT_BIN)/$(COMPONENT1_NAME).uno.zip
COMPONENT1_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).uno.zip")
COMPONENT1_JAR_NAME = $(COMPONENT1_NAME).uno.jar
COMPONENT1_JAR = $(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT1_NAME).uno.Manifest

REGISTERFLAG = $(OUT_MISC)/devguide_$(COMPONENT1_NAME)_register_component.flag

JAVAFILES  = \
	   CalcHelper.java	\
	   ChartHelper.java	\
	   ChartInCalc.java	\
	   ChartInDraw.java	\
	   ChartInWriter.java	\
	   Helper.java	\
	   JavaSampleChartAddIn.java	\
	   ListenAtCalcRangeInDraw.java \
	   SelectionChangeListener.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

$(COMPONENT1_NAME)_CLASSFILES = $(COMPONENT1_NAME).class

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : ChartExamples

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(OUT_COMP_CLASS) && jar cvfm ../$(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(OUT_BIN)/%.zip : $(OUT_CLASS)/%.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(^F)

$(REGISTERFLAG) : $(COMPONENT1_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT1_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

ChartExamples : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo When you run the "$(QM)SelectionChangeListener$(QM)" example please select the
	@echo legend or the title to add a SelectionChangeListener. The example terminates
	@echo when the document is closed.
	@echo -
	@echo Please use the following commands to execute the examples!
	@echo -
	@echo make ChartInCalc.run
	@echo make ChartInDraw.run
	@echo make ChartInWriter.run
	@echo make ListenAtCalcRangeInDraw.run
	@echo make SelectionChangeListener.run
	@echo --------
	@echo The Chart add-in component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Load the "$(QM)AddinChart.sxc$(QM)" document in your office to see the new chart type,
	@echo and see the example description.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
