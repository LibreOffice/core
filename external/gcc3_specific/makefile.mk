PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------


.IF "$(GUI)" == "WNT"

.IF "$(COM)" == "GCC"

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
MINGWGCCDLL=$(BIN)$/$(MINGW_GCCDLL)
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
MINGWGXXDLL=$(BIN)$/$(MINGW_GXXDLL)
.ENDIF

all : $(MINGWGCCDLL) $(MINGWGXXDLL)

# Guesstimate where $(MINGW_GCCDLL) and $(MINGW_GXXDLL) might be

POTENTIAL_MINGW_RUNTIME_BINDIRS = \
	$(COMPATH)/i686-w64-mingw32/sys-root/mingw/bin \
	/usr/i686-w64-mingw32/sys-root/mingw/bin \

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
$(MINGWGCCDLL) :
    @for D in $(POTENTIAL_MINGW_RUNTIME_BINDIRS); do \
        test -f $$D/$(MINGW_GCCDLL) && $(COPY) -p $$D/$(MINGW_GCCDLL) $(BIN)$/ && break; \
    done
    @test -f $@ || (echo Could not find $(MINGW_GCCDLL) && exit 1)
.ENDIF

$(MINGWGXXDLL) :
.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
    @for D in $(POTENTIAL_MINGW_RUNTIME_BINDIRS); do \
        test -f $$D/$(MINGW_GXXDLL) && $(COPY) -p $$D/$(MINGW_GXXDLL) $(BIN)$/ && break; \
    done
    @test -f $@ || (echo Could not find $(MINGW_GXXDLL) && exit 1)

.ENDIF

.ENDIF

.ELSE

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

.ENDIF

.INCLUDE: target.mk

