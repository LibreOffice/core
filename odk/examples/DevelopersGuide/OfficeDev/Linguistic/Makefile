# Builds the OfficeDevLinguistic examples of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/OfficeDevLinguisticExample

COMPONENT1_NAME=SampleHyphenator
COMPONENT1_PACKAGE = $(OUT_BIN)/$(COMPONENT1_NAME).uno.zip
COMPONENT1_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).uno.zip")
COMPONENT1_JAR_NAME = $(COMPONENT1_NAME).uno.jar
COMPONENT1_JAR = $(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT1_NAME).uno.Manifest

COMPONENT2_NAME=SampleSpellChecker
COMPONENT2_PACKAGE = $(OUT_BIN)/$(COMPONENT2_NAME).uno.zip
COMPONENT2_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT2_NAME).uno.zip")
COMPONENT2_JAR_NAME = $(COMPONENT2_NAME).uno.jar
COMPONENT2_JAR = $(OUT_CLASS)/$(COMPONENT2_JAR_NAME)
COMPONENT2_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT2_NAME).uno.Manifest

COMPONENT3_NAME=SampleThesaurus
COMPONENT3_PACKAGE = $(OUT_BIN)/$(COMPONENT3_NAME).uno.zip
COMPONENT3_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT3_NAME).uno.zip")
COMPONENT3_JAR_NAME = $(COMPONENT3_NAME).uno.jar
COMPONENT3_JAR = $(OUT_CLASS)/$(COMPONENT3_JAR_NAME)
COMPONENT3_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT3_NAME).uno.Manifest

REGISTERFLAG = $(OUT_MISC)/devguide_officedevlinguistic_register_component.flag

JAVAFILES  = \
	OneInstanceFactory.java \
	PropChgHelper.java \
	PropChgHelper_Hyph.java \
	PropChgHelper_Spell.java \
	XHyphenatedWord_impl.java \
	XMeaning_impl.java \
	XPossibleHyphens_impl.java \
	XSpellAlternatives_impl.java \
	SampleHyphenator.java \
	SampleSpellChecker.java \
	SampleThesaurus.java \
	LinguisticExamples.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

$(COMPONENT1_NAME)_CLASSFILES = XHyphenatedWord_impl.class \
	XPossibleHyphens_impl.class \
	PropChgHelper.class \
	PropChgHelper_Hyph.class \
	OneInstanceFactory.class \
	$(COMPONENT1_NAME).class

$(COMPONENT2_NAME)_CLASSFILES = XSpellAlternatives_impl.class \
	PropChgHelper_Spell.class \
	PropChgHelper.class \
	OneInstanceFactory.class \
	$(COMPONENT2_NAME).class

$(COMPONENT3_NAME)_CLASSFILES = XMeaning_impl.class \
	OneInstanceFactory.class \
	$(COMPONENT3_NAME).class

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

.PHONY: ALL
ALL : \
	OfficeDevLinguistic

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
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(<F)

$(REGISTERFLAG) : $(COMPONENT1_PACKAGE) $(COMPONENT2_PACKAGE) $(COMPONENT3_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT1_PACKAGE_URL)
	$(DEPLOYTOOL) $(COMPONENT2_PACKAGE_URL)
	$(DEPLOYTOOL) $(COMPONENT3_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

OfficeDevLinguistic : $(REGISTERFLAG) 
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make LinguisticExamples.run 
	@echo --------
	@echo Before you can run the examples the components "$(QM)SampleHyphenator$(QM)", 
	@echo "$(QM)SampleSpellChecker$(QM)" and "$(QM)SampleThesaurus$(QM)" must be deployed. 
	@echo The components will be automatically deployed if SDK_AUTO_DEPLOYMENT = YES.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT2_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT3_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
