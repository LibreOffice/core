#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rene $ $Date: 2006-12-06 14:22:38 $
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

PRJ=.

PRJNAME=afms
TARGET=afms

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=Adobe-Core35_AFMs-314
TARFILE_ROOTDIR=Adobe-Core35_AFMs-314

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk

ZIP1DIR         = $(MISC)/build/$(TARFILE_NAME)
ZIP1TARGET      = fontunxafm
ZIP1LIST        = *.afm

.INCLUDE : target.mk

.INCLUDE : tg_ext.mk

$(ZIP1TARGETN):	$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)

