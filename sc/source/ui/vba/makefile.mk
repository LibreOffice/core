#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=vbaobj
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

INCPRE=$(INCCOM)$/$(TARGET)

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/vbaglobals.obj \
        $(SLO)$/vbaworkbook.obj \
        $(SLO)$/vbaworksheets.obj \
        $(SLO)$/vbaapplication.obj \
        $(SLO)$/vbarange.obj \
        $(SLO)$/vbaname.obj \
        $(SLO)$/vbanames.obj \
        $(SLO)$/vbacomment.obj \
        $(SLO)$/vbacomments.obj \
        $(SLO)$/vbaworkbooks.obj \
        $(SLO)$/vbaworksheet.obj \
        $(SLO)$/vbaoutline.obj \
        $(SLO)$/vbafont.obj\
        $(SLO)$/vbahelper.obj\
        $(SLO)$/vbainterior.obj\
        $(SLO)$/vbawsfunction.obj\
        $(SLO)$/vbawindow.obj\
        $(SLO)$/vbachart.obj\
        $(SLO)$/vbachartobject.obj\
        $(SLO)$/vbachartobjects.obj\
        $(SLO)$/vbaseriescollection.obj\
        $(SLO)$/vbadialogs.obj \
        $(SLO)$/vbadialog.obj	\
        $(SLO)$/vbapivottable.obj \
        $(SLO)$/vbapivotcache.obj \
        $(SLO)$/vbapivottables.obj \
        $(SLO)$/vbawindows.obj \
        $(SLO)$/vbapalette.obj \
        $(SLO)$/vbaborders.obj \
        $(SLO)$/vbacharacters.obj \
        $(SLO)$/vbacombobox.obj \
        $(SLO)$/vbavalidation.obj \
                $(SLO)$/vbacontrol.obj \
                $(SLO)$/vbacontrols.obj \
                $(SLO)$/vbaoleobject.obj \
                $(SLO)$/vbaoleobjects.obj \
                $(SLO)$/vbabutton.obj \
                $(SLO)$/vbalabel.obj \
                $(SLO)$/vbatextbox.obj \
                $(SLO)$/vbatextboxshape.obj \
                $(SLO)$/vbaradiobutton.obj \
                $(SLO)$/vbalistbox.obj \
        $(SLO)$/vbalistcontrolhelper.obj \
                $(SLO)$/vbapropvalue.obj \
                $(SLO)$/vbapane.obj \
                $(SLO)$/vbashape.obj \
                $(SLO)$/vbacolorformat.obj \
                $(SLO)$/vbashapes.obj \
                $(SLO)$/vbalineformat.obj \
                $(SLO)$/vbafillformat.obj \
                $(SLO)$/vbapictureformat.obj \
                $(SLO)$/vbashaperange.obj \
                $(SLO)$/vbatextframe.obj \
                $(SLO)$/vbacharttitle.obj \
                $(SLO)$/vbacharts.obj \
                $(SLO)$/vbaaxistitle.obj \
                $(SLO)$/vbaaxes.obj \
                $(SLO)$/vbaaxis.obj \
                $(SLO)$/vbaformat.obj \
                $(SLO)$/vbacondition.obj \
                $(SLO)$/vbaformatcondition.obj \
                $(SLO)$/vbaformatconditions.obj \
                $(SLO)$/vbastyle.obj \
                $(SLO)$/vbastyles.obj \
                $(SLO)$/vbaassistant.obj \
                $(SLO)$/vbauserform.obj \
                $(SLO)$/vbacheckbox.obj \
                $(SLO)$/vbatogglebutton.obj \
                $(SLO)$/vbaframe.obj \
                $(SLO)$/vbascrollbar.obj \
                $(SLO)$/vbaprogressbar.obj \
                $(SLO)$/vbamultipage.obj \
                $(SLO)$/vbapages.obj \
                $(SLO)$/vbacommandbarcontrol.obj \
                $(SLO)$/vbacommandbarcontrols.obj \
                $(SLO)$/vbacommandbar.obj \
                $(SLO)$/vbacommandbars.obj \
                $(SLO)$/vbahyperlink.obj \
                $(SLO)$/vbapagesetup.obj \
                $(SLO)$/vbapagebreak.obj \
                $(SLO)$/vbapagebreaks.obj \
                $(SLO)$/vbaspinbutton.obj \
                $(SLO)$/vbaimage.obj \
                $(SLO)$/service.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

