PRJ=../..
PRJNAME=toolkit
TARGET=test
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(ENABLE_LAYOUT)" == "TRUE"

CFLAGS += -I$(PRJ)/source/layout

# Allow zoom and wordcount to be built without depending on svx,sv,sfx2
CFLAGS += -I../$(PRJ)/svx/inc -I../$(PRJ)/svtools/inc -I../$(PRJ)/sfx2/inc

.INCLUDE : $(PRJ)$/util$/makefile.pmk

CFLAGS += -DENABLE_LAYOUT=1 -DTEST_LAYOUT=1

.IF "$(COMNAME)" == "gcc3"
CFLAGS+=-Wall -Wno-non-virtual-dtor
.ENDIF

CXXFILES=\
    editor.cxx \
    wordcountdialog.cxx \
    test.cxx \
    zoom.cxx

OBJFILES=\
    $(OBJ)$/editor.obj \
    $(OBJ)$/test.obj \
    $(OBJ)$/wordcountdialog.obj \
    $(OBJ)$/zoom.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1STDLIBS= \
        $(TOOLSLIB)			\
        $(COMPHELPERLIB)		\
        $(VCLLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)		\
        $(SALLIB)			\
        $(XMLSCRIPTLIB)			\
        $(LAYOUTLIB)\
        $(TKLIB)

all: svtools ALLTAR

.INCLUDE :  target.mk

# FIXME: move this rule and .xml files into sw/svx trees,
#        install into xml directory instead of lib.
ALLTAR: \
    $(DLLDEST)$/wordcount.xml\
    $(DLLDEST)$/zoom.xml\
    $(DLLDEST)$/de/wordcount.xml\
    $(DLLDEST)$/de/zoom.xml\
    $(DLLDEST)$/nl/wordcount.xml\
    $(DLLDEST)$/nl/zoom.xml

TRALAY=tralay
$(WITH_LANG:f:t"/%.xml ")/%.xml: %.xml
    $(TRALAY) -m localize.sdf -o . -l $(WITH_LANG:f:t" -l ") $<
    rm -rf en-US

$(DLLDEST)$/%.xml: %.xml
# modes, INSTALL?
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

svtools:
    # FIXME: there's a bug in svtools layout or usage
    # Include files are in svtools/inc, but are referenced as <svtools/..>
    # They probably should be in svtools/inc/svtools
    # This means that include files can only be included after svtools
    # is built, which would mean a circular dependency,
    # because svtools depends on toolkit.
    ln -sf ../$(PRJ)/svtools/inc svtools

dist .PHONY :
    $(SHELL) ./un-test.sh zoom.cxx > ../$(PRJ)/svx/source/dialog/zoom.cxx
    $(SHELL) ./un-test.sh zoom.hxx > ../$(PRJ)/svx/source/dialog/zoom.hxx
    touch ../$(PRJ)/svx/source/dialog/dlgfact.cxx
    $(SHELL) ./un-test.sh wordcountdialog.cxx > ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.cxx
    $(SHELL) ./un-test.sh wordcountdialog.hxx > ../$(PRJ)/sw/source/ui/inc/wordcountdialog.hxx
    touch ../$(PRJ)/sw/source/ui/dialog/swdlgfact.cxx
    # FIXME: broken setup
    ln -sf ../inc/wordcountdialog.hxx ../$(PRJ)/sw/source/ui/dialog/wordcountdialog.hxx 

.ELSE # ENABLE_LAYOUT != TRUE
all .PHONY:
.ENDIF # ENABLE_LAYOUT != TRUE
