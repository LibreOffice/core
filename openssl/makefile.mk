#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 17:00:54 $
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

PRJNAME=openssl
TARGET=openssl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

# workaround a strange behavior of ld.so (2.1.3)
.IF "$(OS)$(CPU)"=="LINUXI"
LD_LIBRARY_PATH!:=$(subst,../lib, $(LD_LIBRARY_PATH))
.EXPORT : LD_LIBRARY_PATH
.ENDIF

TARFILE_NAME=openssl-0.9.5a

PATCH_FILE_NAME=openssl-0.9.5a.patch

.IF "$(GUI)"=="UNX"
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=config
CONFIGURE_FLAGS=-DPIC

.IF "$(COM)"=="GCC"
CONFIGURE_FLAGS+=-fpic
.ELSE
CONFIGURE_FLAGS+=-KPIC
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=make

OUT2LIB= \
    libcrypto.a \
    libssl.a
    
OUTDIR2INC= \
    include$/openssl$

.ENDIF			# "$(GUI)"=="UNX"

.IF "$(GUI)"=="WNT"

.EXPORT : PERL

CONFIGURE_ACTION=$(PERL) Configure VC-WIN32 ^ ms\do_ms.btm

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=*nmake -f ms/nt.mak

OUT2LIB= \
    out32$$/libeay32.lib \
    out32$$/ssleay32.lib
    
OUTDIR2INC= \
    inc32$/openssl

.ENDIF			# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(GUI)"=="WNT"
$(PACKAGE_DIR)$/so_custom_patch :  $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    +win32_custom.bat $(PACKAGE_DIR) $(TARFILE_NAME) ms && $(TOUCH) $@
    
$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_custom_patch
.ENDIF          # "$(GUI)"=="WNT"


