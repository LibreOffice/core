#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
