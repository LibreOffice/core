PRJ=..

PRJNAME=test
TARGET=tst

# --- Settings -----------------------------------------------------

!INCLUDE <svpre.mak>
!INCLUDE <settings.mak>
!INCLUDE <sv.mak>

# --- Files --------------------------------------------------------

CXXFILES=	\
		test.cxx
SLOFILES=	\
		$(SLO)\test.obj
#SRC1FILES=
#SRS1NAME=
#SRSFILES=      $(SRS)\
#SRC2FILES=
#SRS2NAME=

#RESLIB1NAME=tst
#RESLIB1SRSFILES=\
#                $(SRS)\
#                $(SOLARRESDIR)\

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
#SHL1LIBS=$(SLB)\$(TARGET).lib

!IF "$(COM)"!="WTC"
SHL1STDLIBS=tools.lib $(SVLIB) usr.lib
!ELSE
SHL1STDLIBS=tools.lib libr sv.lib libr isv.lib libr usr.lib
!ENDIF

SHL1DEPN=   $(L)tools.lib $(SVLIBDEPEND)
SHL1DEF=    $(MISC)\$(SHL1TARGET).def
SHL1IMPLIB=$(TARGET)
#SHL1RES=    $(RES)\$(TARGET).res
SHL1OBJS=	$(SLO)\test.obj

# --- Targets ------------------------------------------------------
!INCLUDE <target.mak>

!IF "$(GUI)" == "WNT"

$(MISC)\$(SHL1TARGET).def:  makefile
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'StarOne Test-DLL'                                 >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
    @echo EXPORTS                                                   >>$@
	@echo CreateWindow @2 					>>$@
!ENDIF

!IF "$(GUI)" == "WIN"

$(MISC)\$(SHL1TARGET).def:  makefile
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'StarOne Test-DLL'                                 >>$@
    @echo EXETYPE     WINDOWS                                       >>$@
    @echo PROTMODE                                                  >>$@
    @echo CODE        LOADONCALL MOVEABLE DISCARDABLE               >>$@
    @echo DATA        PRELOAD MOVEABLE SINGLE                       >>$@
    @echo HEAPSIZE    0                                             >>$@
    @echo EXPORTS                                                   >>$@
	@echo _CreateWindow @2 								>>$@
!ENDIF

!IF "$(GUI)" == "OS2"

$(MISC)\$(SHL1TARGET).def:  makefile
	@echo ================================================================
	@echo building $@
	@echo ----------------------------------------------------------------
!IF "$(COM)"!="WTC"
	echo  LIBRARY		INITINSTANCE TERMINSTANCE			>$@
	echo  DESCRIPTION	'StarOne Test-DLL'           					>>$@
	echo  PROTMODE										   >>$@
    	@echo CODE        LOADONCALL 			              >>$@
	@echo DATA		  PRELOAD MULTIPLE NONSHARED					  >>$@
    	@echo EXPORTS                                                   >>$@
!IF "$(COM)"!="ICC"
	@echo _CreateWindow @2 								>>$@
!ELSE
	@echo CreateWindow @2 								>>$@
!ENDIF

!ELSE
    	@echo option DESCRIPTION 'StarOne Test-DLL'		                    >$@
    	@echo name $(BIN)\$(SHL1TARGET).dll                         >>$@
	@echo CreateWindow_ @2 								>>$@
	@gawk -f s:\util\exp.awk temp.def				>>$@
	del temp.def
!ENDIF
!ENDIF
