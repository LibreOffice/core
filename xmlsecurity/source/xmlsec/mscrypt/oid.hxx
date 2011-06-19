/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OID_HXX_
#define _OID_HXX_

typedef struct OIDItemStr OIDItem;

struct OIDItemStr {
    char *oid;
    char *desc;
};

OIDItem OIDs[] = {
    {"1.2.840.113549",  "RSA Data Security Inc."},
    {"1.2.840.113549.1",    "PKCS"},
    {"1.2.840.113549.2",    "RSA digest algorithm"},
    {"1.2.840.113549.3",    "RSA cipher algorithm"},
    {"1.2.840.113549.1.1",  "PKCS #1"},
    {"1.2.840.113549.1.2",  "Unknown"},
    {"1.2.840.113549.1.3",  "Unknown"},
    {"1.2.840.113549.1.4",  "Unknown"},
    {"1.2.840.113549.1.5",  "PKCS #5"},
    {"1.2.840.113549.1.6",  "Unknown"},
    {"1.2.840.113549.1.7",  "PKCS #7"},
    {"1.2.840.113549.1.8",  "Unknown"},
    {"1.2.840.113549.1.9",  "PKCS #9"},
    {"1.2.840.113549.1.10", "Unknown"},
    {"1.2.840.113549.1.12", "PKCS #12"},
    {"1.2.840.113549.1.1.2",    "PKCS #1 MD2 With RSA Encryption"},
    {"1.2.840.113549.1.1.3",    "PKCS #1 MD4 With RSA Encryption"},
    {"1.2.840.113549.1.1.4",    "PKCS #1 MD5 With RSA Encryption"},
    {"1.2.840.113549.1.1.1",    "PKCS #1 RSA Encryption"},
    {"1.2.840.113549.1.1.2",    "PKCS #1 MD2 With RSA Encryption"},
    {"1.2.840.113549.1.1.3",    "PKCS #1 MD4 With RSA Encryption"},
    {"1.2.840.113549.1.1.4",    "PKCS #1 MD5 With RSA Encryption"},
    {"1.2.840.113549.1.1.5",    "PKCS #1 SHA-1 With RSA Encryption"},
    {"1.2.840.113549.1.1.5",    "PKCS #1 SHA-1 With RSA Encryption"},
    {"1.2.840.113549.1.3.1",    "Unknown"},
    {"1.2.840.113549.1.7.1",    "PKCS #7 Data"},
    {"1.2.840.113549.1.7.2",    "PKCS #7 Signed Data"},
    {"1.2.840.113549.1.7.3",    "PKCS #7 Enveloped Data"},
    {"1.2.840.113549.1.7.4",    "PKCS #7 Signed and Enveloped Data"},
    {"1.2.840.113549.1.7.5",    "PKCS #7 Digested Data"},
    {"1.2.840.113549.1.7.5",    "PKCS #7 Digested Data"},
    {"1.2.840.113549.1.7.6",    "PKCS #7 Encrypted Data"},
    {"1.2.840.113549.1.9.1",    "PKCS #9 Email Address"},
    {"1.2.840.113549.1.9.2",    "PKCS #9 Unstructured Name"},
    {"1.2.840.113549.1.9.3",    "PKCS #9 Content Type"},
    {"1.2.840.113549.1.9.4",    "PKCS #9 Message Digest"},
    {"1.2.840.113549.1.9.5",    "PKCS #9 Signing Time"},
    {"1.2.840.113549.1.9.6",    "PKCS #9 Counter Signature"},
    {"1.2.840.113549.1.9.7",    "PKCS #9 Challenge Password"},
    {"1.2.840.113549.1.9.8",    "PKCS #9 Unstructured Address"},
    {"1.2.840.113549.1.9.9",    "PKCS #9 Extended Certificate Attributes"},
    {"1.2.840.113549.1.9.15",   "PKCS #9 S/MIME Capabilities"},
    {"1.2.840.113549.1.9.15.1", "Unknown"},
    {"1.2.840.113549.3.2",  "RC2-CBC"},
    {"1.2.840.113549.3.4",  "RC4"},
    {"1.2.840.113549.3.7",  "DES-EDE3-CBC"},
    {"1.2.840.113549.3.9",  "RC5-CBCPad"},
    {"1.2.840.10046",   "ANSI X9.42"},
    {"1.2.840.10046.2.1",   "Diffie-Hellman Public Key Algorithm"},
    {"1.2.840.10040",   "ANSI X9.57"},
    {"1.2.840.10040.4.1",   "ANSI X9.57 DSA Signature"},
    {"1.2.840.10040.4.3",   "ANSI X9.57 Algorithm DSA Signature with SHA-1 Digest"},
    {"2.5", "Directory"},
    {"2.5.8",   "X.500-defined algorithms"},
    {"2.5.8.1", "X.500-defined encryption algorithms"},
    {"2.5.8.2", "Unknown"},
    {"2.5.8.3", "Unknown"},
    {"2.5.8.1.1",   "RSA Encryption Algorithm"},
    {"1.3.14",  "Open Systems Implementors Workshop"},
    {"1.3.14.3.2",  "OIW SECSIG Algorithm"},
    {"1.3.14.3.2.2",    "Unknown"},
    {"1.3.14.3.2.3",    "Unknown"},
    {"1.3.14.3.2.4",    "Unknown"},
    {"1.3.14.3.2.6",    "DES-ECB"},
    {"1.3.14.3.2.7",    "DES-CBC"},
    {"1.3.14.3.2.8",    "DES-OFB"},
    {"1.3.14.3.2.9",    "DES-CFB"},
    {"1.3.14.3.2.10",   "DES-MAC"},
    {"1.3.14.3.2.11",   "Unknown"},
    {"1.3.14.3.2.12",   "Unknown"},
    {"1.3.14.3.2.13",   "Unknown"},
    {"1.3.14.3.2.14",   "Unknown"},
    {"1.3.14.3.2.15",   "ISO SHA with RSA Signature"},
    {"1.3.14.3.2.16",   "Unknown"},
    {"1.3.14.3.2.17",   "DES-EDE"},
    {"1.3.14.3.2.18",   "Unknown"},
    {"1.3.14.3.2.19",   "Unknown"},
    {"1.3.14.3.2.20",   "Unknown"},
    {"1.3.14.3.2.21",   "Unknown"},
    {"1.3.14.3.2.22",   "Unknown"},
    {"1.3.14.3.2.23",   "Unknown"},
    {"1.3.14.3.2.24",   "Unknown"},
    {"1.3.14.3.2.25",   "Unknown"},
    {"1.3.14.3.2.26",   "SHA-1"},
    {"1.3.14.3.2.27",   "Forgezza DSA Signature with SHA-1 Digest"},
    {"1.3.14.3.2.28",   "Unknown"},
    {"1.3.14.3.2.29",   "Unknown"},
    {"1.3.14.7.2",  "Unknown"},
    {"1.3.14.7.2.1",    "Unknown"},
    {"1.3.14.7.2.2",    "Unknown"},
    {"1.3.14.7.2.3",    "Unknown"},
    {"1.3.14.7.2.2.1",  "Unknown"},
    {"1.3.14.7.2.3.1",  "Unknown"},
    {"2.16.840.1.101.2.1",  "US DOD Infosec"},
    {"2.16.840.1.101.2.1.1.1",  "Unknown"},
    {"2.16.840.1.101.2.1.1.2",  "MISSI DSS Algorithm (Old)"},
    {"2.16.840.1.101.2.1.1.3",  "Unknown"},
    {"2.16.840.1.101.2.1.1.4",  "Skipjack CBC64"},
    {"2.16.840.1.101.2.1.1.5",  "Unknown"},
    {"2.16.840.1.101.2.1.1.6",  "Unknown"},
    {"2.16.840.1.101.2.1.1.7",  "Unknown"},
    {"2.16.840.1.101.2.1.1.8",  "Unknown"},
    {"2.16.840.1.101.2.1.1.9",  "Unknown"},
    {"2.16.840.1.101.2.1.1.10", "MISSI KEA Algorithm"},
    {"2.16.840.1.101.2.1.1.11", "Unknown"},
    {"2.16.840.1.101.2.1.1.12", "MISSI KEA and DSS Algorithm (Old)"},
    {"2.16.840.1.101.2.1.1.13", "Unknown"},
    {"2.16.840.1.101.2.1.1.14", "Unknown"},
    {"2.16.840.1.101.2.1.1.15", "Unknown"},
    {"2.16.840.1.101.2.1.1.16", "Unknown"},
    {"2.16.840.1.101.2.1.1.17", "Unknown"},
    {"2.16.840.1.101.2.1.1.18", "Unknown"},
    {"2.16.840.1.101.2.1.1.19", "MISSI DSS Algorithm"},
    {"2.16.840.1.101.2.1.1.20", "MISSI KEA and DSS Algorithm"},
    {"2.16.840.1.101.2.1.1.21", "Unknown"}
};

int nOID = 115;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
