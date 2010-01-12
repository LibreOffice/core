# copied from settings.mk
SOLARBINDIR=$(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)

# Please modify the following lines to match your environment:
#   If you use the run: target at the end of the file, then adapt pipe name
PIPE_NAME = $(USER)

# The following variables probably don't need to be changed.
JAVA = java
#   The JAR_PATH points to the jar files of your local office installation.
JAR_PATH = $(SOLARBINDIR)$/


# The rest of this makefile should not need to be touched.

JAR_FILES =		\
    unoil.jar	\
    ridl.jar	\
    jurt.jar	\
    juh.jar		\
    java_uno.jar


JAVA_CLASSPATHS := 			\
    ..$/$(INPATH)$/class						\
    $(foreach,i,$(JAR_FILES) $(JAR_PATH)$i) \
    $(CLASSPATH)

CLASSPATH !:=$(JAVA_CLASSPATHS:t$(PATH_SEPERATOR))

all:
    build

# Example of how to run the work bench.
run:
    +$(JAVA) -classpath "$(CLASSPATH)" org/openoffice/accessibility/awb/AccessibilityWorkBench -p $(PIPE_NAME)

runjar:
    +$(JAVA) -classpath "$(CLASSPATH)" -jar AccessibilityWorkBench.jar -p $(PIPE_NAME)
