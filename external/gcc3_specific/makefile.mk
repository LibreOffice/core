PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------

.IF "$(SYSTEM_STDLIBS)" != "YES" && "$(COMID)"=="gcc3"

.IF "$(OS)"!="MACOSX" && "$(OS)"!="IOS" && "$(OS)"!="ANDROID" && "$(OS)"!="FREEBSD" && "$(OS)"!="NETBSD" && "$(OS)"!="OPENBSD" && "$(OS)"!="DRAGONFLY"

.EXPORT : CC

.IF "$(OS)"=="AIX"

all .SEQUENTIAL : $(LB)$/libstdc++.a $(LB)$/libgcc_s.a

$(LB)$/libstdc++.a :
    $(GCCINSTLIB) libstdc++.a $(LB)

$(LB)$/libgcc_s.a :
    $(GCCINSTLIB) libgcc_s.a $(LB)

.ELSE

all .SEQUENTIAL : $(LB)$/libstdc++.so.$(SHORTSTDCPP3) $(LB)$/libgcc_s.so.$(SHORTSTDC3)


$(LB)$/libstdc++.so.$(SHORTSTDCPP3) :
    $(GCCINSTLIB) libstdc++.so.$(SHORTSTDCPP3) $(LB)

$(LB)$/libgcc_s.so.$(SHORTSTDC3) :
    $(GCCINSTLIB) libgcc_s.so.$(SHORTSTDC3) $(LB)

.ENDIF

.ENDIF 

.ENDIF

.INCLUDE: target.mk

