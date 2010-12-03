/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: securityenvironment_nssimpl.cxx,v $
 * $Revision: 1.23 $
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


#include "secerr.h"
#include "sslerr.h"
#include "nspr.h"
#include "certt.h"
#include <sal/macros.h>

#include "../diagnose.hxx"

using namespace xmlsecurity;

struct ErrDesc {
    PRErrorCode  errNum;
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
    static char sEmpty[] = "";
    const int numDesc = SAL_N_ELEMENTS(allDesc);
    for (int i = 0; i < numDesc; i++)
    {
        if (allDesc[i].errNum == errNum)
            return  allDesc[i].errString;
    }

    return sEmpty;
}

void
printChainFailure(CERTVerifyLog *log)
{
    unsigned long errorFlags  = 0;
    unsigned int       depth  = (unsigned int)-1;
    const char * specificError = NULL;
    const char * issuer = NULL;
    CERTVerifyLogNode *node   = NULL;

    if (log->count > 0)
    {
        xmlsec_trace("Bad certifcation path:");
        for (node = log->head; node; node = node->next)
        {
            if (depth != node->depth)
            {
                depth = node->depth;
                xmlsec_trace("Certificate:  %d. %s %s:", depth,
                        node->cert->subjectName,
                        depth ? "[Certificate Authority]": "");
            }
            xmlsec_trace("  ERROR %ld: %s", node->error,
                    getCertError(node->error));
            specificError = NULL;
            issuer = NULL;
            switch (node->error)
            {
            case SEC_ERROR_INADEQUATE_KEY_USAGE:
                errorFlags = (unsigned long)node->arg;
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
            case SEC_ERROR_INADEQUATE_CERT_TYPE:
                errorFlags = (unsigned long)node->arg;
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
                xmlsec_trace("%s", specificError);
            if (issuer)
                xmlsec_trace("%s", issuer);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
