# Builds the NotesAccess java demo.

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

JAVAFILES  = \
	NotesAccess.java \
	
		
CLASSFILES = $(patsubst %.java,$(OUT_CLASS)$(PS)%.class,$(JAVAFILES) )

DK_CLASSPATH =	$(subst $(EMPTYSTRING) $(EMPTYSTRING),, \
		$(CLASSES_DIR)$(PS)jurt.jar \
  		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)unoil.jar \
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)ridl.jar \
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)sandbox.jar \
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)juh.jar \
		$(PATH_SEPARATOR)$(CLASSES_DIR)$(PS)Notes.jar \
		$(PATH_SEPARATOR)$(OUT_CLASS) \
		)

win_microcxx : $(CLASSFILES)

solaris_workshopcxx : $(CLASSFILES)

linux_gcc : $(CLASSFILES)
	
include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(OUT) $(OUT_CLASS) $(JAVAFILES)
	javac -classpath $(DK_CLASSPATH) -d $(OUT_CLASS) $(JAVAFILES) 
	@echo ----
	@echo Please use the following command to execute
	@echo java -classpath $(DK_CLASSPATH) NotesAccess
	@echo ----

