#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=cssio
PACKAGE=com$/sun$/star$/io

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    FilePermission.idl\
    BufferSizeExceededException.idl\
    MarkableInputStream.idl\
    MarkableOutputStream.idl\
    DataInputStream.idl\
    DataOutputStream.idl\
    DataTransferEvent.idl\
    IOException.idl\
    NotConnectedException.idl\
    AlreadyConnectedException.idl\
    ObjectInputStream.idl\
    ObjectOutputStream.idl\
    Pipe.idl\
    Pump.idl\
    SequenceInputStream.idl\
    SequenceOutputStream.idl\
    UnexpectedEOFException.idl\
    WrongFormatException.idl\
    XActiveDataControl.idl\
    XActiveDataSink.idl\
    XActiveDataSource.idl\
    XActiveDataStreamer.idl\
    XConnectable.idl\
    XDataExporter.idl\
    XDataImporter.idl\
    XDataInputStream.idl\
    XDataOutputStream.idl\
    XDataTransferEventListener.idl\
    XInputStream.idl\
    XStream.idl\
    XMarkableStream.idl\
    XObjectInputStream.idl\
    XObjectOutputStream.idl\
    XOutputStream.idl\
    XPersist.idl\
    XPersistObject.idl\
    XStreamListener.idl\
    XSeekable.idl\
    XSeekableInputStream.idl\
    XSequenceOutputStream.idl\
    TextInputStream.idl\
    TextOutputStream.idl\
    XTextInputStream.idl\
    XTextOutputStream.idl\
    XTruncate.idl\
    XXMLExtractor.idl\
    XInputStreamProvider.idl\
    UnknownHostException.idl\
    SocketException.idl\
    NoRouteToHostException.idl\
    ConnectException.idl \
    XAsyncOutputMonitor.idl\
    XTempFile.idl\
    TempFile.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
