#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:06:15 $
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

PRJNAME=cosv
TARGET=cosv_storage

ENABLE_EXCEPTIONS=true



# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.INCLUDE : $(PRJ)$/source$/fullcpp.mk




# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/dirchain.obj 	\
    $(OBJ)$/file.obj 		\
    $(OBJ)$/mbstream.obj    \
    $(OBJ)$/persist.obj	    \
    $(OBJ)$/ploc.obj 	    \
    $(OBJ)$/ploc_dir.obj    \
    $(OBJ)$/plocroot.obj

#SLOFILES= \
#	$(SLO)$/file.obj 		\
#	$(SLO)$/csfileim.obj 	\
#	$(SLO)$/memstorg.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



