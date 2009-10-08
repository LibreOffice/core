# Builds the java demo component.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

# Include settings makefile depending on platform/compiler
ifeq "$(MAKECMDGOALS)" "win_microcxx"
        -include $(SETTINGS)/win_microcxx_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_BIN)
endif   
ifeq "$(MAKECMDGOALS)" "solaris_workshopcxx"
        -include $(SETTINGS)/solaris_workshopcxx_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_LIB)
endif   
ifeq "$(MAKECMDGOALS)" "linux_gcc"
        -include $(SETTINGS)/linux_gcc_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_LIB)
endif

# Define non-platform/compiler specific settings

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings


# Targets
.PHONY: ALL
ALL :
	@echo -------------------------------------------------------------
	@echo You must supply a target where TARGET is one of the following:
	@echo 
	@echo win_microcxx        : Windows using Microsoft C++ compiler/linker
	@echo solaris_workshopcxx : Solaris using Sun Workshop C++ compiler
	@echo linux_gcc           : Linux using gcc
	@echo -------------------------------------------------------------

REGISTERFLAGFILE = $(OUT_MISC)$(PS)register_components_XFlatXml_is_ok.flag

OUT_COMPONENT = $(OUT_CLASS)$(PS)XFlatXml
COMPONENT_RDB = $(OUT_BIN)$(PS)XFlatXml.rdb

COMPONENT_JAR = $(OUT_CLASS)$(PS)XFlatXml.jar
COMPONENT_MANIFESTFILE = manifest

IDLFILES = XFilterAdapter.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = com$(PS)sun$(PS)star$(PS)document

JAVAFILES  =	XOutputStreamToOutputStreamAdapter.java XInputStreamToInputStreamAdapter.java TestStream.java XFlatXml.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_MISC)$(PS)$(PACKAGE)$(PS)%.java,$(IDLFILES) )
GENURDFILES = $(patsubst %.idl,$(OUT_MISC)$(PS)%.urd,$(IDLFILES) )

CLASSFILES = $(patsubst %.java,$(OUT_COMPONENT)$(PS)%.class,$(JAVAFILES) )

CLASSFILES += $(patsubst %.java,$(OUT_COMPONENT)$(PS)$(PACKAGE)$(PS)%.class,$(GENJAVAFILES) )

DK_CLASSPATH =	$(subst $(EMPTYSTRING) $(EMPTYSTRING),, \
		$(CLASSES_DIR)$(PS)jurt.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)unoil.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)ridl.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)sandbox.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)java_uno.jar\
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)juh.jar\
        $(PATH_SEPARATOR)$(CLASSPATH) \
		$(PATH_SEPARATOR)$(OUT_COMPONENT) \
		)

OFFICE_FILEURL = $(subst \,/,$(OFFICE_PROGRAM_PATH))
OFFICE_CLASSPATH = $(subst $(CLASSES_DIR),$(OFFICE_PROGRAM_PATH)$(PS)classes, $(DK_CLASSPATH))

win_microcxx : $(COMPONENT_JAR) $(REGISTERFLAGFILE)

solaris_workshopcxx : $(COMPONENT_JAR) $(REGISTERFLAGFILE)

linux_gcc : $(COMPONENT_JAR) $(REGISTERFLAGFILE)

include $(SETTINGS)/stdtarget.mk

$(OUT_COMPONENT) : $(OUT) $(OUT_CLASS)
	$(MKDIR) $(OUT_COMPONENT)

$(GENJAVAFILES) : $(IDLFILES)
	$(BIN_DIR)$(PS)idlc -I. -I$(IDL_DIR) -O$(OUT_MISC) $(IDLFILES)
	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) /UCR $(GENURDFILES)
	$(BIN_DIR)$(PS)regmerge $(COMPONENT_RDB) / $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	$(BIN_DIR)$(PS)javamaker -BUCR -Tcom.sun.star.document.XFilterAdapter -O$(OUT_MISC) $(COMPONENT_RDB)
	$(BIN_DIR)$(PS)cppumaker -BUCR -Tcom.sun.star.document.XFilterAdapter -O$(OUT)$(PS)inc$(PS)examples $(COMPONENT_RDB)

$(CLASSFILES) : $(OUT_COMPONENT) $(GENJAVAFILES) $(JAVAFILES)
	javac -classpath $(DK_CLASSPATH) -d $(OUT_COMPONENT) $(GENJAVAFILES) $(JAVAFILES)

$(COMPONENT_JAR) : $(CLASSFILES) 
	jar cvfm $(COMPONENT_JAR) Manifest -C $(OUT_COMPONENT) .

#$(REGISTERFLAGFILE) : $(REGISTERFLAGFILE)
$(REGISTERFLAGFILE) : FORCE
ifneq "$(OFFICE_PROGRAM_PATH)" ""
	$(BIN_DIR)$(PS)regmerge $(OFFICE_PROGRAM_PATH)$(PS)applicat.rdb /UCR $(GENURDFILES)
	$(COPY) $(COMPONENT_JAR) $(OFFICE_PROGRAM_PATH)$(PS)classes
	java -classpath $(OFFICE_CLASSPATH) com.sun.star.tools.uno.RegComp "$(URLPREFIX)$(OFFICE_FILEURL)/applicat.rdb" register "$(URLPREFIX)$(OFFICE_FILEURL)/classes/XFlatXml.jar" com.sun.star.loader.Java2
	@echo bla > $(REGISTERFLAGFILE)
else
	@echo ----------------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set an OFFICE_PROGRAM_PATH
	@echo  environment variable to the program directory of a valid office installation .
	@echo ----------------------------------------------------------------------------------------
endif


FORCE :
