all:
	cd source/org/openoffice/accessibility ; $(MAKE) all

ROOT=source
SUBDIRS=source/org/openoffice/accessibility
include source/makefile.in

run: all
	$(JAVA) -classpath $(CLASSPATH) org.openoffice.accessibility.awb.AccessibilityWorkBench
