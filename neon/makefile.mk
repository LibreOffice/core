#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: rt $ $Date: 2002-01-23 13:32:41 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=.

PRJNAME=so_neon
TARGET=so_neon

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

NEON_NAME=neon-0.14.0

TARFILE_NAME=$(NEON_NAME)
PATCH_FILE_NAME=$(NEON_NAME).patch

ADDITIONAL_FILES=src$/makefile.mk src$/config.h

BUILD_DIR=src
BUILD_ACTION=dmake $(PROFULLSWITCH)

OUT2INC= \
    $(BUILD_DIR)$/dates.h			\
    $(BUILD_DIR)$/dav_207.h         \
    $(BUILD_DIR)$/dav_basic.h       \
    $(BUILD_DIR)$/dav_locks.h       \
    $(BUILD_DIR)$/dav_props.h       \
    $(BUILD_DIR)$/hip_xml.h         \
    $(BUILD_DIR)$/http_auth.h       \
    $(BUILD_DIR)$/http_basic.h      \
    $(BUILD_DIR)$/http_cookies.h    \
    $(BUILD_DIR)$/http_redirect.h   \
    $(BUILD_DIR)$/http_request.h    \
    $(BUILD_DIR)$/http_utils.h      \
    $(BUILD_DIR)$/neon_defs.h       \
    $(BUILD_DIR)$/ne_alloc.h        \
    $(BUILD_DIR)$/nsocket.h         \
    $(BUILD_DIR)$/string_utils.h    \
    $(BUILD_DIR)$/uri.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

