# Builds the java demo component.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ToDo
OUT_COMPONENT = $(OUT_CLASS)$(PS)$(COMPONENT_NAME)
COMPONENT_RDB_NAME = $(COMPONENT_NAME).rdb
COMPONENT_RDB = $(OUT_BIN)$(PS)$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE_NAME = $(COMPONENT_NAME).zip
COMPONENT_PACKAGE_URL = $(subst \\,/,"file:///$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_PACKAGE_NAME)")
COMPONENT_JAR_NAME = $(COMPONENT_NAME).jar
COMPONENT_JAR = $(OUT_CLASS)$(PS)$(COMPONENT_JAR_NAME)
COMPONENT_MANIFESTFILE = $(OUT_COMPONENT)$(PS)$(COMPONENT_NAME).Manifest

REGISTERFLAG = $(OUT_MISC)$(PS)$(COMPONENT_NAME)_register_component.flag
COMPONENTJARFLAG = $(OUT_MISC)$(PS)$(COMPONENT_NAME)_create_component_jar.flag
TYPEFLAG= $(OUT_MISC)$(PS)$(COMPONENT_NAME)_type_generation.flag
JAVAFLAG= $(OUT_MISC)$(PS)$(COMPONENT_NAME)_javac.flag

IDLFILES = XToDo.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = org$(PS)openoffice

JAVAFILES  =	ToDo.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_MISC)$(PS)$(PACKAGE)$(PS)%.java,$(IDLFILES))
GENURDFILES = $(subst \\,\,$(patsubst %.idl,$(OUT_MISC)$(PS)%.urd,$(IDLFILES)))

CLASSFILES = $(patsubst %.java,$(OUT_COMPONENT)$(PS)%.class,$(JAVAFILES))

CLASSFILES += $(patsubst %.java,$(OUT_COMPONENT)$(PS)$(PACKAGE)$(PS)%.class,$(GENJAVAFILES))
COMPONENT_CLASSFILES = $(COMPONENT_NAME).class $(COMPONENT_NAME)$$$(COMPONENT_NAME)Implementation.class
COMPONENT_CLASSFILES += $(subst $(OUT_MISC)$(PS),,$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH =	$(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)$(PS)jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)java_uno.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)jut.jar\
		$(PATH_SEPARATOR)$(OUT_COMPONENT))

OFFICE_FILEURL = $(subst \,/,$(OFFICE_PROGRAM_PATH))


# Targets
.PHONY: ALL
ALL : register

include $(SETTINGS)/stdtarget.mk

$(OUT_COMPONENT) : $(OUT) $(OUT_CLASS)
	$(MKDIR) $@

$(OUT_COMPONENT)$(PS)$(COMPONENT_NAME).Manifest : $(OUT_COMPONENT)
	@echo RegistrationClassName: $(COMPONENT_NAME)> $@

$(TYPEFLAG) : $(IDLFILES)
	$(BIN_DIR)$(PS)idlc -I. -I$(IDL_DIR) -O$(OUT_MISC) $^
	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) /UCR $(GENURDFILES)
#	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) / $(DKREGISTRYNAME)
	$(BIN_DIR)$(PS)javamaker -BUCR -Torg.openoffice.XToDo -O$(OUT_MISC) $(COMPONENT_RDB) $(DKREGISTRYNAME)
	@echo bla > $@

$(JAVAFLAG) : $(OUT_COMPONENT) $(TYPEFLAG) $(JAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMPONENT) $(GENJAVAFILES) $(JAVAFILES)
	@echo bla > $@

$(COMPONENTJARFLAG) : $(COMPONENT_MANIFESTFILE) $(JAVAFLAG)
#	jar cvfm $(COMPONENT_JAR) $(COMPONENT_MANIFESTFILE) -C $(OUT_COMPONENT) .
	cd $(OUT_COMPONENT) && jar cvfm $(COMPONENT_JAR) $(COMPONENT_MANIFESTFILE) $(COMPONENT_CLASSFILES)
	$(COPY) $(COMPONENT_JAR) $(OUT_BIN)
	cd $(OUT_BIN) && jar cvfM $(COMPONENT_PACKAGE_NAME) $(COMPONENT_JAR_NAME) $(COMPONENT_RDB_NAME)
	$(DEL) $(OUT_BIN)$(PS)$(COMPONENT_JAR_NAME)
	-$(DEL) $(REGISTERFLAG)
	@echo $(COMPONENT_PACKAGE_URL)
	@echo bla > $@

$(REGISTERFLAG) : $(COMPONENTJARFLAG)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	cd $(OFFICE_PROGRAM_PATH) && pkgchk $(COMPONENT_PACKAGE_URL)
	@echo bla > $@
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. 
	@echo --------------------------------------------------------------------------------
endif

register: $(REGISTERFLAG)
