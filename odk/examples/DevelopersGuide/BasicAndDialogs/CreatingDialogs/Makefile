# Builds the CreatingDialogs example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/BaDCreatingDialogsExample

COMPONENT1_NAME=SampleDialog
COMPONENT1_PACKAGE = $(OUT_BIN)/$(COMPONENT1_NAME).uno.zip
COMPONENT1_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).uno.zip")
COMPONENT1_JAR_NAME = $(COMPONENT1_NAME).uno.jar
COMPONENT1_JAR = $(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT1_NAME).uno.Manifest

REGISTERFLAG = $(OUT_MISC)/devguide_basicanddialogs_creatingdialogs_register_component.flag

# often the java files are structured in a hierarchy similar to the package, 
# for the example we know the package
PACKAGE = com.sun.star.comp.sdk.examples

JAVAFILES  = \
	   SampleDialog.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

$(COMPONENT1_NAME)_CLASSFILES = SampleDialog.class \

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : CreatingDialogsExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(PACKAGE).$(basename $(basename $(@F)))> $@

$(OUT_COMP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $<

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	jar cvfm $@ $< -C $(OUT_COMP_CLASS) .

$(OUT_BIN)/%.zip : $(OUT_CLASS)/%.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(<F)


$(REGISTERFLAG) : $(COMPONENT1_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
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

CreatingDialogsExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The SampleDialog Java component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Please load the "$(QM)CreatingDialogs.sxw$(QM)" document to use the component or use it
	@echo by yourself in your office installation, see the example description.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
