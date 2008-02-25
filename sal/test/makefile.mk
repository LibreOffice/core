#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:45:09 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..

PRJNAME=sal
TARGET=saltest
TARGETTYPE=CUI
LIBTARGET=NO
USE_LDUMP2=TRUE
ENABLE_EXCEPTIONS=TRUE
#LDUMP2=LDUMP3


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
