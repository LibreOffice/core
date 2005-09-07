#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.34 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:25:32 $
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
PRJ=..$/..

PRJNAME=sfx2
TARGET=dialog

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------------

EXCEPTIONSFILES=\
        $(SLO)$/filtergrouping.obj	\
        $(SLO)$/filedlghelper.obj	\
        $(SLO)$/mailmodel.obj		\
        $(SLO)$/mailmodelapi.obj	\
        $(SLO)$/cfg.obj				\
        $(SLO)$/partwnd.obj         \
        $(SLO)$/templdlg.obj        \
        $(SLO)$/itemconnect.obj     \
        $(SLO)$/acccfg.obj			\
        $(SLO)$/recfloat.obj

SLOFILES =\
        $(EXCEPTIONSFILES)			\
        $(SLO)$/macropg.obj         \
        $(SLO)$/sfxdlg.obj          \
        $(SLO)$/about.obj           \
        $(SLO)$/alienwarn.obj       \
        $(SLO)$/basedlgs.obj		\
        $(SLO)$/dinfdlg.obj			\
        $(SLO)$/dinfedt.obj			\
        $(SLO)$/dockwin.obj			\
        $(SLO)$/intro.obj			\
        $(SLO)$/mailchildwin.obj	\
        $(SLO)$/mailwindow.obj		\
        $(SLO)$/mgetempl.obj		\
        $(SLO)$/navigat.obj			\
        $(SLO)$/newstyle.obj		\
        $(SLO)$/passwd.obj			\
        $(SLO)$/sfxurl.obj			\
        $(SLO)$/slotinfo.obj		\
        $(SLO)$/splitwin.obj		\
        $(SLO)$/styfitem.obj		\
        $(SLO)$/styledlg.obj		\
        $(SLO)$/tabdlg.obj			\
        $(SLO)$/tplcitem.obj		\
        $(SLO)$/tplpitem.obj		\
        $(SLO)$/versdlg.obj			\
        $(SLO)$/printopt.obj		\
        $(SLO)$/srchdlg.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
        recfloat.src		\
        alienwarn.src       \
        cfg.src				\
        dialog.src			\
        dinfdlg.src			\
        dinfedt.src			\
        dlgrun.src			\
        eventdlg.src		\
        filedlghelper.src	\
        macropg.src			\
        mail.src			\
        mailwindow.src		\
        mgetempl.src		\
        newstyle.src		\
        passwd.src			\
        templdlg.src		\
        versdlg.src			\
        printopt.src        \
        srchdlg.src

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
    +echo \#define DLL_NAME \"libcui$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    +echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(UPD)$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/sfxdlg.obj : $(INCCOM)$/cuilib.hxx

