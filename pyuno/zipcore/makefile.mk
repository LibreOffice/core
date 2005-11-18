PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk

.IF "$(SYSTEM_PYTHON)" == "YES"
systempython:
        @echo "Not building python-core because system python is being used"
.ELSE

.INCLUDE : pyversion.mk

PYDIRNAME=python-core-$(PYVERSION)
DESTROOT=$(BIN)$/python-core-$(PYVERSION)
.IF "$(GUI)" == "UNX"
PYTHONBINARY=$(BIN)$/python$(EXECPOST).bin
.ELSE
PYTHONBINARY=$(DESTROOT)$/bin$/python$(EXECPOST)
.ENDIF

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
    sed 's/%%PYVERSION%%/$(PYVERSION)/g' < $@ > $@.new
    mv $@.new $@
.IF "$(GUI)" == "UNX"
    chmod +x $@
.ENDIF

$(BIN)$/python-core-$(PYVERSION).zip : $(FILES)
.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "MACOSX"
    cd $(BIN) && find . -name '*$(DLLPOST)' | xargs strip
.ENDIF
.ENDIF
    -rm -f $@
    +cd $(BIN) && zip -r $(PYDIRNAME).zip $(PYDIRNAME)

$(DESTROOT)$/lib$/% : $(SOLARLIBDIR)$/python$/%
    -+$(MKDIRHIER) $(@:d) 
    -rm -f $@
    cat $< > $@

.IF "$(GUI)"== "UNX"
$(BIN)$/python$(EXECPOST).bin : $(SOLARBINDIR)$/python$(EXECPOST)
    -+$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@
.IF "$(OS)" != "MACOSX"
    strip $@
.ENDIF
    chmod +x $@
.ELSE
$(DESTROOT)$/bin$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    -+$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@
.ENDIF

.ENDIF
