PRJ=..$/..
PRJNAME=odk
TARGET=gendocu

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

DOCU_INDEX_FILE = $(DESTDIRGENDOCU)$/module-ix.html

IDLFILES={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f -print))}

DIRLIST = \
    $(DESTDIRCOMMONDOCU) \
    $(DESTDIRGENDOCU) \


all: $(DOCU_INDEX_FILE)

$(DIRLIST) :
     -$(MKDIRHIER) 	$@
    
$(DOCU_INDEX_FILE) : idl_chapter_refs.txt $(IDLFILES) $(DIRLIST)
.IF "$(BUILD_SOSL)"==""
    +autodoc -html $(DESTDIRGENDOCU) -dvgroot ..$/..$/DevelopersGuide -name "StarOffice6.0 API" -lg \
    idl -dvgfile idl_chapter_refs.txt -t $(DESTDIRIDL)
.ELSE
    +autodoc -html $(DESTDIRGENDOCU) -dvgroot ..$/..$/DevelopersGuide -name "OpenOffice.org 1.0.2 API" -lg \
    idl -dvgfile idl_chapter_refs.txt -t $(DESTDIRIDL)
.ENDIF
