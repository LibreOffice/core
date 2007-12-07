#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-12-07 10:42:39 $
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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=vbaobj
ENABLE_EXCEPTIONS=TRUE

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
                $(SLO)$/vbaoleobject.obj \
                $(SLO)$/vbaoleobjects.obj \
                $(SLO)$/vbabutton.obj \
                $(SLO)$/vbalabel.obj \
                $(SLO)$/vbatextbox.obj \
                $(SLO)$/vbaradiobutton.obj \
                $(SLO)$/vbalistbox.obj \
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
        $(SLO)$/service.obj \
 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

