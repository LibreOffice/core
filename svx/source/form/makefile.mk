#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.33 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 17:39:57 $
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

PRJNAME=svx
TARGET=form

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=form
SRC1FILES= \
    fmexpl.src 		\
    filtnav.src		\
    fmstring.src 	\
    tbxform.src		\
    fmsearch.src	\
    formshell.src   \
    datanavi.src

SLOFILES = \
    $(SLO)$/formtoolbars.obj \
    $(SLO)$/fmdocumentclassification.obj \
    $(SLO)$/fmcontrolbordermanager.obj \
    $(SLO)$/fmcontrollayout.obj \
    $(SLO)$/fmtextcontroldialogs.obj \
    $(SLO)$/fmtextcontrolfeature.obj \
    $(SLO)$/fmtextcontrolshell.obj \
    $(SLO)$/dbcharsethelper.obj			\
    $(SLO)$/ParseContext.obj			\
    $(SLO)$/typeconversionclient.obj    \
    $(SLO)$/confirmdelete.obj      \
    $(SLO)$/dbtoolsclient.obj      \
    $(SLO)$/sqlparserclient.obj      \
    $(SLO)$/dataaccessdescriptor.obj      \
    $(SLO)$/fmpage.obj      \
    $(SLO)$/fmPropBrw.obj			\
    $(SLO)$/fmundo.obj  	\
    $(SLO)$/fmprop.obj      \
    $(SLO)$/fmmodel.obj		\
    $(SLO)$/navigatortree.obj      \
    $(SLO)$/navigatortreemodel.obj      \
    $(SLO)$/fmexpl.obj      \
    $(SLO)$/fmctrler.obj    \
    $(SLO)$/fmpgeimp.obj	\
    $(SLO)$/fmvwimp.obj     \
    $(SLO)$/fmdpage.obj		\
    $(SLO)$/fmitems.obj  	\
    $(SLO)$/tbxform.obj     \
    $(SLO)$/fmobj.obj      	\
    $(SLO)$/fmdmod.obj		\
    $(SLO)$/fmobjfac.obj	\
    $(SLO)$/fmsrccfg.obj	\
    $(SLO)$/fmservs.obj     \
    $(SLO)$/typemap.obj		\
    $(SLO)$/fmexch.obj      \
    $(SLO)$/tabwin.obj      \
    $(SLO)$/fmurl.obj		\
    $(SLO)$/filtnav.obj		\
    $(SLO)$/fmtools.obj     \
    $(SLO)$/fmshimp.obj     \
    $(SLO)$/fmshell.obj     \
    $(SLO)$/fmsrcimp.obj	\
    $(SLO)$/fmview.obj		\
    $(SLO)$/sdbdatacolumn.obj \
    $(SLO)$/formcontrolling.obj \
    $(SLO)$/fmdispatch.obj  \
    $(SLO)$/datanavi.obj \
    $(SLO)$/xfm_addcondition.obj \
    $(SLO)$/formpdfexport.obj \
    $(SLO)$/datalistener.obj \
    $(SLO)$/fmscriptingenv.obj \
    $(SLO)$/stringlistresource.obj \
    $(SLO)$/databaselocationinput.obj

.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES=$(SLO)$/fmprop.obj
.ENDIF

# --- Targets ----------------------------------

.INCLUDE :  target.mk

