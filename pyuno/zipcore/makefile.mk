PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk
.INCLUDE : pyversion.mk

PYDIRNAME=python-core-$(PYVERSION)
PACKSUFFIX=.zip
DESTROOT=$(BIN)$/python-core-$(PYVERSION)

.IF "$(GUI)"=="WNT"
INISUFFIX=.ini
BATCHSUFFIX=.bat
ENVSUFFIX=.bat
EXESUFFIX=.exe
DOLLAR_SIGN=$$
.ELSE
DOLLAR_SIGN=\$$
BATCHSUFFIX=.sh
ENVSUFFIX=.tcsh
INISUFFIX=rc
.ENDIF
PYTHONBINARY=$(BIN)$/$(PYDIRNAME)$/bin$/python$(EXESUFFIX)

FINDDIRS=$(subst,/,$/ $(shell +cd $(SOLARLIBDIR)$/python && $(FIND) . -type d))
FINDLIBFILES=$(subst,/,$/ $(shell +cd $(SOLARLIBDIR)$/python && $(FIND) . -type f))

PYRUNTIME_DIRS=\
    $(BIN)$/$(PYDIRNAME) 	\
    $(BIN)$/$(PYDIRNAME)$/bin 	\
    $(BIN)$/$(PYDIRNAME)$/lib 	\
    $(foreach,i,$(FINDDIRS) $(BIN)$/$(PYDIRNAME)$/lib$/$(i))


FILES=\
    $(PYTHONBINARY)	\
    $(foreach,i,$(FINDLIBFILES) $(BIN)$/$(PYDIRNAME)$/lib$/$(i)) 


target: \
    dirs \
    $(BIN)$/python-core-$(PYVERSION).zip \
    $(BIN)$/python.sh

$(BIN)$/python.sh : python.sh
    -rm -f $@
    cat $? > $@
    chmod +x $@

$(BIN)$/python-core-$(PYVERSION).zip : $(FILES)
    -+cd $(BIN) && find . -name '*.pyc' | xargs rm -f  
.IF "$(GUI)" == "UNX"
    cd $(BIN) && find . -name '*.so' | xargs strip 
.ENDIF
    -rm -f $@
    +cd $(BIN) && zip -r $(PYDIRNAME).zip $(PYDIRNAME)

dirs .PHONY:
    -mkdir $(PYRUNTIME_DIRS) 

$(BIN)$/$(PYDIRNAME)$/lib$/% : $(SOLARLIBDIR)$/python$/%
    -rm -f $@
    cat $? > $@

$(BIN)$/$(PYDIRNAME)$/bin$/python$(EXESUFFIX) : $(SOLARBINDIR)$/python$(EXESUFFIX)
    -rm -f $@
    cat $? > $@
.IF "$(GUI)"== "UNX"
    chmod +x $@
.ENDIF
