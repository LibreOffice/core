PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------


.IF "$(BUILD_SPECIAL)"==""

.IF "$(GUI)" == "WNT"

.IF "$(COM)" == "GCC"

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
MINGWGCCDLL=$(BIN)$/$(MINGW_GCCDLL)
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
MINGWGXXDLL=$(BIN)$/$(MINGW_GXXDLL)
.ENDIF

all : $(BIN)$/mingwm10.dll $(MINGWGCCDLL) $(MINGWGXXDLL)

$(BIN)$/mingwm10.dll :
    $(COPY) -p $(COMPATH)$/bin$/mingwm10.dll $(BIN)$/

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
$(MINGWGCCDLL) :
    $(COPY) -p $(COMPATH)$/bin$/$(MINGW_GCCDLL) $(BIN)$/
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
$(MINGWGXXDLL) :
    $(COPY) -p $(COMPATH)$/bin$/$(MINGW_GXXDLL) $(BIN)$/
.ENDIF

.ENDIF

.ELSE

.IF "$(SYSTEM_STDLIBS)" != "YES" && "$(COMID)"=="gcc3"

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
.IF "$(OS)"!="FREEBSD"

.EXPORT : CC

all .SEQUENTIAL : $(LB)$/libstdc++.so.$(SHORTSTDCPP3) $(LB)$/libgcc_s.so.1


$(LB)$/libstdc++.so.$(SHORTSTDCPP3) :
       $(GCCINSTLIB) libstdc++.so.$(SHORTSTDCPP3) $(LB)

$(LB)$/libgcc_s.so.1 :
       $(GCCINSTLIB) libgcc_s.so.1 $(LB)

.ENDIF
.ENDIF 
.ENDIF 

.ENDIF

.ENDIF
.ENDIF			# "$(BUILD_SPECIAL)"==""


.INCLUDE: target.mk

