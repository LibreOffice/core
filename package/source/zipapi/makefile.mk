#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: mtg $ $Date: 2000-11-24 11:16:05 $
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

PRJ=..$/..
PRJNAME=package
TARGET=zipapi
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk

# --- Files --------------------------------------------------------

SLOFILES= \
        $(SLO)$/Adler32.obj	\
        $(SLO)$/CRC32.obj	\
        $(SLO)$/ByteChucker.obj	\
        $(SLO)$/ByteGrabber.obj	\
        $(SLO)$/Inflater.obj	\
        $(SLO)$/Deflater.obj	\
        $(SLO)$/ZipEnumeration.obj	\
        $(SLO)$/ZipFile.obj	\
        $(SLO)$/ZipOutputStream.obj	\
        $(SLO)$/EntryInputStream.obj

# --- UNO stuff ---------------------------------------------------

CPPUMAKERFLAGS=
#UNOUCROUT=$(OUT)$/inc
#INCPRE+=$(UNOUCROUT)

UNOUCRDEP=  $(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=  $(SOLARBINDIR)$/applicat.rdb

UNOTYPES=\
    com.sun.star.package.XChecksum \
    com.sun.star.package.XInflater \
    com.sun.star.package.XDeflater \
    com.sun.star.package.XZipFile \
    com.sun.star.package.XZipInputStream \
    com.sun.star.package.XZipOutputStream \
    com.sun.star.package.ZipConstants \
    com.sun.star.package.ZipEntry \
    com.sun.star.package.ZipException \
    com.sun.star.io.XSeekable \
    com.sun.star.io.XOutputStream \
    com.sun.star.lang.XInitialization \
    com.sun.star.container.XHierarchicalNameAccess \
    com.sun.star.lang.XSingleServiceFactory \
    com.sun.star.util.XChangesBatch \
    com.sun.star.container.XEnumeration \
    com.sun.star.container.XNamed \
    com.sun.star.container.XNameContainer \
    com.sun.star.container.XEnumerationAccess \
    com.sun.star.io.XActiveDataSink 

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
