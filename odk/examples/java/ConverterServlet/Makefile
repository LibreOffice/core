# Builds the ConvertServlet java demo.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

JAVAFILES  = \
	ConverterServlet.java \

CLASSFILES = $(patsubst %.java,$(OUT_CLASS)$(PS)%.class,$(JAVAFILES) )

SDK_CLASSPATH =	$(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)$(PS)jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)servlet.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)cos.jar\
		$(PATH_SEPARATOR)$(OUT_CLASS))


# Targets
.PHONY: ALL
ALL : converterservlet

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(OUT) $(OUT_CLASS) $(JAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_CLASS) $(JAVAFILES) 

converterservlet: $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please make the resulting class known to your web server
	@echo --------------------------------------------------------------------------------
