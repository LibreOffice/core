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

