# Builds the Java Calc Addin example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT1_NAME=ToDo
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT1_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMPONENT1_NAME)
COMPONENT1_RDB_NAME=$(COMPONENT1_NAME).rdb
COMPONENT1_RDB=$(OUT_COMP_GEN)/$(COMPONENT1_RDB_NAME)
COMPONENT1_PACKAGE=$(OUT_BIN)/$(COMPONENT1_NAME).zip
COMPONENT1_PACKAGE_URL=$(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).zip")
COMPONENT1_JAR_NAME=$(COMPONENT1_NAME).jar
COMPONENT1_JAR=$(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT1_NAME).Manifest

REGISTERFLAG=$(OUT_MISC)$(PS)java_$(COMPONENT1_NAME)_register_component.flag

IDLFILES = XToDo.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = org/openoffice

JAVAFILES  = ToDo.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))
CLASSFILES += $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

$(COMPONENT1_NAME)_CLASSFILES = $(COMPONENT1_NAME).class \
		$(COMPONENT1_NAME)$(QUOTE)$$$(COMPONENT1_NAME)Implementation.class

$(COMPONENT1_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : JavaToDoExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_CLASS))

$(OUT_COMP_CLASS)/%.Manifest : $(OUT_COMP_GEN)
	@echo RegistrationClassName: $(basename $(@F))> $@

$(OUT_COMP_GEN)/%.urd : %.idl $(OUT_COMP_GEN)
	idlc -C -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT1_RDB) $(OUT_COMP_GEN)
	javamaker -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT1_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(OUT_COMP_GEN) $(JAVAFILES) $(GENJAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	cd $(OUT_COMP_CLASS) && jar cvfm ../$(@F) $(<F) $($(basename $(@F))_CLASSFILES)

$(OUT_BIN)/%.zip : $(OUT_COMP_GEN)/%.rdb $(OUT_CLASS)/%.jar
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_CLASS))
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(^F)
	$(DEL) $(subst /,$(PS),$(OUT_CLASS)/$(<F))

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

JavaToDoExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The ToDo Java component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Please load the "$(QM)ToDo.sxc$(QM)" document to see how this component works, see
	@echo the example description.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
