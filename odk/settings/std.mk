ifneq "$(OO_SDK_OUT)" ""
OUT=$(subst \,/,$(OO_SDK_OUT))/$(OS)example.out
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
BIN_DIR=$(PRJ)/bin
CLASSES_DIR=$(PRJ)/classes
URE_CLASSES_DIR=$(subst \,/,$(OO_SDK_URE_JAVA_DIR))
OFFICE_CLASSES_DIR=$(subst \,/,$(OFFICE_PROGRAM_PATH))/classes
COMP_PACKAGE_DIR=$(subst /,$(PS),$(OUT_BIN))

SDKTYPEFLAG=$(OUT_MISC)/oosdk_cpp_types.flag

URE_TYPES="$(subst \,/,$(URE_MISC)$(PS)types.rdb)"
URE_SERVICES=$(subst \\,\,$(URE_MISC)$(PS)services.rdb)

OFFICE_TYPES="$(subst \,/,$(OFFICE_PROGRAM_PATH)$(PS)types$(PS)offapi.rdb)"
OFFICE_SERVICES=$(subst \\,\,$(OFFICE_PROGRAM_PATH)$(PS)services$(PS)services.rdb)

OFFICE_TYPE_LIBRARY="$(OFFICE_TYPES)"

JAVA_OPTIONS=
ifneq "$(OO_SDK_JAVA_HOME)" ""
JAVA_BITS := $(shell $(OO_SDK_JAVA_HOME)/$(JAVABIN)/java -version 2>&1 | tail -1 | cut -d " " -f3)
ifeq "$(JAVA_BITS)" "64-Bit"
ifneq "$(PROCTYPE)" "x86_64"
JAVA_OPTIONS=-d32
endif
endif
endif

DEPLOYTOOL="$(OFFICE_PROGRAM_PATH)$(PS)unopkg" add -f
SDK_JAVA="$(OO_SDK_JAVA_HOME)/$(JAVABIN)/java" $(JAVA_OPTIONS)
SDK_JAVAC="$(OO_SDK_JAVA_HOME)/$(JAVABIN)/javac"
SDK_JAR="$(OO_SDK_JAVA_HOME)/$(JAVABIN)/jar"
SDK_ZIP="$(OO_SDK_ZIP_HOME)/zip"
IDLC="$(OO_SDK_HOME)/bin/idlc"
CPPUMAKER="$(OO_SDK_HOME)/bin/cppumaker"
JAVAMAKER="$(OO_SDK_HOME)/bin/javamaker"
REGMERGE="$(OO_SDK_URE_HOME)/bin/regmerge"
REGCOMP="$(OO_SDK_URE_HOME)/bin/regcomp"

SDK_JAVA_UNO_BOOTSTRAP_FILES=\
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader$$Drain.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader$$Drain.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKey.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKeyException.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)win/unowinreg.dll$(SQM)
