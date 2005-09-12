PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------


.IF "$(SYSTEM_STDLIBS)" != "YES" && "$(COMID)"=="gcc3"

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="IRIX"
.IF "$(OS)"!="FREEBSD"


all .SEQUENTIAL : $(SOLARLIBDIR)$/libstdc++.so.$(SHORTSTDCPP3) $(SOLARLIBDIR)$/libgcc_s.so.1 $(SOLARLIBDIR)$/libgcc_s.so 


$(SOLARLIBDIR)$/libstdc++.so.$(SHORTSTDCPP3) :
       $(GCCINSTLIB) libstdc++.so.$(SHORTSTDCPP3) $(SOLARLIBDIR)

$(SOLARLIBDIR)$/libgcc_s.so.1 :
       $(GCCINSTLIB) libgcc_s.so.1 $(SOLARLIBDIR)

$(SOLARLIBDIR)$/libgcc_s.so : $(SOLARLIBDIR)$/libgcc_s.so.1
        -rm -f $@
        +ln -s libgcc_s.so.1 $@

.ENDIF
.ENDIF 
.ENDIF 

.ENDIF


.INCLUDE: target.mk

