#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..

PRJNAME=automation
TARGET=testtool

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ------------------------------------------------------------

OBJFILES = \
    $(OBJ)$/cmdstrm.obj              \
    $(OBJ)$/cretstrm.obj             \
    $(OBJ)$/objtest.obj              \
    $(OBJ)$/tcommuni.obj             \
    $(OBJ)$/comm_bas.obj             \
    $(OBJ)$/httprequest.obj          \
    

EXCEPTIONSFILES= \
    $(OBJ)$/tcommuni.obj             \
    $(OBJ)$/cmdstrm.obj              \
    $(OBJ)$/objtest.obj

HXXFILES = $(INCCOM)$/keycodes.hxx			\
    $(INCCOM)$/classes.hxx			\
    $(INCCOM)$/r_cmds.hxx			\
    $(INCCOM)$/res_type.hxx			

.IF "$(GUI)" == "WNT" 
OBJFILES += $(OBJ)$/sysdir_win.obj
OBJFILES += $(OBJ)$/registry_win.obj
.ENDIF
# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk


$(OBJFILES) : $(HXXFILES)
$(HXXFILES) : $(MISC)$/xfilter.pl


$(INCCOM)$/keycodes.hxx : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/vcl$/keycodes.hxx \
                          $(MISC)$/xfilter.pl
    $(PERL) $(MISC)$/xfilter.pl $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)  vcl$/keycodes.hxx  $(INCCOM)$/keycodes  KEY_

$(INCCOM)$/classes.hxx :  ..$/inc$/rcontrol.hxx \
                          $(MISC)$/xfilter.pl
    $(PERL) $(MISC)$/xfilter.pl ..$/inc  rcontrol.hxx  $(INCCOM)$/classes  M_

$(INCCOM)$/r_cmds.hxx :   ..$/inc$/rcontrol.hxx \
                          $(MISC)$/xfilter.pl
    $(PERL) $(MISC)$/xfilter.pl ..$/inc  rcontrol.hxx  $(INCCOM)$/r_cmds  RC_

$(INCCOM)$/res_type.hxx : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/tools$/wintypes.hxx \
                          $(MISC)$/xfilter.pl
    $(PERL) $(MISC)$/xfilter.pl $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)  tools$/wintypes.hxx  $(INCCOM)$/res_type  WINDOW_



$(MISC)$/xfilter.pl : filter.pl
       tr  -d "\015" < filter.pl > $(MISC)$/xfilter.pl
       chmod 664 $(MISC)$/xfilter.pl



.IF "$(GUI)"=="UNX"
INIFILESUFFIX=rc
BRANDPATH=none
.ELIF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
INIFILESUFFIX=.ini
BRANDPATH=..
.END

$(BIN)$/testtool$(INIFILESUFFIX): testtool.ini
        $(SED) -e s/$(EMQ)!INIFILESUFFIX$(EMQ)!/$(INIFILESUFFIX)/ \
            -e s/$(EMQ)!BRANDPATH$(EMQ)!/$(BRANDPATH)/ < $< > $@

ALLTAR: \
        $(BIN)$/testtool$(INIFILESUFFIX)

