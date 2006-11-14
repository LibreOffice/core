#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: ihi $ $Date: 2006-11-14 13:30:27 $
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
PRJINC=$(PRJ)$/source

PRJNAME=svx
TARGET=contact
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
        $(SLO)$/viewobjectcontactofunocontrol.obj

SLOFILES=\
        $(EXCEPTIONSFILES)                              \
        $(SLO)$/displayinfo.obj							\
        $(SLO)$/viewcontact.obj							\
        $(SLO)$/viewcontactpainthelper.obj				\
        $(SLO)$/viewcontactofsdrobj.obj					\
        $(SLO)$/viewcontactofvirtobj.obj				\
        $(SLO)$/viewcontactoftextobj.obj				\
        $(SLO)$/viewcontactofsdrpage.obj				\
        $(SLO)$/viewcontactofmasterpagedescriptor.obj	\
        $(SLO)$/viewcontactofgroup.obj					\
        $(SLO)$/viewcontactofe3dscene.obj				\
        $(SLO)$/viewcontactofpageobj.obj				\
        $(SLO)$/viewcontactofgraphic.obj				\
        $(SLO)$/objectcontact.obj						\
        $(SLO)$/objectcontactofobjlistpainter.obj		\
        $(SLO)$/objectcontactofpageview.obj				\
        $(SLO)$/viewobjectcontact.obj					\
        $(SLO)$/viewobjectcontactredirector.obj			\
        $(SLO)$/vocbitmapbuffer.obj						\
        $(SLO)$/vocofmasterpagedescriptor.obj			\
        $(SLO)$/viewobjectcontactlist.obj				\
        $(SLO)$/viewcontactofsdrobjcustomshape.obj      \
        $(SLO)$/viewcontactofsdrmediaobj.obj			\
        $(SLO)$/viewobjectcontactofsdrmediaobj.obj		\
        $(SLO)$/viewcontactofunocontrol.obj				\
        $(SLO)$/sdrmediawindow.obj					
        
.INCLUDE :  target.mk
