PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk
.INCLUDE : pyversion.mk

PYDIRNAME=python-core-$(PYVERSION)
DESTROOT=$(BIN)$/python-core-$(PYVERSION)
PYTHONBINARY=$(DESTROOT)$/bin$/python$(EXECPOST)

FINDLIBFILES_TMP:=$(subst,/,$/ \
    $(shell +$(FIND) $(SOLARLIBDIR)$/python -type f| $(GREP) -v .pyc ))
FINDLIBFILES=$(subst,$(SOLARLIBDIR)$/python, $(FINDLIBFILES_TMP))

FILES=\
    $(PYTHONBINARY)	\
    $(foreach,i,$(FINDLIBFILES) $(DESTROOT)$/lib$(i)) 

target: \
    $(BIN)$/python-core-$(PYVERSION).zip \
    $(BIN)$/python.sh

$(BIN)$/python.sh : python.sh
    -rm -f $@
    cat $? > $@
.IF "$(GUI)" == "UNX"
    chmod +x $@
.ENDIF

$(BIN)$/python-core-$(PYVERSION).zip : $(FILES)
.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "MACOSX"
    cd $(BIN) && find . -name '*$(DLLPOST)' | xargs $(STRIP) 
.ENDIF
.ENDIF
    -rm -f $@
    +cd $(BIN) && zip -r $(PYDIRNAME).zip $(PYDIRNAME)

$(DESTROOT)$/lib$/% : $(SOLARLIBDIR)$/python$/%
    -+$(MKDIRHIER) $(@:d) 
    -rm -f $@
    cat $< > $@

$(DESTROOT)$/bin$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    -+$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@
.IF "$(GUI)"== "UNX"
    chmod +x $@
.ENDIF
