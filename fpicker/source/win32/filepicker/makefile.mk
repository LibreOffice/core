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



PRJ=..$/..$/..

PRJNAME=fpicker
TARGET=fps
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

CDEFS+=-D_UNICODE
CDEFS+=-DUNICODE
#CDEFS+=-DWIN32_LEAN_AND_MEAN
#CDEFS+=-DWIN32
CDEFS+=-U_WIN32_WINNT -D_WIN32_WINNT=0x0600


# --- Resources ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=fps.rc
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/FileOpenDlg.obj\
         $(SLO)$/FPentry.obj\
         $(SLO)$/FilePicker.obj\
         $(SLO)$/WinFileOpenImpl.obj\
         $(SLO)$/FilterContainer.obj\
         $(SLO)$/controlaccess.obj\
         $(SLO)$/dibpreview.obj\
         $(SLO)$/helppopupwindow.obj\
         $(SLO)$/controlcommand.obj\
         $(SLO)$/filepickerstate.obj\
         $(SLO)$/getfilenamewrapper.obj\
         $(SLO)$/asynceventnotifier.obj\
         $(SLO)$/previewadapter.obj\
         $(SLO)$/previewbase.obj\
         $(SLO)$/filepickereventnotification.obj\
         $(SLO)$/customcontrol.obj\
         $(SLO)$/customcontrolcontainer.obj\
         $(SLO)$/customcontrolfactory.obj\
         $(SLO)$/dialogcustomcontrols.obj\
         $(SLO)$/asyncrequests.obj\
         $(SLO)$/VistaFilePickerEventHandler.obj\
         $(SLO)$/VistaFilePickerImpl.obj\
         $(SLO)$/VistaFilePicker.obj\
         $(SLO)$/SolarMutex.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
