/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <secerr.h>
#include "secerror.hxx"
#include <sslerr.h>
#include <nspr.h>
#include <nss.h>
#include <certt.h>
#include <sal/log.hxx>
#include <sal/macros.h>
#include <sal/types.h>

struct ErrDesc {
    PRErrorCode const errNum;
    const char * errString;
};


const ErrDesc allDesc[] = {

#include "certerrors.h"

};


/* Returns a UTF-8 encoded constant error string for "errNum".
 * Returns NULL of errNum is unknown.
 */
const char *
getCertError(PRErrorCode errNum)
{
    for (const ErrDesc& i : allDesc)
    {
        if (i.errNum == errNum)
            return i.errString;
    }

    return "";
}

void
printChainFailure(CERTVerifyLog *log)
{
    unsigned int       depth  = static_cast<unsigned int>(-1);
    CERTVerifyLogNode *node   = nullptr;

    if (log->count > 0)
    {
        SAL_INFO("xmlsecurity.xmlsec", "Bad certification path:");
        unsigned long errorFlags  = 0;
        for (node = log->head; node; node = node->next)
        {
            if (depth != node->depth)
            {
                depth = node->depth;
                SAL_INFO("xmlsecurity.xmlsec", "Certificate:  " << depth <<
                         node->cert->subjectName << ": " <<
                         (depth ? "[Certificate Authority]": ""));
            }
            SAL_INFO("xmlsecurity.xmlsec", "  ERROR " << node->error << ": " <<
                     getCertError(node->error));
            const char * specificError = nullptr;
            const char * issuer = nullptr;
            switch (node->error)
            {
            case SEC_ERROR_INADEQUATE_KEY_USAGE:
                errorFlags = reinterpret_cast<unsigned long>(node->arg);
                switch (errorFlags)
                {
                case KU_DIGITAL_SIGNATURE:
                    specificError = "Certificate cannot sign.";
                    break;
                case KU_KEY_ENCIPHERMENT:
                    specificError = "Certificate cannot encrypt.";
                    break;
                case KU_KEY_CERT_SIGN:
                    specificError = "Certificate cannot sign other certs.";
                    break;
                default:
                    specificError = "[unknown usage].";
                    break;
                }
                break;
            case SEC_ERROR_INADEQUATE_CERT_TYPE:
                errorFlags = reinterpret_cast<unsigned long>(node->arg);
                switch (errorFlags)
                {
                case NS_CERT_TYPE_SSL_CLIENT:
                case NS_CERT_TYPE_SSL_SERVER:
                    specificError = "Certificate cannot be used for SSL.";
                    break;
                case NS_CERT_TYPE_SSL_CA:
                    specificError = "Certificate cannot be used as an SSL CA.";
                    break;
                case NS_CERT_TYPE_EMAIL:
                    specificError = "Certificate cannot be used for SMIME.";
                    break;
                case NS_CERT_TYPE_EMAIL_CA:
                    specificError = "Certificate cannot be used as an SMIME CA.";
                    break;
                case NS_CERT_TYPE_OBJECT_SIGNING:
                    specificError = "Certificate cannot be used for object signing.";
                    break;
                case NS_CERT_TYPE_OBJECT_SIGNING_CA:
                    specificError = "Certificate cannot be used as an object signing CA.";
                    break;
                default:
                    specificError = "[unknown usage].";
                    break;
                }
                break;
            case SEC_ERROR_UNKNOWN_ISSUER:
                specificError = "Unknown issuer:";
                issuer = node->cert->issuerName;
                break;
            case SEC_ERROR_UNTRUSTED_ISSUER:
                specificError = "Untrusted issuer:";
                issuer = node->cert->issuerName;
                break;
            case SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE:
                specificError = "Expired issuer certificate:";
                issuer = node->cert->issuerName;
                break;
            default:
                break;
            }
            if (specificError)
                SAL_INFO("xmlsecurity.xmlsec", specificError);
            if (issuer)
                SAL_INFO("xmlsecurity.xmlsec", issuer);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
