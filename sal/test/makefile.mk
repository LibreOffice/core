#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kr $ $Date: 2001-08-30 11:51:36 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=sal
TARGET=saltest
TARGETTYPE=CUI
LIBTARGET=NO
USE_LDUMP2=TRUE
#LDUMP2=LDUMP3


# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
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

APP1OBJS=	\
                $(OBJ)$/testprofile.obj		\
                $(OBJ)$/teststring.obj		\
                $(OBJ)$/testuuid.obj		\
                $(OBJ)$/teststrbuf.obj		\
                $(OBJ)$/testbyteseq.obj		\
                $(OBJ)$/testuri.obj			\
                $(OBJ)$/test.obj			\
                $(OBJ)$/testlogfile.obj
OBJFILES= \
    $(APP1OBJS) \
    $(APP2OBJS) 

APP1TARGET=	test
APP1STDLIBS=$(SALLIB) $(TSLLIB)

APP2TARGET = testbootstrap
APP2STDLIBS = $(SALLIB)


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
      $(BIN)$/inirc

$(BIN)$/$(APP2TARGET).exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/inirc:
        echo "MYBOOTSTRAPTESTVALUE=auxaux" > $@

.ELSE

ALL:  $(BIN)$/ini.ini

$(BIN)$/ini.ini:
        echo MYBOOTSTRAPTESTVALUE=auxaux > $@

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
$(MISC)$/tsl$(UPD)$(DLLPOSTFIX).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
