
.IF "$(EXPORTS)"!=""
EXPORT_HEADER=export_mozilla_header
.ENDIF

.IF "$(XPIDLSRCS)"!=""
XPIDL_TYPELIB_MODULE=$(MISC)$/$(XPIDL_MODULE).xpt
XPIDLHDR:=$(XPIDLSRCS:s/.idl/.h/)
XPIDLXPT:=$(XPIDLSRCS:s/.idl/.xpt/)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLHDR) $(INCCOM)$/$i)
EXTRA_MOZ_TARGET+= $(foreach,i,$(XPIDLXPT) $(MISC)$/$i)

EXTRA_MOZ_TARGET+= $(XPIDL_TYPELIB_MODULE)
.ENDIF

ALLMOZ:=$(EXTRA_MOZ_TARGET) $(DO_XPIDL) $(GEN_PRCPUCFG) $(EXPORT_HEADER)

$(INCCOM)$/%.h : %.idl
    +$(COPY) $< $(INCCOM)
    +$(XPIDL) -m header -w -I $(SOLARIDLDIR)$/mozilla -I$(INCCOM) -I . -o $(INCCOM)$/$* $<

$(MISC)$/%.xpt : %.idl
    +$(XPIDL) -m typelib -w -I $(SOLARIDLDIR)$/mozilla -I$(INCCOM) -I . -o $(MISC)$/$* $<

.INCLUDE : target.mk

.IF "$(GUI)"=="WNT"
CPUCFG=_winnt.cfg
.ENDIF

.IF "$(OS)"=="SOLARIS"
CPUCFG=_solaris32.cfg
.ENDIF

.IF "$(OS)"=="LINUX"
CPUCFG=_linux.cfg
.ENDIF

.IF "$(GEN_PRCPUCFG)"!=""
$(GEN_PRCPUCFG): $(PRJ)$/pr$/include$/md$/$(CPUCFG) 
    @+$(COPY) $(PRJ)$/pr$/include$/md$/$(CPUCFG) $@
.ENDIF

.IF "$(EXPORTS)"!=""
$(EXPORT_HEADER):
    @+$(COPY) $(EXPORTS) $(INCCOM)
.ENDIF

.IF "$(XPIDLSRCS)"!=""
$(XPIDL_TYPELIB_MODULE): $(foreach,i,$(XPIDLXPT) $(MISC)$/$i)
    +$(XPIDL_LINK) $(MISC)$/$(XPIDL_MODULE).xpt $^
    @+-mkdir $(BIN)$/components
    @+$(COPY) $@ $(BIN)$/components
.ENDIF
