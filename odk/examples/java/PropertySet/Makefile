# Builds the Java PropertySet component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT1_NAME=PropTest
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT1_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMPONENT1_NAME)
COMPONENT1_PACKAGE=$(OUT_BIN)/$(COMPONENT1_NAME).zip
COMPONENT1_PACKAGE_URL=$(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).zip")
COMPONENT1_JAR_NAME=$(COMPONENT1_NAME).jar
COMPONENT1_JAR=$(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT1_NAME).Manifest

REGISTERFLAG=$(OUT_MISC)$(PS)java_$(COMPONENT1_NAME)_register_component.flag

JAVAFILES  = PropTest.java

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
ALL : JavaPropertySetExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_CLASS))

$(OUT_COMP_CLASS)/%.Manifest : $(OUT_COMP_GEN)
	@echo RegistrationClassName: $(basename $(@F))> $@

$(CLASSFILES) : $(OUT_COMP_GEN) $(JAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	cd $(OUT_COMP_CLASS) && jar cvfm ../$(@F) $(<F) $($(basename $(@F))_CLASSFILES)

$(OUT_BIN)/%.zip : $(OUT_CLASS)/%.jar
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(^F)

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

JavaPropertySetExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The PropTest was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Load the PropertySet.sxw document to see how this component works. You can use 
	@echo this component inside your office installation, see the example description.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
