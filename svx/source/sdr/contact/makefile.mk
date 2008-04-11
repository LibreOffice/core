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
# $Revision: 1.13 $
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
