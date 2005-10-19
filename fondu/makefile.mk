#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: ericb
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

PRJNAME=fondu
TARGET=fondu

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=fondu_src-051010

.IF "$(OS)"!="MACOSX"
dummy:
    @echo "Nothing to build for OS $(OS)"
.ENDIF # "$(OS)"!="MACOSX"


CONFIGURE_DIR=.

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure

CONFIGURE_FLAGS=


BUILD_DIR=$(CONFIGURE_DIR)

BUILD_ACTION=make


OUT2BIN=\
    $(BUILD_DIR)$/dfont2res \
    $(BUILD_DIR)$/fondu	\
    $(BUILD_DIR)$/frombin	\
    $(BUILD_DIR)$/setfondname	\
    $(BUILD_DIR)$/showfond	\
    $(BUILD_DIR)$/tobin	\
    $(BUILD_DIR)$/ufond


ADDITIONAL_FILES=

OUT2INC=

OUT2CLASS=



# --- Targets ------------------------------------------------------

# contains some nessecary variables.

.INCLUDE : set_ext.mk

.INCLUDE : target.mk

# contains the "magic" to create targets out of the set variables.

.INCLUDE : tg_ext.mk