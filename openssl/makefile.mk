#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: tkr $ $Date: 2007-10-17 16:03:02 $
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

PRJNAME=so_openssl
TARGET=so_openssl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_OPENSSL)" == "YES"
@all:
    @echo "Using system openssl...."
.ENDIF

.IF "$(DISABLE_OPENSSL)" == "TRUE"
@all:
    @echo "openssl disabled...."
.ENDIF

OPENSSL_NAME=openssl-0.9.8e

TARFILE_NAME=$(OPENSSL_NAME)

CONFIGURE_DIR=.
CONFIGURE_ACTION=config
CONFIGURE_FLAGS=shared -I$(SYSBASE)$/usr$/include -L$(SYSBASE)$/usr$/lib

BUILD_DIR=.
BUILD_ACTION=make

.IF "$(OS)" == "LINUX"

PATCH_FILE_NAME=openssllnx.patch

BUILD_ACTION=make 'SHARED_LDFLAGS=-Wl,--version-script=./lib$$(SHLIBDIRS)_OOo_0_9_8e.map'
.ENDIF

.IF "$(OS)" == "SOLARIS"
OPENSSL_NAME=openssl-0.9.8a
CONFIGURE_DIR=. 
CONFIGURE_ACTION=Configure solaris-sparcv9-gcc  
.IF "$(PROCTYPE)" == "x86"
CONFIGURE_ACTION=Configure solaris-x86-gcc  
.ENDIF
.IF "$(PROCTYPE)" == "x86_64"
CONFIGURE_ACTION=Configure solaris-x86_64-gcc  
.ENDIF

CONFIGURE_FLAGS=shared

.ENDIF



.IF "$(OS)" == "WNT"

PATCH_FILE_NAME=openssl.patch

CONFIGURE_DIR=. 
CONFIGURE_ACTION=$(PERL) configure
CONFIGURE_FLAGS=VC-WIN32

#.IMPORT .IGNORE : SOLARINC
#INCLUDE := $(SOLARINC:s/-I/;)
#.EXPORT : INCLUDE

BUILD_DIR=.
BUILD_ACTION=cmd /c "ms\do_ms $(PERL)" && nmake -f ms/ntdll.mak
.ENDIF

#set INCLUDE=D:\sol_temp\n\msvc7net3\PlatformSDK\include;D:\sol_temp\n\msvc7net3\include\ && set path=%path%;D:\sol_temp\r\btw\SRC680\perl\bin &&

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

