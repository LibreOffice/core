#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.40 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:28:38 $
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
        $(SLO)$/acccfg.obj			\
        $(SLO)$/cfg.obj				\
        $(SLO)$/filedlghelper.obj	\
        $(SLO)$/filtergrouping.obj	\
        $(SLO)$/itemconnect.obj     \
        $(SLO)$/mailmodel.obj		\
        $(SLO)$/partwnd.obj         \
        $(SLO)$/recfloat.obj        \
        $(SLO)$/templdlg.obj        \
        $(SLO)$/newerverwarn.obj

SLOFILES =\
        $(EXCEPTIONSFILES)			\
        $(SLO)$/about.obj           \
        $(SLO)$/alienwarn.obj       \
        $(SLO)$/basedlgs.obj		\
        $(SLO)$/dinfdlg.obj			\
        $(SLO)$/dinfedt.obj			\
        $(SLO)$/dockwin.obj			\
        $(SLO)$/intro.obj			\
        $(SLO)$/macropg.obj         \
        $(SLO)$/mailwindow.obj		\
        $(SLO)$/mgetempl.obj		\
        $(SLO)$/navigat.obj			\
        $(SLO)$/newstyle.obj		\
        $(SLO)$/passwd.obj			\
        $(SLO)$/printopt.obj		\
        $(SLO)$/sfxdlg.obj          \
        $(SLO)$/sfxurl.obj			\
        $(SLO)$/splitwin.obj		\
        $(SLO)$/srchdlg.obj         \
        $(SLO)$/styfitem.obj		\
        $(SLO)$/styledlg.obj		\
        $(SLO)$/tabdlg.obj			\
        $(SLO)$/tplcitem.obj		\
        $(SLO)$/tplpitem.obj		\
        $(SLO)$/versdlg.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
        recfloat.src		\
        alienwarn.src       \
        cfg.src				\
        dialog.src			\
        dinfdlg.src			\
        dinfedt.src			\
        filedlghelper.src	\
        macropg.src			\
        mailwindow.src		\
        mgetempl.src		\
        newstyle.src		\
        passwd.src			\
        templdlg.src		\
        versdlg.src			\
        printopt.src        \
        srchdlg.src         \
        newerverwarn.src

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
    echo \#define DLL_NAME \"libcui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/sfxdlg.obj : $(INCCOM)$/cuilib.hxx

$(SLO)$/about.obj : $(INCCOM)$/aboutbmpnames.hxx

.INCLUDE .IGNORE : $(MISC)$/about_bmp_names.mk

.IF "$(ABOUT_BITMAPS:f)"!="$(LASTTIME_ABOUT_BITMAPS)"
DO_PHONY=.PHONY
.ENDIF			# "$(ABOUT_BITMAPS:f)"!="$(LASTTIME_ABOUT_BITMAPS)"

$(INCCOM)$/aboutbmpnames.hxx $(DO_PHONY):
    echo const char ABOUT_BITMAP_STRINGLIST[]=$(EMQ)"$(ABOUT_BITMAPS:f:t",")$(EMQ)"$(EMQ); > $@
    echo LASTTIME_ABOUT_BITMAPS=$(ABOUT_BITMAPS:f) > $(MISC)$/about_bmp_names.mk
