
ALLMOZ:=$(PRCPUCFG)

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

$(PRCPUCFG): $(PRJ)$/pr$/include$/md$/$(CPUCFG) 
    @+$(COPY) $(PRJ)$/pr$/include$/md$/$(CPUCFG) $@
