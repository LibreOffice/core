OSOUT=$(OS)example.out
OUT=$(PRJ)/$(OSOUT)
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
COMPONENT_PACKAGE_DIR=$(OO_SDK_HOME)$(PS)$(OS)example.out$(PS)bin

DEPLOYTOOL="$(OFFICE_PROGRAM_PATH)$(PS)pkgchk" -f
SDK_JAVA="$(OO_SDK_JAVA_HOME)/bin/java"
SDK_JAVAC="$(OO_SDK_JAVA_HOME)/bin/javac"
SDK_JAR="$(OO_SDK_JAVA_HOME)/bin/jar"
IDLC="$(OO_SDK_HOME)/$(PLATFORM)/bin/idlc"
CPPUMAKER="$(OO_SDK_HOME)/$(PLATFORM)/bin/cppumaker"
JAVAMAKER="$(OO_SDK_HOME)/$(PLATFORM)/bin/javamaker"
REGMERGE="$(OO_SDK_HOME)/$(PLATFORM)/bin/regmerge"
XML2CMP="$(OO_SDK_HOME)/$(PLATFORM)/bin/xml2cmp"

SDK_JAVA_UNO_BOOTSTRAP_FILES=\
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/Loader$$CustomURLClassLoader.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/InstallationFinder$$StreamGobbler.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKey.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/WinRegKeyException.class$(SQM) \
    -C $(CLASSES_DIR) $(SQM)win/unowinreg.dll$(SQM)
