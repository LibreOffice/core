# Builds the NotesAccess java demo.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

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


# Targets
.PHONY: ALL
ALL : notesaccess
	
include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(OUT) $(OUT_CLASS) $(JAVAFILES)
	javac -classpath $(DK_CLASSPATH) -d $(OUT_CLASS) $(JAVAFILES) 

notesaccess: $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute
	@echo java -classpath $(DK_CLASSPATH) NotesAccess
	@echo --------------------------------------------------------------------------------

run :
	java -classpath $(DK_CLASSPATH) WriterSelector

