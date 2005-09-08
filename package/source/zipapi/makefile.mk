#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 16:16:13 $
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
PRJNAME=package
TARGET=zipapi
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk

# --- Files --------------------------------------------------------
#CFLAGS+=/Ob0 /Od
.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF
SLOFILES= \
        $(SLO)$/Adler32.obj			\
        $(SLO)$/CRC32.obj			\
        $(SLO)$/ByteChucker.obj		\
        $(SLO)$/ByteGrabber.obj		\
        $(SLO)$/Inflater.obj		\
        $(SLO)$/Deflater.obj		\
        $(SLO)$/ZipEnumeration.obj	\
        $(SLO)$/ZipFile.obj			\
        $(SLO)$/ZipOutputStream.obj	\
        $(SLO)$/XUnbufferedStream.obj

# --- UNO stuff ---------------------------------------------------

CPPUMAKERFLAGS=
UNOUCROUT=$(OUT)$/inc
INCPRE+=$(UNOUCROUT)

UNOTYPES=\
    com.sun.star.packages.zip.ZipConstants \
    com.sun.star.packages.zip.ZipException \
    com.sun.star.io.BufferSizeExceededException \
    com.sun.star.io.XOutputStream \
    com.sun.star.io.XInputStream \
    com.sun.star.io.NotConnectedException

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
