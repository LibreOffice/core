PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------

all: zipit 

zipit .SETDIR=$(DESTDIR)$/.. .PHONY:
    .IF "$(OS)"=="WNT"
        +zip -ur $(ODKNAME).zip $(ODKNAME)
    .ELIF "$(GUI)"=="UNX"
# 		tar does not properly support update
        +tar cf - $(ODKNAME) | gzip - > $(ODKNAME).tar.gz
    .ENDIF


