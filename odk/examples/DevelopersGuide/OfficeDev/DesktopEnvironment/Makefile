# Builds the OfficeDevDestopEnvironment example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/OfficeDevDesktopEnv

JAR1_NAME=DesktopExample
JAR1_JAR_NAME = $(JAR1_NAME).jar
JAR1_JAR = $(OUT_CLASS)/$(JAR1_JAR_NAME)
JAR1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(JAR1_NAME).Manifest

# often the java files are structured in a hierarchy similar to the package, 
# for the example we know the package
PACKAGE = OfficeDev/samples/DesktopEnvironment

JAVAFILES  = \
		   CustomizeView.java \
		   Desk.java \
		   DocumentView.java \
		   FunctionHelper.java \
		   Interceptor.java \
		   IOnewayLink.java \
		   IShutdownListener.java \
		   JavaWindowPeerFake.java \
		   NativeView.java \
		   OfficeConnect.java \
		   OnewayExecutor.java \
		   StatusListener.java \
		   StatusView.java \
		   ViewContainer.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/$(PACKAGE)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

ifeq "$(OS)" "WIN"
SUBDIR= nativelib/windows 
else
SUBDIR= nativelib/unix 
endif

# Targets
.PHONY: ALL
ALL : \
	$(SUBDIR) \
	OfficeDevDesktopEnvExample

include $(SETTINGS)/stdtarget.mk

.PHONY : $(SUBDIR)
$(SUBDIR) :
	$(MAKE) -C $@	  

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: OfficeDev.samples.DesktopEnvironment.Desk> $@
	@echo Class-Path: sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	jar cvfm $@ $< -C $(OUT_COMP_CLASS) .

OfficeDevDesktopEnvExample : $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make DesktopExample.run
	@echo ------
	@echo If you want to run the $(JAR1_JAR) file please set your
	@echo CLASSPATH = $(SDK_CLASSPATH)
	@echo Start the example with jar -jar $(JAR1_JAR)
	@echo --------------------------------------------------------------------------------

%.run: $(JAR1_JAR)
	java -classpath "$(SDK_CLASSPATH)" OfficeDev.samples.DesktopEnvironment.Desk

.PHONY: clean
clean :
	$(MAKE) -C $(SUBDIR) clean
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst /,$(PS),$(JAR1_JAR))
