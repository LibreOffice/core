PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------


.IF "$(COMID)"=="gcc3"

.IF "$(OS)"!="MACOSX"


all : $(SOLARLIBDIR)$/libstdc++.so.$(LIBSTDCPP3) $(SOLARLIBDIR)$/libgcc_s.so.1 $(SOLARLIBDIR)$/libstdc++.so.$(SHORTSTDCPP3) $(SOLARLIBDIR)$/libgcc_s.so 


$(SOLARLIBDIR)$/libstdc++.so.$(LIBSTDCPP3) :
       $(GCCINSTLIB) libstdc++.so.$(LIBSTDCPP3) $(SOLARLIBDIR)

$(SOLARLIBDIR)$/libgcc_s.so.1 :
       $(GCCINSTLIB) libgcc_s.so.1 $(SOLARLIBDIR)

$(SOLARLIBDIR)$/libstdc++.so.$(SHORTSTDCPP3) : $(SOLARLIBDIR)$/libstdc++.so.$(LIBSTDCPP3)
       -rm -f $@
       +ln -s libstdc++.so.$(LIBSTDCPP3) $@

$(SOLARLIBDIR)$/libgcc_s.so : $(SOLARLIBDIR)$/libgcc_s.so.1
        -rm -f $@
        +ln -s libgcc_s.so.1 $@

.ENDIF 

.ENDIF


.INCLUDE: target.mk

