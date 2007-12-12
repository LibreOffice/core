#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 15:00:46 $
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

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=ooo_boost
TARGET=ooo_boost

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# force patched boost for sunpro CC
# to workaround opt bug when compiling with -xO3
.IF "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
all:
    @echo "An already available installation of boost should exist on your system."        
    @echo "Therefore the version provided here does not need to be built in addition."
.ELSE			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")

# --- Files --------------------------------------------------------

TARFILE_NAME=boost-1.30.2
PATCH_FILE_NAME=$(TARFILE_NAME).patch

.IF "$(COMID)"=="gcc3"

# enabled for gcc4.x
.IF "$(CCNUMVER)">="000400000000" 
  TARFILE_NAME=boost_1_34_1
  PATCH_FILE_NAME=$(TARFILE_NAME).patch
.ENDIF

.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF			# "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
