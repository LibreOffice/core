# This is the dmake version.

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

all : AccessibilityWorkBench

JAR_FILES =		\
    unoil.jar	\
    ridl.jar	\
    jurt.jar	\
    juh.jar		\
    java_uno.jar

JAVA_FILES = 								\
    AccTreeNode.java						\
    AccessibilityTree.java					\
    AccessibilityTreeModel.java 			\
    AccessibilityTreeModelBase.java 		\
    AccessibilityWorkBench.java				\
    AccessibleActionHandler.java			\
    AccessibleActionNode.java				\
    AccessibleCellHandler.java				\
    AccessibleComponentHandler.java			\
    AccessibleContextHandler.java			\
    AccessibleEditableTextHandler.java		\
    AccessibleExtendedComponentHandler.java	\
    AccessibleHyperlinkHandler.java			\
    AccessibleHypertextHandler.java			\
    AccessibleImageHandler.java				\
    AccessibleRelationHandler.java			\
    AccessibleSelectionHandler.java			\
    AccessibleTableHandler.java				\
    AccessibleTextHandler.java				\
    AccessibleTreeCellRenderer.java			\
    AccessibleTreeHandler.java				\
    AccessibleTreeNode.java					\
    AccessibleUNOHandler.java				\
    Canvas.java								\
    CanvasShape.java						\
    ChildEventHandler.java					\
    ContextEventHandler.java				\
    EventHandler.java						\
    EventListener.java						\
    EventLogger.java						\
    EventQueue.java							\
    FrameActionListener.java				\
    GeometryEventHandler.java				\
    HelpWindow.java							\
    InformationWriter.java					\
    MessageArea.java						\
    NodeFactory.java						\
    NodeHandler.java						\
    NodeMap.java							\
    OfficeConnection.java					\
    Options.java							\
    QueuedListener.java						\
    QueuedTopWindowListener.java			\
    SelectionDialog.java					\
    SimpleOffice.java						\
    StringNode.java							\
    TableEventHandler.java					\
    TextLogger.java							\
    TextUpdateListener.java					\
    TopWindowListener.java					\
    VectorNode.java

JAVA_CLASSPATHS := 			\
    .						\
    $(foreach,i,$(JAR_FILES) $(JAR_PATH)$i) \
    $(CLASSPATH)

CLASSPATH !:=$(JAVA_CLASSPATHS:t$(PATH_SEPERATOR))

JFLAGS = -deprecation -classpath $(CLASSPATH)

%.class : %.java
    +$(JAVAC) $(JFLAGS) $<

%.class : %.java
    +$(JAVAC) $(JFLAGS) $<

AccessibilityWorkBench : ObjectView Tools $(JAVA_FILES:b:+".class")

ObjectView .SETDIR=ov :
    @echo "making package ObjectView"
    dmake

Tools .SETDIR=tools :
    @echo "making package Tools"
    dmake

# Remove all class files.
clean : ObjectView.clean Tools.clean
    rm *.class
    rm AccessibilityWorkBench.jar
ObjectView.clean .SETDIR=ov :
    rm *.class
Tools.clean .SETDIR=tools :
    rm *.class

# Create a jar file of all files neccessary to build and run the work bench.
dist: AccessibilityWorkBench.jar

AccessibilityWorkBench.jar: $(JAVA_FILES:b:+".class") jawb.mf
    +jar -cfm AccessibilityWorkBench.jar jawb.mf *.class ov\*.class tools\*.class

# Example of how to run the work bench.
run: all
    +$(JAVA) -classpath $(CLASSPATH) AccessibilityWorkBench -p $(PORT_NUMBER)

runjar: all dist
    +$(JAVA) -classpath $(CLASSPATH) -jar AccessibilityWorkBench.jar -p $(PORT_NUMBER)

