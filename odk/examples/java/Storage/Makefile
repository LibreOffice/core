# Builds the Java Storage test example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=JavaStorageTestExample
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)

JAVAFILES  = \
	StorageFunctionality.java \
	TestHelper.java \
	StorageTest.java \
	Test01.java \
	Test02.java \
	Test03.java \
	Test04.java \
	Test05.java \
	Test06.java \
	Test07.java \
	Test08.java \
	Test09.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaStorageTestExample

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

JavaStorageTestExample : $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use following command to execute the example!
	@echo ------
	@echo make StorageFunctionality.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))

