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



PRJ = ..
PRJNAME = uui
TARGET = source
ENABLE_EXCEPTIONS = true

.INCLUDE: settings.mk

SLOFILES = \
    $(SLO)$/cookiedg.obj \
    $(SLO)$/iahndl.obj \
    $(SLO)$/iahndl-authentication.obj \
    $(SLO)$/iahndl-ssl.obj \
    $(SLO)$/iahndl-cookies.obj \
    $(SLO)$/iahndl-filter.obj \
    $(SLO)$/iahndl-locking.obj \
    $(SLO)$/iahndl-ioexceptions.obj \
    $(SLO)$/iahndl-errorhandler.obj \
    $(SLO)$/logindlg.obj \
    $(SLO)$/services.obj \
    $(SLO)$/masterpassworddlg.obj \
    $(SLO)$/masterpasscrtdlg.obj \
    $(SLO)$/openlocked.obj \
    $(SLO)$/passworddlg.obj \
    $(SLO)$/fltdlg.obj \
    $(SLO)$/interactionhandler.obj \
    $(SLO)$/requeststringresolver.obj \
    $(SLO)$/unknownauthdlg.obj \
    $(SLO)$/sslwarndlg.obj \
    $(SLO)$/secmacrowarnings.obj \
    $(SLO)$/filechanged.obj \
    $(SLO)$/alreadyopen.obj \
    $(SLO)$/lockfailed.obj \
    $(SLO)$/trylater.obj \
    $(SLO)$/newerverwarn.obj \
    $(SLO)$/nameclashdlg.obj \
    $(SLO)$/passwordcontainer.obj

SRS1NAME=$(TARGET)
SRC1FILES = \
    cookiedg.src \
    ids.src \
    logindlg.src \
    masterpassworddlg.src \
    masterpasscrtdlg.src \
    openlocked.src \
    passworddlg.src \
    passworderrs.src \
    fltdlg.src \
    unknownauthdlg.src\
    sslwarndlg.src\
    secmacrowarnings.src\
    filechanged.src\
    alreadyopen.src\
    lockfailed.src\
    trylater.src\
    nameclashdlg.src\
    newerverwarn.src

.INCLUDE: target.mk

