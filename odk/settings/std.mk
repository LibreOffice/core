ifneq "$(OO_SDK_OUTPUT_DIR)" ""
OUT=$(subst \,/,$(OO_SDK_OUTPUT_DIR))/$(OO_SDK_NAME)/$(OS)example.out
else
OUT=$(PRJ)/$(OS)example.out
endif

UNOPKG_EXT=uno.pkg
UNOOXT_EXT=oxt

OUT_BIN=$(OUT)/bin
OUT_INC=$(OUT)/inc
OUT_SLO=$(OUT)/slo
OUT_LIB=$(OUT)/lib
OUT_SLB=$(OUT)/slb
OUT_MISC=$(OUT)/misc
OUT_OBJ=$(OUT)/obj
OUT_CLASS=$(OUT)/class
IDL_DIR=$(PRJ)/idl
BIN_DIR=$(PRJ)/$(PLATFORM)/bin
CLASSES_DIR=$(PRJ)/classes
OFFICE_CLASSES_DIR=$(subst \,/,$(OFFICE_PROGRAM_PATH))/classes
COMP_PACKAGE_DIR=$(subst /,$(PS),$(OUT_BIN))

ifneq "$(OO_SDK_URE_HOME)" ""
URE_TYPES=$(subst \\,\,$(URE_MISC)$(PS)types.rdb)
URE_SERVICES=$(subst \\,\,$(URE_MISC)$(PS)services.rdb)
else
URE_TYPES=$(subst \\,\,$(OFFICE_PROGRAM_PATH)$(PS)types.rdb)
URE_SERVICES=$(subst \\,\,$(OFFICE_PROGRAM_PATH)$(PS)services.rdb)
endif

OFFICE_TYPE_LIBRARY="$(URE_TYPES)"
# DKREGISTRYNAME is only for compatibility reasons 
DKREGISTRYNAME=$(OFFICE_TYPE_LIBRARY)

DEPLOYTOOL="$(OFFICE_PROGRAM_PATH)$(PS)unopkg" add -f
SDK_JAVA="$(OO_SDK_JAVA_HOME)/bin/java"
SDK_JAVAC="$(OO_SDK_JAVA_HOME)/bin/javac"
SDK_JAR="$(OO_SDK_JAVA_HOME)/bin/jar"
SDK_ZIP="$(OO_SDK_ZIP_HOME)/zip"
IDLC="$(OO_SDK_HOME)/$(PLATFORM)/bin/idlc"
CPPUMAKER="$(OO_SDK_HOME)/$(PLATFORM)/bin/cppumaker"
JAVAMAKER="$(OO_SDK_HOME)/$(PLATFORM)/bin/javamaker"
REGMERGE="$(OO_SDK_HOME)/$(PLATFORM)/bin/regmerge"
REGCOMP="$(OO_SDK_HOME)/$(PLATFORM)/bin/regcomp"
XML2CMP="$(OO_SDK_HOME)/$(PLATFORM)/bin/xml2cmp"

SDK_JAVA_UNO_BOOTSTRAP_FILES=\
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKey.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKeyException.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)win/unowinreg.dll$(SQM)
