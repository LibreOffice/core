PRJ=..$/..

PRJNAME=testtools
TARGET=bridgetest
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------
.IF "$(GUI)"=="WNT"
MY_DLLPOSTFIX=.dll
MY_DLLPREFIX=
DESTDIR=$(BIN)
BATCH_SUFFIX=.bat
GIVE_EXEC_RIGHTS=@echo
WINTARGETS=$(DESTDIR)$/regcomp.exe $(DESTDIR)$/uno.exe
.ELSE
MY_DLLPOSTFIX=.so
MY_DLLPREFIX=lib
DESTDIR=$(OUT)$/lib
BATCH_INPROCESS=bridgetest_inprocess
GIVE_EXEC_RIGHTS=chmod +x 
.ENDIF

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(OUT)$/inc

UNOTYPES= \
        com.sun.star.uno.TypeClass		\
        com.sun.star.uno.XAggregation		\
        com.sun.star.uno.XWeak			\
        com.sun.star.lang.XTypeProvider		\
        com.sun.star.lang.XServiceInfo		\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XComponent		\
        com.sun.star.lang.XMain			\
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.container.XSet		\
        com.sun.star.test.bridge.XBridgeTest 	\
        com.sun.star.bridge.XUnoUrlResolver		\
        com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.uno.XComponentContext          

SLOFILES= \
        $(SLO)$/cppobj.obj	\
        $(SLO)$/bridgetest.obj

# ---- test ----

LIB1TARGET=$(SLB)$/cppobj.lib
LIB1OBJFILES= \
        $(SLO)$/cppobj.obj

SHL1TARGET=cppobj
SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1VERSIONMAP=component.map

# ---- test object ----

LIB2TARGET=$(SLB)$/bridgetest.lib
LIB2OBJFILES= \
        $(SLO)$/bridgetest.obj

SHL2TARGET=bridgetest
SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=component.map



.IF "$(SOLAR_JAVA)" != ""
JAVATARGETS=\
    $(DESTDIR)$/bridgetest_javaserver$(BATCH_SUFFIX) \
    $(DESTDIR)$/bridgetest_inprocess_java$(BATCH_SUFFIX)
.ENDIF

# --- Targets ------------------------------------------------------
ALL : \
        ALLTAR \
        $(DESTDIR)$/uno_types.rdb \
        $(DESTDIR)$/uno_services.rdb \
        $(DESTDIR)$/bridgetest_inprocess$(BATCH_SUFFIX) \
        $(DESTDIR)$/bridgetest_server$(BATCH_SUFFIX) \
        $(DESTDIR)$/bridgetest_client$(BATCH_SUFFIX) \
        $(JAVATARGETS) \
        $(WINTARGETS)
.INCLUDE :	target.mk

$(DESTDIR)$/uno_types.rdb : $(SOLARBINDIR)$/udkapi.rdb
    echo $(DESTDIR)
    $(GNUCOPY) -p $? $@

$(DESTDIR)$/bridgetest_inprocess$(BATCH_SUFFIX) : bridgetest_inprocess
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DESTDIR)$/bridgetest_client$(BATCH_SUFFIX) : bridgetest_client
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

$(DESTDIR)$/bridgetest_server$(BATCH_SUFFIX) : bridgetest_server
    $(GNUCOPY) -p $? $@
    $(GIVE_EXEC_RIGHTS) $@

.IF "$(SOLAR_JAVA)" != ""
# jar-files, which regcomp needs so that it can use java
MY_JARS=java_uno.jar ridl.jar sandbox.jar jurt.jar juh.jar

# CLASSPATH, which regcomp needs to be run
MY_CLASSPATH_TMP=$(foreach,i,$(MY_JARS) $(SOLARBINDIR)$/$i)$(PATH_SEPERATOR)$(XCLASSPATH)
MY_CLASSPATH=$(strip $(subst,!,$(PATH_SEPERATOR) $(MY_CLASSPATH_TMP:s/ /!/)))

$(DESTDIR)$/bridgetest_javaserver$(BATCH_SUFFIX) : makefile.mk
    -rm -f $@
    +echo java -classpath $(MY_CLASSPATH)$(PATH_SEPERATOR)..$/class$/testComponent.jar \
        com.sun.star.comp.bridge.TestComponentMain \
        \""uno:socket,host=localhost,port=2002;urp;test"\" \
        > $@
    $(GIVE_EXEC_RIGHTS) $@

$(DESTDIR)$/bridgetest_inprocess_java$(BATCH_SUFFIX) : makefile.mk
    -rm -f $@
.IF "$(GUI)"=="WNT"
    +echo set CLASSPATH=$(MY_CLASSPATH) >> $@
.ELSE
    +echo setenv CLASSPATH $(MY_CLASSPATH) >> $@
.ENDIF
    +echo uno -ro uno_services.rdb -ro uno_types.rdb \
       -s com.sun.star.test.bridge.BridgeTest -- com.sun.star.test.bridge.JavaTestObject >> $@
    $(GIVE_EXEC_RIGHTS) $@
.ENDIF

# I can't make a dependency on shared libraries, because dependent targets
# get the .setdir current directory. AAARGGGGGG !
$(DESTDIR)$/uno_services.rdb .SETDIR=$(DESTDIR) : $(WINTARGETS)
    regcomp -register -r uno_services.rdb \
        -c $(MY_DLLPREFIX)bridgetest$(MY_DLLPOSTFIX)	\
        -c $(MY_DLLPREFIX)cppobj$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)connectr$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)acceptor$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)brdgfctr$(MY_DLLPOSTFIX)		\
        -c $(MY_DLLPREFIX)remotebridge$(MY_DLLPOSTFIX)	\
        -c $(MY_DLLPREFIX)uuresolver$(MY_DLLPOSTFIX)
.IF "$(SOLAR_JAVA)" != ""
    regcomp -register -r uno_services.rdb \
        -c $(MY_DLLPREFIX)javaloader$(MY_DLLPOSTFIX)	\
        -c $(MY_DLLPREFIX)jen$(MY_DLLPOSTFIX)

# currently no chance to construct absolute file url for testComponent.jar
# 	regmerge regcomp.rdb / $(SOLARBINDIR)$/udkapi.rdb uno_services.rdb
# 	regcomp -register -r uno_services.rdb -br regcomp.rdb \
# 		-classpath $(MY_CLASSPATH)	\
# 		-c file:///c:/source/testtools/wntmsci9/class/testComponent.jar
.ENDIF


$(DESTDIR)$/regcomp.exe : $(SOLARBINDIR)$/regcomp.exe
    -rm -f $@
    cp $? $@

$(DESTDIR)$/uno.exe : $(SOLARBINDIR)$/uno.exe
    -rm -f $@
    cp $? $@
