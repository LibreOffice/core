# Builds the DevGuide Jobs Addon component example of the SDK.

PRJ=../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT1_NAME=AsyncJob
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT1_NAME)
OUT_COMP_MISC=$(OUT_MISC)/$(COMPONENT1_NAME)
COMPONENT1_PACKAGE=$(OUT_BIN)/$(COMPONENT1_NAME).uno.zip
COMPONENT1_PACKAGE_URL=$(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).uno.zip")
COMPONENT1_JAR_NAME=$(COMPONENT1_NAME).uno.jar
COMPONENT1_JAR=$(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT1_NAME).uno.Manifest

REGISTERFLAG=$(OUT_COMP_MISC)$(PS)java_$(COMPONENT1_NAME)_register_component.flag

JAVAFILES  = AsyncJob.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : DevGuideJobsAddon

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	jar cvfm $@ $< -C $(OUT_COMP_CLASS) .

$(OUT_BIN)/%.zip : $(OUT_CLASS)/%.jar Addons.xcu Jobs.xcu
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) Addons.xcu $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) Jobs.xcu $(subst /,$(PS),$(OUT_COMP_MISC))
	cd $(OUT_COMP_MISC) && jar cvfM ../../bin/$(@F) $(^F)

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

DevGuideJobsAddon : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The "$(QM)AsyncJob$(QM)" addon component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_MISC))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
