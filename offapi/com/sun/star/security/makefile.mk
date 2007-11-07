#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2007-11-07 09:58:00 $
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

#i20156 - new file for xmlsecurity module

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=xsec-security
PACKAGE=com$/sun$/star$/security

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    CertificateException.idl            \
    CryptographyException.idl           \
    DocumentDigitalSignatures.idl       \
    DocumentSignatureInformation.idl   \
    EncryptionException.idl             \
    KeyException.idl                    \
    NoPasswordException.idl             \
    SecurityInfrastructureException.idl \
    SignatureException.idl              \
    XCertificate.idl                    \
    CertificateCharacters.idl           \
    CertificateValidity.idl             \
    XCertificateExtension.idl           \
    XDocumentDigitalSignatures.idl	\
    KeyUsage.idl			\
    XCertificateContainer.idl		\
    CertificateContainer.idl \
    CertificateContainerStatus.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
