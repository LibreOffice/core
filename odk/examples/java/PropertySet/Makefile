# Builds the java demo component.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

REGISTERFLAG = $(OUT_MISC)$(PS)proptest_register_component.flag
COMPONENTJARFLAG = $(OUT_MISC)$(PS)proptest_create_component.flag

PACKAGE =
OUT_COMPONENT = $(OUT_CLASS)$(PS)PropTest

COMPONENT_JAR = $(OUT_CLASS)$(PS)PropTest.jar
COMPONENT_MANIFESTFILE = manifest

JAVAFILES  =	PropTest.java	


CLASSFILES = $(patsubst %.java,$(OUT_COMPONENT)$(PS)$(PACKAGE)$(PS)%.class,$(JAVAFILES))

SDK_CLASSPATH =	$(subst $(EMPTYSTRING) $(EMPTYSTRING),,\
		$(OFFICE_CLASSES_DIR)$(PS)jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)java_uno.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)$(PS)jut.jar\
		$(PATH_SEPARATOR)$(OUT_COMPONENT)\
		)

OFFICE_FILEURL = $(subst \,/,$(OFFICE_PROGRAM_PATH))


# Targets
.PHONY: ALL
ALL : register

include $(SETTINGS)/stdtarget.mk

$(OUT_COMPONENT) : $(OUT)
	$(MKDIR) $@

$(CLASSFILES) : $(OUT_COMPONENT) $(JAVAFILES)
	javac -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMPONENT) $(JAVAFILES) 

$(COMPONENTJARFLAG) : $(CLASSFILES)
	jar cvfm $(COMPONENT_JAR) $(COMPONENT_MANIFESTFILE) -C $(OUT_COMPONENT) .
	-$(DEL) $(REGISTERFLAG)
	@echo bla > $@

$(REGISTERFLAG) : $(COMPONENTJARFLAG)
ifneq "$(OFFICE_PROGRAM_PATH)" ""
	$(COPY) $(COMPONENT_JAR) "$(OFFICE_CLASSES_DIR)"
	java -classpath "$(SDK_CLASSPATH)" com.sun.star.tools.uno.RegComp "$(URLPREFIX)$(OFFICE_FILEURL)/applicat.rdb" register "$(URLPREFIX)$(OFFICE_FILEURL)/classes/PropTest.jar" com.sun.star.loader.Java2
	@echo bla > $@
else
	@echo ----------------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set an OFFICE_PROGRAM_PATH
	@echo  environment variable to the program directory of a valid office installation .
	@echo ----------------------------------------------------------------------------------------
endif

register: $(REGISTERFLAG)
