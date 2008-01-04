#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:30:09 $
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
