# Builds the java demo component.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

REGISTERFLAG = $(OUT_MISC)$(PS)todo_register_component.flag
COMPONENTJARFLAG = $(OUT_MISC)$(PS)todo_create_component_jar.flag
TYPEFLAG= $(OUT_MISC)$(PS)todo_type_generation.flag
JAVAFLAG= $(OUT_MISC)$(PS)todo_javac.flag

OUT_COMPONENT = $(OUT_CLASS)$(PS)todo
COMPONENT_RDB = $(OUT_BIN)$(PS)ToDo.rdb

COMPONENT_JAR = $(OUT_CLASS)$(PS)ToDo.jar
COMPONENT_MANIFESTFILE = Manifest

IDLFILES = XToDo.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = org$(PS)OpenOffice

JAVAFILES  =	ToDo.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_MISC)$(PS)$(PACKAGE)$(PS)%.java,$(IDLFILES))
GENURDFILES = $(subst \\,\,$(patsubst %.idl,$(OUT_MISC)$(PS)%.urd,$(IDLFILES)))

CLASSFILES = $(patsubst %.java,$(OUT_COMPONENT)$(PS)%.class,$(JAVAFILES))

CLASSFILES += $(patsubst %.java,$(OUT_COMPONENT)$(PS)$(PACKAGE)$(PS)%.class,$(GENJAVAFILES))

DK_CLASSPATH =	$(subst $(EMPTYSTRING) $(EMPTYSTRING),,\
		$(CLASSES_DIR)$(PS)jurt.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)unoil.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)ridl.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)sandbox.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)java_uno.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)juh.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)jut.jar\
		$(PATH_SEPARATOR)$(OUT_COMPONENT)\
		)

OFFICE_FILEURL = $(subst \,/,$(OFFICE_PROGRAM_PATH))
OFFICE_CLASSPATH = $(subst \\,\,$(subst $(CLASSES_DIR),$(OFFICE_PROGRAM_PATH)$(PS)classes,$(DK_CLASSPATH)))


# Targets
.PHONY: ALL
ALL : register

include $(SETTINGS)/stdtarget.mk

$(OUT_COMPONENT) : $(OUT) $(OUT_CLASS)
	$(MKDIR) $@

$(TYPEFLAG) : $(IDLFILES)
	$(BIN_DIR)$(PS)idlc -I. -I$(IDL_DIR) -O$(OUT_MISC) $^
	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) /UCR $(GENURDFILES)
	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) / $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	$(BIN_DIR)$(PS)javamaker -BUCR -Torg.OpenOffice.XToDo -O$(OUT_MISC) $(COMPONENT_RDB)
	@echo bla > $@

$(JAVAFLAG) : $(OUT_COMPONENT) $(TYPEFLAG) $(JAVAFILES)
	javac -classpath $(DK_CLASSPATH) -d $(OUT_COMPONENT) $(GENJAVAFILES) $(JAVAFILES)
	@echo bla > $@

$(COMPONENTJARFLAG) : $(JAVAFLAG)
	jar cvfm $(COMPONENT_JAR) $(COMPONENT_MANIFESTFILE) -C $(OUT_COMPONENT) .
	-$(DEL) $(REGISTERFLAG)
	@echo bla > $@

$(REGISTERFLAG) : $(COMPONENTJARFLAG)
ifneq "$(OFFICE_PROGRAM_PATH)" ""
	$(BIN_DIR)$(PS)regmerge "$(URLPREFIX)$(OFFICE_FILEURL)/applicat.rdb" /UCR $(GENURDFILES)
	$(COPY) $(COMPONENT_JAR) "$(OFFICE_PROGRAM_PATH)/classes"
	java -classpath "$(OFFICE_CLASSPATH)" com.sun.star.tools.uno.RegComp "$(URLPREFIX)$(OFFICE_FILEURL)/applicat.rdb" register "$(URLPREFIX)$(OFFICE_FILEURL)/classes/ToDo.jar" com.sun.star.loader.Java2
	@echo bla > $@
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set an 
	@echo  OFFICE_PROGRAM_PATH environment variable to a valid office installation.
	@echo --------------------------------------------------------------------------------
endif

register: $(REGISTERFLAG)
