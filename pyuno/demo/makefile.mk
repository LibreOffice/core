PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk
.INCLUDE : pyversion.mk

VERSION=0.9.2
PYDIRNAME=python-$(PYVERSION)
.IF "$(GUI)"=="WNT"
INISUFFIX=.ini
BATCHSUFFIX=.bat
ENVSUFFIX=.bat
PLATFORM=win32
EXESUFFIX=.exe
PACKSUFFIX=.zip
MYCOPY=copy
DOLLAR_SIGN=$$
.ELSE
DOLLAR_SIGN=\$$
PACKSUFFIX=.tar.gz
MYCOPY=cp
BATCHSUFFIX=.sh
ENVSUFFIX=.tcsh
INISUFFIX=rc
PYUNOMODULE=$(DESTROOT)$/program$/pyuno$(DLLPOST)
PYTHONLIBRARY=$(DESTROOT)$/program$/$(DLLPRE)python$(DLLPOST).$(PYVERSION)
PYRUNTIMELINK=$(DESTROOT)$/program$/python
PYRUNTIMELIBLINK1=$(DESTROOT)$/program$/libpython.so.2
PYRUNTIMELIBLINK2=$(DESTROOT)$/program$/libpython.so

.IF "$(OS)$(CPU)"=="SOLARISS"
PLATFORM=solaris-sparc
.ELIF "$(OS)$(CPU)"=="SOLARISI"
PLATFORM=solaris-x86
.ELIF "$(OS)$(CPU)"=="LINUXI"
PLATFORM=linux-x86
.ELIF "$(OS)$(CPU)"=="LINUXP"
PLATFORM=linux-ppc
.ELSE
error please add your platform
.ENDIF

.ENDIF

DESTROOT=$(BIN)$/root

FINDDIRS=$(subst,/,$/ $(shell +cd $(SOLARLIBDIR)$/python && $(FIND) . -type d))
FINDLIBFILES=$(subst,/,$/ $(shell +cd $(SOLARLIBDIR)$/python && $(FIND) . -type f))

PYRUNTIME_DIRS=\
    $(DESTROOT)				\
    $(DESTROOT)$/program			\
    $(DESTROOT)$/program/pydemo		\
    $(DESTROOT)$/program$/$(PYDIRNAME) 	\
    $(DESTROOT)$/program$/$(PYDIRNAME)$/bin 	\
    $(DESTROOT)$/program$/$(PYDIRNAME)$/lib 	\
    $(foreach,i,$(FINDDIRS) $(DESTROOT)$/program$/$(PYDIRNAME)$/lib$/$(i))


FILES=\
    $(DESTROOT)$/program$/$(DLLPRE)pyuno$(DLLPOST) 		\
    $(DESTROOT)$/program$/$(DLLPRE)pycpld$(DLLPOST)	\
    $(DESTROOT)$/program$/pyuno$(INISUFFIX)			\
    $(DESTROOT)$/program$/uno.py				\
    $(DESTROOT)$/program$/unohelper.py			\
    $(DESTROOT)$/program$/pythonloader.py			\
    $(DESTROOT)$/program$/pyuno_setup$(BATCHSUFFIX)		\
    $(DESTROOT)$/program$/regcomp$(EXESUFFIX)		\
    $(DESTROOT)$/program$/pyunoenv$(ENVSUFFIX)		\
    $(DESTROOT)$/program$/pydemo$/biblioaccess.py		\
    $(DESTROOT)$/program$/pydemo$/ooextract.py		\
    $(DESTROOT)$/program$/pydemo$/swriter.py		\
    $(DESTROOT)$/program$/pydemo$/swritercomp.py		\
    $(DESTROOT)$/program$/pydemo$/swritercompclient.py	\
    $(DESTROOT)$/program$/pydemo$/swritercompclient.py	\
    $(DESTROOT)$/program$/pydemo$/python-bridge.html	\
    $(PYUNOMODULE)						\
    $(PYTHONLIBRARY)					\
    $(DESTROOT)$/program$/$(PYDIRNAME)$/bin$/python$(EXESUFFIX)	\
    $(foreach,i,$(FINDLIBFILES) $(DESTROOT)$/program$/$(PYDIRNAME)$/lib$/$(i)) \
    $(PYRUNTIMELINK)					\
    $(PYRUNTIMELIBLINK1)					\
    $(PYRUNTIMELIBLINK2)



$(BIN)$/pyuno-$(PLATFORM)-$(PYVERSION)$(PACKSUFFIX) : makefile.mk dirs $(FILES)
    -rm $@
.IF "$(GUI)"=="WNT"
    +cd $(DESTROOT) && zip -r ..$/pyuno-$(PLATFORM)-$(VERSION)$(PACKSUFFIX) program
.ELSE
    $(FIND) $(DESTROOT) -name '*.so' | xargs strip
    cd $(DESTROOT) && tar -cO program | gzip - > ..$/pyuno-$(PLATFORM)-$(VERSION)$(PACKSUFFIX)
.ENDIF


dirs .PHONY:
    -mkdir $(PYRUNTIME_DIRS) 

# Some symbolic links for unix  
.IF "$(GUI)" == "UNX"
$(PYRUNTIMELINK) : makefile.mk
    -rm -f $@
    cd $(DESTROOT)$/program && ln -s $(PYDIRNAME) python

$(PYRUNTIMELIBLINK1) : makefile.mk
    -rm -f $@
    cd $(DESTROOT)$/program && ln -s $(DLLPRE)python$(DLLPOST).$(PYVERSION) $(DLLPRE)python$(DLLPOST).$(PYMAJOR)

$(PYRUNTIMELIBLINK2) : makefile.mk
    -rm -f $@
    cd $(DESTROOT)$/program && ln -s $(DLLPRE)python$(DLLPOST).$(PYVERSION) $(DLLPRE)python$(DLLPOST)
.ENDIF

$(DESTROOT)$/program$/regcomp$(EXESUFFIX) : $(SOLARBINDIR)$/regcomp$(EXESUFFIX)
    cp $? $@
.IF "$(GUI)" == "UNX"
    strip $@
    chmod +x $@
.ENDIF


$(DESTROOT)$/program$/pyunoenv$(ENVSUFFIX) : pyunoenv$(ENVSUFFIX)
    -rm -f $@
    cat $? > $@

$(DESTROOT)$/program$/$(DLLPRE)pyuno$(DLLPOST) : $(DLLDEST)$/$(DLLPRE)pyuno$(DLLPOST)
    cp $? $@

$(DESTROOT)$/program$/pyuno_setup$(BATCHSUFFIX) : makefile.mk
    -rm -f $@
.IF "$(GUI)"!="WNT"
    echo #\!/bin/sh >> $@
    chmod +x $@
.ENDIF
    echo regcomp -register -r applicat.rdb -c pycpld >>$@
#	echo "$(MYCOPY) applicat.rdb pydemo$/applicat.rdb" >> $@
    echo regcomp -register -br applicat.rdb -r applicat.rdb -c vnd.openoffice.pymodule:swritercomp -l com.sun.star.loader.Python >>$@

$(DESTROOT)$/program$/$(DLLPRE)python$(DLLPOST).$(PYVERSION) : $(SOLARLIBDIR)$/$(DLLPRE)python$(DLLPOST).$(PYVERSION)
    cp $? $@

$(DESTROOT)$/program$/$(DLLPRE)pycpld$(DLLPOST) : $(DLLDEST)$/$(DLLPRE)pycpld$(DLLPOST)
    cp $? $@

$(DESTROOT)$/program$/%.py : $(DLLDEST)$/%.py
    cp $? $@

.IF "$(GUI)" == "UNX"
$(DESTROOT)$/program$/pyuno$(DLLPOST) : $(DLLDEST)$/pyuno$(DLLPOST)
    cp $? $@
.ENDIF

$(DESTROOT)$/program$/pydemo$/%.py : %.py
    -rm -f $@
    cat $? > $@

$(DESTROOT)$/program$/pyuno$(INISUFFIX) : makefile.mk
    -rm -f $@ $(DESTROOT)$/program$/pyuno.tmp
    echo UNO_TYPES=$(DOLLAR_SIGN)PYUNOLIBDIR/applicat.rdb > $(DESTROOT)$/program$/pyuno.tmp
    echo UNO_SERVICES=$(DOLLAR_SIGN)PYUNOLIBDIR/applicat.rdb >> $(DESTROOT)$/program$/pyuno.tmp
    mv $(DESTROOT)$/program$/pyuno.tmp $@

$(DESTROOT)$/program$/pydemo$/python-bridge.html : ..$/doc$/python-bridge.html
    -rm -f $@
    cat $? > $@


# $(DESTROOT)$/program$/$(PYDIRNAME)$/lib$/%.so : $(SOLARLIBDIR)$/python$/%.so
# 	-rm -f $@
# 	cat $? > $@
# 	strip $@

$(DESTROOT)$/program$/$(PYDIRNAME)$/lib$/% : $(SOLARLIBDIR)$/python$/%
    -rm -f $@
    cat $? > $@


$(DESTROOT)$/program$/$(PYDIRNAME)$/bin$/python$(EXESUFFIX) : $(SOLARBINDIR)$/python$(EXESUFFIX)
    -rm -f $@
    cat $? > $@
.IF "$(GUI)" == "UNX"
    strip $@
    chmod +x $@
.ENDIF
