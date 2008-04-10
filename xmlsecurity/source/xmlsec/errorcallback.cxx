/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: errorcallback.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include "errorcallback.hxx"

#include "xmlsec/xmlsec.h"
#include "xmlsec/errors.h"

using namespace ::com::sun::star::xml::crypto;

// ::com::sun::star::uno::Reference< XXMLSecurityTemplate > g_xErrorRecorder;
// bool g_bErrorRecorded;

// SecurityOperationStatus getOperationStatus(int reason)
// {
//  switch (reason)
//  {
//  case XMLSEC_ERRORS_R_XMLSEC_FAILED:
//      return SecurityOperationStatus_ENGINE_FAILED;
//  case XMLSEC_ERRORS_R_MALLOC_FAILED:
//      return SecurityOperationStatus_MALLOC_FAILED;
//  case XMLSEC_ERRORS_R_STRDUP_FAILED:
//      return SecurityOperationStatus_STRDUP_FAILED;
//  case XMLSEC_ERRORS_R_CRYPTO_FAILED:
//      return SecurityOperationStatus_CRYPTO_FAILED;
//  case XMLSEC_ERRORS_R_XML_FAILED:
//      return SecurityOperationStatus_XML_FAILED;
//  case XMLSEC_ERRORS_R_XSLT_FAILED:
//      return SecurityOperationStatus_XSLT_FAILED;
//  case XMLSEC_ERRORS_R_IO_FAILED:
//      return SecurityOperationStatus_IO_FAILED;
//  case XMLSEC_ERRORS_R_DISABLED:
//      return SecurityOperationStatus_DISABLED;
//  case XMLSEC_ERRORS_R_NOT_IMPLEMENTED:
//      return SecurityOperationStatus_NOT_IMPLEMENTED;
//  case XMLSEC_ERRORS_R_INVALID_SIZE:
//      return SecurityOperationStatus_INVALID_SIZE;
//  case XMLSEC_ERRORS_R_INVALID_DATA:
//      return SecurityOperationStatus_INVALID_DATA;
//  case XMLSEC_ERRORS_R_INVALID_RESULT:
//      return SecurityOperationStatus_INVALID_RESULT;
//  case XMLSEC_ERRORS_R_INVALID_TYPE:
//      return SecurityOperationStatus_INVALID_TYPE;
//  case XMLSEC_ERRORS_R_INVALID_OPERATION:
//      return SecurityOperationStatus_INVALID_OPERATION;
//  case XMLSEC_ERRORS_R_INVALID_STATUS:
//      return SecurityOperationStatus_INVALID_STATUS;
//  case XMLSEC_ERRORS_R_INVALID_FORMAT:
//      return SecurityOperationStatus_INVALID_FORMAT;
//  case XMLSEC_ERRORS_R_DATA_NOT_MATCH:
//      return SecurityOperationStatus_DATA_NOT_MATCH;
//  case XMLSEC_ERRORS_R_INVALID_NODE:
//      return SecurityOperationStatus_INVALID_NODE;
//  case XMLSEC_ERRORS_R_INVALID_NODE_CONTENT:
//      return SecurityOperationStatus_INVALID_NODE_CONTENT;
//  case XMLSEC_ERRORS_R_INVALID_NODE_ATTRIBUTE:
//      return SecurityOperationStatus_INVALID_NODE_ATTRIBUTE;
//  case XMLSEC_ERRORS_R_MISSING_NODE_ATTRIBUTE:
//      return SecurityOperationStatus_MISSING_NODE_ATTRIBUTE;
//  case XMLSEC_ERRORS_R_NODE_ALREADY_PRESENT:
//      return SecurityOperationStatus_NODE_ALREADY_PRESENT;
//  case XMLSEC_ERRORS_R_UNEXPECTED_NODE:
//      return SecurityOperationStatus_UNEXPECTED_NODE;
//  case XMLSEC_ERRORS_R_NODE_NOT_FOUND:
//      return SecurityOperationStatus_NODE_NOT_FOUND;
//  case XMLSEC_ERRORS_R_INVALID_TRANSFORM:
//      return SecurityOperationStatus_INVALID_TRANSFORM;
//  case XMLSEC_ERRORS_R_INVALID_TRANSFORM_KEY:
//      return SecurityOperationStatus_INVALID_TRANSFORM_KEY;
//  case XMLSEC_ERRORS_R_INVALID_URI_TYPE:
//      return SecurityOperationStatus_INVALID_URI_TYPE;
//  case XMLSEC_ERRORS_R_TRANSFORM_SAME_DOCUMENT_REQUIRED:
//      return SecurityOperationStatus_TRANSFORM_SAME_DOCUMENT_REQUIRED;
//  case XMLSEC_ERRORS_R_TRANSFORM_DISABLED:
//      return SecurityOperationStatus_TRANSFORM_DISABLED;
//  case XMLSEC_ERRORS_R_INVALID_KEY_DATA:
//      return SecurityOperationStatus_INVALID_KEY_DATA;
//  case XMLSEC_ERRORS_R_KEY_DATA_NOT_FOUND:
//      return SecurityOperationStatus_KEY_DATA_NOT_FOUND;
//  case XMLSEC_ERRORS_R_KEY_DATA_ALREADY_EXIST:
//      return SecurityOperationStatus_KEY_DATA_ALREADY_EXIST;
//  case XMLSEC_ERRORS_R_INVALID_KEY_DATA_SIZE:
//      return SecurityOperationStatus_INVALID_KEY_DATA_SIZE;
//  case XMLSEC_ERRORS_R_KEY_NOT_FOUND:
//      return SecurityOperationStatus_KEY_NOT_FOUND;
//  case XMLSEC_ERRORS_R_KEYDATA_DISABLED:
//      return SecurityOperationStatus_KEYDATA_DISABLED;
//  case XMLSEC_ERRORS_R_MAX_RETRIEVALS_LEVEL:
//      return SecurityOperationStatus_MAX_RETRIEVALS_LEVEL;
//  case XMLSEC_ERRORS_R_MAX_RETRIEVAL_TYPE_MISMATCH:
//      return SecurityOperationStatus_MAX_RETRIEVAL_TYPE_MISMATCH;
//  case XMLSEC_ERRORS_R_MAX_ENCKEY_LEVEL:
//      return SecurityOperationStatus_MAX_ENCKEY_LEVEL;
//  case XMLSEC_ERRORS_R_CERT_VERIFY_FAILED:
//      return SecurityOperationStatus_CERT_VERIFY_FAILED;
//  case XMLSEC_ERRORS_R_CERT_NOT_FOUND:
//      return SecurityOperationStatus_CERT_NOT_FOUND;
//  case XMLSEC_ERRORS_R_CERT_REVOKED:
//      return SecurityOperationStatus_CERT_REVOKED;
//  case XMLSEC_ERRORS_R_CERT_ISSUER_FAILED:
//      return SecurityOperationStatus_CERT_ISSUER_FAILED;
//  case XMLSEC_ERRORS_R_CERT_NOT_YET_VALID:
//      return SecurityOperationStatus_CERT_NOT_YET_VALID;
//  case XMLSEC_ERRORS_R_CERT_HAS_EXPIRED:
//      return SecurityOperationStatus_CERT_HAS_EXPIRED;
//  case XMLSEC_ERRORS_R_DSIG_NO_REFERENCES:
//      return SecurityOperationStatus_DSIG_NO_REFERENCES;
//  case XMLSEC_ERRORS_R_DSIG_INVALID_REFERENCE:
//      return SecurityOperationStatus_DSIG_INVALID_REFERENCE;
//  case XMLSEC_ERRORS_R_ASSERTION:
//      return SecurityOperationStatus_ASSERTION;
//  default:
//      return SecurityOperationStatus_RUNTIMEERROR_FAILED;
//  }
// }


extern "C"
void errorCallback(const char * /*file*/,
                   int /*line*/,
                   const char * /*func*/,
                   const char * /*errorObject*/,
                   const char * /*errorSubject*/,
                   int /*reason*/,
                   const char  * /*msg*/)
{
#if OSL_DEBUG_LEVEL > 1
//     const char * afunc = func ? func : "";
//     const char * errObj = errorObject ? errorObject : "";
//     const char * errSub = errorSubject ? errorSubject : "";
//     const char * amsg = msg ? msg : "";
//  fprintf(stdout, "xmlsec error: %s, %s,  %s, %i %s  \n", afunc, errObj, errSub, reason, amsg);
#endif
    //ToDo write log message
//  if (g_xErrorRecorder.is() && !g_bErrorRecorded)
//  {
//      g_xErrorRecorder->setStatus(getOperationStatus(reason));

//      if ( reason != XMLSEC_ERRORS_R_ASSERTION && reason!=XMLSEC_ERRORS_R_XMLSEC_FAILED)
//      {
//             g_bErrorRecorded = true;
//      }
//  }
}

// void setErrorRecorder(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSecurityTemplate >& xTemplate)
// {
//  g_xErrorRecorder = xTemplate;
//  g_xErrorRecorder->setStatus(SecurityOperationStatus_OPERATION_SUCCEEDED);
//  g_bErrorRecorded = false;
//  xmlSecErrorsSetCallback(errorCallback);
// }

//void setErrorRecorder(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSignatureTemplate >& xTemplate)

void setErrorRecorder()
{
//  ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSecurityTemplate >
//      xSecurityTemplate(xTemplate, ::com::sun::star::uno::UNO_QUERY);
//  setErrorRecorder( xSecurityTemplate );
    xmlSecErrorsSetCallback(errorCallback);
}

// void setErrorRecorder(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLEncryptionTemplate >& xTemplate)
// {
//  ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSecurityTemplate >
//      xSecurityTemplate(xTemplate, ::com::sun::star::uno::UNO_QUERY);
//  setErrorRecorder( xSecurityTemplate );
// }

void clearErrorRecorder()
{
    xmlSecErrorsSetCallback(NULL);
//  g_xErrorRecorder = NULL;
}

