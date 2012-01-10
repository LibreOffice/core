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

PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=xsec-crypto
PACKAGE=com$/sun$/star$/xml$/crypto

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    XXMLSecurityTemplate.idl     \
    XXMLSignature.idl     \
    XXMLSignatureTemplate.idl     \
    XXMLEncryption.idl     \
    XXMLEncryptionTemplate.idl     \
    XXMLSecurityContext.idl     \
    XSecurityEnvironment.idl     \
    XSEInitializer.idl     \
    XMLSignature.idl     \
    XMLSignatureTemplate.idl     \
    XMLEncryption.idl     \
    XMLEncryptionTemplate.idl     \
    XMLSecurityContext.idl     \
    SecurityEnvironment.idl     \
    SEInitializer.idl     \
    XMLSignatureException.idl     \
    XMLEncryptionException.idl     \
    XUriBinding.idl    \
    CipherID.idl \
    DigestID.idl \
    XCipherContext.idl \
    XCipherContextSupplier.idl \
    XDigestContext.idl \
    XDigestContextSupplier.idl \
    SecurityOperationStatus.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
