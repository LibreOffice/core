#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..

PRJNAME=sal
TARGET=saltest
TARGETTYPE=CUI
LIBTARGET=NO
USE_LDUMP2=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
BOOTSTRAPSCRIPT=bootstrap.bat
BOOTSTRAPINI=testbootstrap.ini
MY_SCRIPTCAT=cat
.ELSE
BOOTSTRAPSCRIPT=bootstrap
BOOTSTRAPINI=testbootstraprc
MY_SCRIPTCAT=tr -d "\015" <
.ENDIF

APP2OBJS = $(OBJ)$/testbootstrap.obj

OBJFILES= \
    $(APP2OBJS) 

APP2TARGET = testbootstrap
APP2STDLIBS = $(SALLIB)

APP3OBJS = $(OBJ)$/test_salmain.obj
APP3TARGET = test_salmain
APP3STDLIBS = $(SALLIB)

APP4OBJS = $(OBJ)$/test_salmainwithargs.obj
APP4TARGET = test_salmainwithargs
APP4STDLIBS = $(SALLIB)


# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : ALLTAR \
      $(BIN)$/$(BOOTSTRAPSCRIPT) \
      $(BIN)$/$(BOOTSTRAPINI)    \
      $(BIN)$/bootstraptest.ini  \
      $(BIN)$/$(APP2TARGET).bin  \
      $(BIN)$/$(APP2TARGET).Bin  \
      $(BIN)$/$(APP2TARGET).Exe  \
      $(BIN)$/bootstrap.pl  


.IF "$(GUI)"=="UNX"
ALL:  $(BIN)$/$(APP2TARGET).exe \
      $(BIN)$/inirc \
      $(BIN)$/defaultrc

$(BIN)$/$(APP2TARGET).exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/inirc:
        echo "CUSTOMINIVALUE=auxaux"   > $@
        echo "INHERITED_OVERWRITTEN_VALUE=inherited_overwritten_value" >> $@

$(BIN)$/defaultrc:
        echo "Default=defaultValue" > $@

.ELSE

ALL:  $(BIN)$/ini.ini		\
      $(BIN)$/default.ini

$(BIN)$/ini.ini:
        echo CUSTOMINIVALUE=auxaux     > $@
        echo INHERITED_OVERWRITTEN_VALUE=inherited_overwritten_value   >> $@


$(BIN)$/default.ini:
        echo Default=defaultValue > $@

.ENDIF

$(BIN)$/bootstrap.pl:
    cp bootstrap.pl $@

$(BIN)$/$(APP2TARGET).bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk


$(BIN)$/$(BOOTSTRAPSCRIPT) : $(BOOTSTRAPSCRIPT)
    $(MY_SCRIPTCAT) $(BOOTSTRAPSCRIPT) > $@
.IF "$(GUI)"!="WNT"
    chmod ug+x $@
.ENDIF

$(BIN)$/$(BOOTSTRAPINI) : $(APP2TARGET).ini
    $(MY_SCRIPTCAT) $(APP2TARGET).ini > $@	

$(BIN)$/bootstraptest.ini : bootstraptest.ini
    $(MY_SCRIPTCAT) bootstraptest.ini > $@	


# --- SO2-Filter-Datei ---

$(MISC)$/tsl$(DLLPOSTFIX).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
