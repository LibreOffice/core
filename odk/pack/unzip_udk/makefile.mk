#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:17:40 $
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
PRJNAME=odk
TARGET=unzip_udk

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

all: ..$/misc$/deltree.txt

..$/misc$/deltree.txt .SETDIR=$(OUT)$/bin : $(PRJ)$/pack$/unzip_udk$/deltree.txt
    +-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME) >& $(NULLDEV)
    +-rm -f $(PRODUCT_NAME).zip >& $(NULLDEV)
    +-rm -f $(PRODUCT_NAME).tar.gz >& $(NULLDEV)
    +-rm -f $(PRODUCT_NAME).tar >& $(NULLDEV)
    +-rm -f $(DEVGUIDE_DOCU_FLAG) >& $(NULLDEV)
    +-rm -f $(DEVGUIDE_SAMPLES_FLAG) >& $(NULLDEV)
    +@echo "" > ..$/misc$/deltree.txt
