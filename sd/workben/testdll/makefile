#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
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

!IF "$(OS)" == "WNT"

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
