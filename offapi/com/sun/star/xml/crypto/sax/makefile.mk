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



#i20156 - new file for xmlsecurity module

PRJ=..$/..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=xsec-sax
PACKAGE=com$/sun$/star$/xml$/crypto$/sax

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    XSecurityController.idl       \
    XElementStackKeeper.idl       \
    XSAXEventKeeper.idl        \
    XSecuritySAXEventKeeper.idl        \
    XSAXEventKeeperStatusChangeBroadcaster.idl        \
    XSAXEventKeeperStatusChangeListener.idl        \
    XKeyCollector.idl        \
    XReferenceCollector.idl        \
    XBlockerMonitor.idl        \
    XMissionTaker.idl        \
    XEncryptionResultBroadcaster.idl        \
    XEncryptionResultListener.idl        \
    XDecryptionResultBroadcaster.idl        \
    XDecryptionResultListener.idl        \
    XSignatureCreationResultBroadcaster.idl        \
    XSignatureCreationResultListener.idl        \
    XSignatureVerifyResultBroadcaster.idl        \
    XSignatureVerifyResultListener.idl        \
    XReferenceResolvedBroadcaster.idl        \
    XReferenceResolvedListener.idl        \
    SignatureCreator.idl        \
    SignatureVerifier.idl        \
    Decryptor.idl        \
    Encryptor.idl        \
    SAXEventKeeper.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
