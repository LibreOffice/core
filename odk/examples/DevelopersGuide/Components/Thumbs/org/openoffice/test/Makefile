# Builds the SpreadSheet examples of the Developers Guide.

PRJ=../../../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

include ../../../thumbs.mk

IDLFILES = XImageShrink.idl

IDLFILES2 = ImageShrink.idl

PACKAGE = org/openoffice/test

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))
GENURDFILES += $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES2))

CLASSFILES = $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
	$(CLASSFILES)

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	idlc -I. -I../../.. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $^

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT1_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javamaker -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT1_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(GENJAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES)

.PHONY: clean
clean :
	-$(DEL) $(subst /,$(PS),$(CLASSFILES))
