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
PRJ=..$/..

PRJNAME=sfx2
TARGET=dialog

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------------

EXCEPTIONSFILES=\
        $(SLO)$/about.obj           \
        $(SLO)$/filedlghelper.obj	\
        $(SLO)$/filtergrouping.obj	\
        $(SLO)$/itemconnect.obj     \
        $(SLO)$/mailmodel.obj		\
        $(SLO)$/partwnd.obj         \
        $(SLO)$/recfloat.obj        \
        $(SLO)$/templdlg.obj        \
        $(SLO)$/dinfdlg.obj         \
        $(SLO)$/dockwin.obj         \
        $(SLO)$/taskpane.obj

SLOFILES =\
        $(EXCEPTIONSFILES)			\
        $(SLO)$/alienwarn.obj       \
        $(SLO)$/basedlgs.obj		\
        $(SLO)$/dinfedt.obj			\
        $(SLO)$/intro.obj			\
        $(SLO)$/mgetempl.obj		\
        $(SLO)$/navigat.obj			\
        $(SLO)$/newstyle.obj		\
        $(SLO)$/passwd.obj			\
        $(SLO)$/printopt.obj		\
        $(SLO)$/sfxdlg.obj          \
        $(SLO)$/splitwin.obj		\
        $(SLO)$/srchdlg.obj         \
        $(SLO)$/styfitem.obj		\
        $(SLO)$/styledlg.obj		\
        $(SLO)$/tabdlg.obj			\
        $(SLO)$/tplcitem.obj		\
        $(SLO)$/tplpitem.obj		\
        $(SLO)$/versdlg.obj         \
        $(SLO)$/securitypage.obj    \
        $(SLO)$/titledockwin.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
        recfloat.src		\
        alienwarn.src       \
        dialog.src			\
        dinfdlg.src			\
        dinfedt.src			\
        filedlghelper.src	\
        mailwindow.src		\
        mgetempl.src		\
        newstyle.src		\
        passwd.src			\
        templdlg.src		\
        versdlg.src			\
        printopt.src        \
        srchdlg.src         \
        securitypage.src    \
        titledockwin.src    \
        taskpane.src

.IF "$(BUILD_VER_STRING)"!=""
.IF "$(GUI)"=="UNX"
CFLAGS+=-DBUILD_VER_STRING='"$(BUILD_VER_STRING)"'
.ELSE # "$(GUI)"=="UNX"
CFLAGS+=-DBUILD_VER_STRING="$(subst,",\" "$(BUILD_VER_STRING)")"
.ENDIF # "$(GUI)"=="UNX"
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    @echo \#define DLL_NAME \"libcui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    @echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/sfxdlg.obj : $(INCCOM)$/cuilib.hxx
