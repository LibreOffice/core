# copied from settings.mk
SOLARBINDIR=$(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)

# Please modify the following lines to match your environment:
#   If you use the run: target at the end of the file, then adapt port number.
PORT_NUMBER = 5678

# The following variables probably don't need to be changed.
JAVAC = javac
JAVA = java
#   The JAR_PATH points to the jar files of your local office installation.
JAR_PATH = $(SOLARBINDIR)$/


# The rest of this makefile should not need to be touched.

all : tools

JAR_FILES =		\
    unoil.jar	\
    ridl.jar	\
    jurt.jar	\
    juh.jar		\
    java_uno.jar

JAVA_FILES = 					\
    tools/NameProvider.java


JAVA_CLASSPATHS := 			\
    .	..					\
    $(foreach,i,$(JAR_FILES) $(JAR_PATH)$i) \
    $(CLASSPATH)

CLASSPATH !:=$(JAVA_CLASSPATHS:t$(PATH_SEPERATOR))

JFLAGS = -deprecation -classpath $(CLASSPATH)

%.class : %.java
    +$(JAVAC) $(JFLAGS) $<

tools : $(JAVA_FILES:b:+".class")
