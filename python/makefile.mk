#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: mh $ $Date: 2003-03-31 14:07:18 $
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
#   The Initial Developer of the Original Code is: Ralph Thomas
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Ralph Thomas, Joerg Budischewski
#
#*************************************************************************

PRJ=.

PRJNAME=so_python
TARGET=so_python

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :      pyversion.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=Python-$(PYVERSION)

CONFIGURE_DIR=

BUILD_DIR=

.IF "$(GUI)" == "UNX"
CONFIGURE_ACTION=./configure --prefix=../python-inst
BUILD_ACTION=make ; make install
PYTHONCORESHL=$(OUT)$/lib$/libpython.so.$(PYVERSION)
PYTHONCORELINK1=$(OUT)$/lib$/libpython.so.$(PYMAJOR)
PYTHONCORELINK2=$(OUT)$/lib$/libpython.so
.ELSE
BUILD_DIR=PCBuild
BUILD_ACTION=msdev pcbuild.dsw /MAKE 	\
    "python - Win32 Release" 	\
    "_sre - Win32 Release" 		\
    "_socket - Win32 Release"	\
    "_symtable - Win32 Release" 	\
    "mmap - Win32 Release" 		\
    "parser - Win32 Release" 	\
    "select - Win32 Release" 	\
    "unicodedata - Win32 Release" 	\
    "winreg - Win32 Release" 	\
    "winsound - Win32 Release" 
.ENDIF

PYVERSIONFILE=$(MISC)$/pyversion.mk

# --- Targets ------------------------------------------------------

.IF "$(GUI)" != "UNX"
PYCONFIG=$(MISC)$/build$/pyconfig.h
.ENDIF

ALL : ALLTAR $(PYCONFIG) $(PYTHONCORESHL) $(PYVERSIONFILE) $(PYTHONCORELINK1) $(PYTHONCORELINK2)

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

#.IF "$(GUI)" !="UNX"
$(MISC)$/build$/pyconfig.h :$(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h $(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE)
    -rm -f $@
    cat $(MISC)$/build$/$(TARFILE_NAME)$/PC$/pyconfig.h > $@
#.ENDIF

.IF "$(GUI)" == "UNX"
$(PYTHONCORESHL) : $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a makefile.mk $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    ld -shared -o $@ -lm -ldl -lutil -lc -lpthread --whole-archive $(MISC)$/build$/$(TARFILE_NAME)$/libpython$(PYMAJOR).$(PYMINOR).a -soname libpython.so.$(PYMAJOR)

$(PYTHONCORELINK1) : makefile.mk
    -rm -f $@
    cd $(OUT)$/lib && ln -s libpython.so.$(PYVERSION) libpython.so.$(PYMAJOR)

$(PYTHONCORELINK2) : makefile.mk
    -rm -f $@
    cd $(OUT)$/lib && ln -s libpython.so.$(PYVERSION) libpython.so
.ENDIF

$(PYVERSIONFILE) : pyversion.mk
    -rm -f $@
    cat $? > $@ 
