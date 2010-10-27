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

{SEC_ERROR_IO, "An I/O error occurred during security authorization."},

{SEC_ERROR_LIBRARY_FAILURE, "security library failure."},

{SEC_ERROR_BAD_DATA, "security library: received bad data."},

{SEC_ERROR_OUTPUT_LEN, "security library: output length error."},

{SEC_ERROR_INPUT_LEN, "security library has experienced an input length error."},

{SEC_ERROR_INVALID_ARGS, "security library: invalid arguments."},

{SEC_ERROR_INVALID_ALGORITHM, "security library: invalid algorithm."},

{SEC_ERROR_INVALID_AVA, "security library: invalid AVA."},

{SEC_ERROR_INVALID_TIME, "Improperly formatted time string."},

{SEC_ERROR_BAD_DER, "security library: improperly formatted DER-encoded message."},

{SEC_ERROR_BAD_SIGNATURE, "Peer's certificate has an invalid signature."},

{SEC_ERROR_EXPIRED_CERTIFICATE, "Peer's Certificate has expired."},

{SEC_ERROR_REVOKED_CERTIFICATE, "Peer's Certificate has been revoked."},

{SEC_ERROR_UNKNOWN_ISSUER, "Peer's Certificate issuer is not recognized."},

{SEC_ERROR_BAD_KEY, "Peer's public key is invalid."},

{SEC_ERROR_BAD_PASSWORD, "The security password entered is incorrect."},

{SEC_ERROR_RETRY_PASSWORD, "New password entered incorrectly.  Please try again."},

{SEC_ERROR_NO_NODELOCK, "security library: no nodelock."},

{SEC_ERROR_BAD_DATABASE, "security library: bad database."},

{SEC_ERROR_NO_MEMORY, "security library: memory allocation failure."},

{SEC_ERROR_UNTRUSTED_ISSUER, "Peer's certificate issuer has been marked as not trusted by the user."},

{SEC_ERROR_UNTRUSTED_CERT, "Peer's certificate has been marked as not trusted by the user."},

{SEC_ERROR_DUPLICATE_CERT, "Certificate already exists in your database."},

{SEC_ERROR_DUPLICATE_CERT_NAME, "Downloaded certificate's name duplicates one already in your database."},

{SEC_ERROR_ADDING_CERT, "Error adding certificate to database."},

{SEC_ERROR_FILING_KEY, "Error refiling the key for this certificate."},

{SEC_ERROR_NO_KEY, "The private key for this certificate cannot be found in key database"},

{SEC_ERROR_CERT_VALID, "This certificate is valid."},

{SEC_ERROR_CERT_NOT_VALID, "This certificate is not valid."},

{SEC_ERROR_CERT_NO_RESPONSE, "Cert Library: No Response"},

{SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE, "The certificate issuer's certificate has expired.  Check your system date and time."},

{SEC_ERROR_CRL_EXPIRED, "The CRL for the certificate's issuer has expired.  Update it or check your system date and time."},

{SEC_ERROR_CRL_BAD_SIGNATURE, "The CRL for the certificate's issuer has an invalid signature."},

{SEC_ERROR_CRL_INVALID, "New CRL has an invalid format."},

{SEC_ERROR_EXTENSION_VALUE_INVALID, "Certificate extension value is invalid."},

{SEC_ERROR_EXTENSION_NOT_FOUND, "Certificate extension not found."},

{SEC_ERROR_CA_CERT_INVALID, "Issuer certificate is invalid."},

{SEC_ERROR_PATH_LEN_CONSTRAINT_INVALID, "Certificate path length constraint is invalid."},

{SEC_ERROR_CERT_USAGES_INVALID, "Certificate usages field is invalid."},

{SEC_INTERNAL_ONLY, "**Internal ONLY module**"},

{SEC_ERROR_INVALID_KEY, "The key does not support the requested operation."},

{SEC_ERROR_UNKNOWN_CRITICAL_EXTENSION, "Certificate contains unknown critical extension."},

{SEC_ERROR_OLD_CRL, "New CRL is not later than the current one."},

{SEC_ERROR_NO_EMAIL_CERT, "Not encrypted or signed: you do not yet have an email certificate."},

{SEC_ERROR_NO_RECIPIENT_CERTS_QUERY, "Not encrypted: you do not have certificates for each of the recipients."},

{SEC_ERROR_NOT_A_RECIPIENT, "Cannot decrypt: you are not a recipient, or matching certificate and \
private key not found."},

{SEC_ERROR_PKCS7_KEYALG_MISMATCH, "Cannot decrypt: key encryption algorithm does not match your certificate."},

{SEC_ERROR_PKCS7_BAD_SIGNATURE, "Signature verification failed: no signer found, too many signers found, \
or improper or corrupted data."},

{SEC_ERROR_UNSUPPORTED_KEYALG, "Unsupported or unknown key algorithm."},

{SEC_ERROR_DECRYPTION_DISALLOWED, "Cannot decrypt: encrypted using a disallowed algorithm or key size."},


/* Fortezza Alerts */
{XP_SEC_FORTEZZA_BAD_CARD, "Fortezza card has not been properly initialized.  \
Please remove it and return it to your issuer."},

{XP_SEC_FORTEZZA_NO_CARD, "No Fortezza cards Found"},

{XP_SEC_FORTEZZA_NONE_SELECTED, "No Fortezza card selected"},

{XP_SEC_FORTEZZA_MORE_INFO, "Please select a personality to get more info on"},

{XP_SEC_FORTEZZA_PERSON_NOT_FOUND, "Personality not found"},

{XP_SEC_FORTEZZA_NO_MORE_INFO, "No more information on that Personality"},

{XP_SEC_FORTEZZA_BAD_PIN, "Invalid Pin"},

{XP_SEC_FORTEZZA_PERSON_ERROR, "Couldn't initialize Fortezza personalities."},
/* end fortezza alerts. */

{SEC_ERROR_NO_KRL, "No KRL for this site's certificate has been found."},

{SEC_ERROR_KRL_EXPIRED, "The KRL for this site's certificate has expired."},

{SEC_ERROR_KRL_BAD_SIGNATURE, "The KRL for this site's certificate has an invalid signature."},

{SEC_ERROR_REVOKED_KEY, "The key for this site's certificate has been revoked."},

{SEC_ERROR_KRL_INVALID, "New KRL has an invalid format."},

{SEC_ERROR_NEED_RANDOM, "security library: need random data."},

{SEC_ERROR_NO_MODULE, "security library: no security module can perform the requested operation."},

{SEC_ERROR_NO_TOKEN, "The security card or token does not exist, needs to be initialized, or has been removed."},

{SEC_ERROR_READ_ONLY, "security library: read-only database."},

{SEC_ERROR_NO_SLOT_SELECTED, "No slot or token was selected."},

{SEC_ERROR_CERT_NICKNAME_COLLISION, "A certificate with the same nickname already exists."},

{SEC_ERROR_KEY_NICKNAME_COLLISION, "A key with the same nickname already exists."},

{SEC_ERROR_SAFE_NOT_CREATED, "error while creating safe object"},

{SEC_ERROR_BAGGAGE_NOT_CREATED, "error while creating baggage object"},

{XP_JAVA_REMOVE_PRINCIPAL_ERROR, "Couldn't remove the principal"},

{XP_JAVA_DELETE_PRIVILEGE_ERROR, "Couldn't delete the privilege"},

{XP_JAVA_CERT_NOT_EXISTS_ERROR, "This principal doesn't have a certificate"},

{SEC_ERROR_BAD_EXPORT_ALGORITHM, "Required algorithm is not allowed."},

{SEC_ERROR_EXPORTING_CERTIFICATES, "Error attempting to export certificates."},

{SEC_ERROR_IMPORTING_CERTIFICATES, "Error attempting to import certificates."},

{SEC_ERROR_PKCS12_DECODING_PFX, "Unable to import.  Decoding error.  File not valid."},

{SEC_ERROR_PKCS12_INVALID_MAC, "Unable to import.  Invalid MAC.  Incorrect password or corrupt file."},

{SEC_ERROR_PKCS12_UNSUPPORTED_MAC_ALGORITHM, "Unable to import.  MAC algorithm not supported."},

{SEC_ERROR_PKCS12_UNSUPPORTED_TRANSPORT_MODE, "Unable to import.  Only password integrity and privacy modes supported."},

{SEC_ERROR_PKCS12_CORRUPT_PFX_STRUCTURE, "Unable to import.  File structure is corrupt."},

{SEC_ERROR_PKCS12_UNSUPPORTED_PBE_ALGORITHM, "Unable to import.  Encryption algorithm not supported."},

{SEC_ERROR_PKCS12_UNSUPPORTED_VERSION, "Unable to import.  File version not supported."},

{SEC_ERROR_PKCS12_PRIVACY_PASSWORD_INCORRECT, "Unable to import.  Incorrect privacy password."},

{SEC_ERROR_PKCS12_CERT_COLLISION, "Unable to import.  Same nickname already exists in database."},

{SEC_ERROR_USER_CANCELLED, "The user pressed cancel."},

{SEC_ERROR_PKCS12_DUPLICATE_DATA, "Not imported, already in database."},

{SEC_ERROR_MESSAGE_SEND_ABORTED, "Message not sent."},

{SEC_ERROR_INADEQUATE_KEY_USAGE, "Certificate key usage inadequate for attempted operation."},

{SEC_ERROR_INADEQUATE_CERT_TYPE, "Certificate type not approved for application."},

{SEC_ERROR_CERT_ADDR_MISMATCH, "Address in signing certificate does not match address in message headers."},

{SEC_ERROR_PKCS12_UNABLE_TO_IMPORT_KEY, "Unable to import.  Error attempting to import private key."},

{SEC_ERROR_PKCS12_IMPORTING_CERT_CHAIN, "Unable to import.  Error attempting to import certificate chain."},

{SEC_ERROR_PKCS12_UNABLE_TO_LOCATE_OBJECT_BY_NAME, "Unable to export.  Unable to locate certificate or key by nickname."},

{SEC_ERROR_PKCS12_UNABLE_TO_EXPORT_KEY, "Unable to export.  Private Key could not be located and exported."},

{SEC_ERROR_PKCS12_UNABLE_TO_WRITE, "Unable to export.  Unable to write the export file."},

{SEC_ERROR_PKCS12_UNABLE_TO_READ, "Unable to import.  Unable to read the import file."},

{SEC_ERROR_PKCS12_KEY_DATABASE_NOT_INITIALIZED, "Unable to export.  Key database corrupt or deleted."},

{SEC_ERROR_KEYGEN_FAIL, "Unable to generate public/private key pair."},

{SEC_ERROR_INVALID_PASSWORD, "Password entered is invalid.  Please pick a different one."},

{SEC_ERROR_RETRY_OLD_PASSWORD, "Old password entered incorrectly.  Please try again."},

{SEC_ERROR_BAD_NICKNAME, "Certificate nickname already in use."},

{SEC_ERROR_NOT_FORTEZZA_ISSUER, "Peer FORTEZZA chain has a non-FORTEZZA Certificate."},

{SEC_ERROR_CANNOT_MOVE_SENSITIVE_KEY, "A sensitive key cannot be moved to the slot where it is needed."},

{SEC_ERROR_JS_INVALID_MODULE_NAME, "Invalid module name."},

{SEC_ERROR_JS_INVALID_DLL, "Invalid module path/filename"},

{SEC_ERROR_JS_ADD_MOD_FAILURE, "Unable to add module"},

{SEC_ERROR_JS_DEL_MOD_FAILURE, "Unable to delete module"},

{SEC_ERROR_OLD_KRL, "New KRL is not later than the current one."},

{SEC_ERROR_CKL_CONFLICT, "New CKL has different issuer than current CKL.  Delete current CKL."},

{SEC_ERROR_CERT_NOT_IN_NAME_SPACE, "The Certifying Authority for this certificate is not permitted to issue a \
certificate with this name."},

{SEC_ERROR_KRL_NOT_YET_VALID, "The key revocation list for this certificate is not yet valid."},

{SEC_ERROR_CRL_NOT_YET_VALID, "The certificate revocation list for this certificate is not yet valid."},

{SEC_ERROR_UNKNOWN_CERT, "The requested certificate could not be found."},

{SEC_ERROR_UNKNOWN_SIGNER, "The signer's certificate could not be found."},

{SEC_ERROR_CERT_BAD_ACCESS_LOCATION, "The location for the certificate status server has invalid format."},

{SEC_ERROR_OCSP_UNKNOWN_RESPONSE_TYPE, "The OCSP response cannot be fully decoded; it is of an unknown type."},

{SEC_ERROR_OCSP_BAD_HTTP_RESPONSE, "The OCSP server returned unexpected/invalid HTTP data."},

{SEC_ERROR_OCSP_MALFORMED_REQUEST, "The OCSP server found the request to be corrupted or improperly formed."},

{SEC_ERROR_OCSP_SERVER_ERROR, "The OCSP server experienced an internal error."},

{SEC_ERROR_OCSP_TRY_SERVER_LATER, "The OCSP server suggests trying again later."},

{SEC_ERROR_OCSP_REQUEST_NEEDS_SIG, "The OCSP server requires a signature on this request."},

{SEC_ERROR_OCSP_UNAUTHORIZED_REQUEST, "The OCSP server has refused this request as unauthorized."},

{SEC_ERROR_OCSP_UNKNOWN_RESPONSE_STATUS, "The OCSP server returned an unrecognizable status."},

{SEC_ERROR_OCSP_UNKNOWN_CERT, "The OCSP server has no status for the certificate."},

{SEC_ERROR_OCSP_NOT_ENABLED, "You must enable OCSP before performing this operation."},

{SEC_ERROR_OCSP_NO_DEFAULT_RESPONDER, "You must set the OCSP default responder before performing this operation."},

{SEC_ERROR_OCSP_MALFORMED_RESPONSE, "The response from the OCSP server was corrupted or improperly formed."},

{SEC_ERROR_OCSP_UNAUTHORIZED_RESPONSE, "The signer of the OCSP response is not authorized to give status for \
this certificate."},

{SEC_ERROR_OCSP_FUTURE_RESPONSE, "The OCSP response is not yet valid (contains a date in the future},."},

{SEC_ERROR_OCSP_OLD_RESPONSE, "The OCSP response contains out-of-date information."},

{SEC_ERROR_DIGEST_NOT_FOUND, "The CMS or PKCS #7 Digest was not found in signed message."},

{SEC_ERROR_UNSUPPORTED_MESSAGE_TYPE, "The CMS or PKCS #7 Message type is unsupported."},

{SEC_ERROR_MODULE_STUCK, "PKCS #11 module could not be removed because it is still in use."},

{SEC_ERROR_BAD_TEMPLATE, "Could not decode ASN.1 data. Specified template was invalid."},

{SEC_ERROR_CRL_NOT_FOUND, "No matching CRL was found."},

{SEC_ERROR_REUSED_ISSUER_AND_SERIAL, "You are attempting to import a cert with the same issuer/serial as \
an existing cert, but that is not the same cert."},

{SEC_ERROR_BUSY, "NSS could not shutdown. Objects are still in use."},

{SEC_ERROR_EXTRA_INPUT, "DER-encoded message contained extra unused data."},

{SEC_ERROR_UNSUPPORTED_ELLIPTIC_CURVE, "Unsupported elliptic curve."},

{SEC_ERROR_UNSUPPORTED_EC_POINT_FORM, "Unsupported elliptic curve point form."},

{SEC_ERROR_UNRECOGNIZED_OID, "Unrecognized Object Identifier."},

{SEC_ERROR_OCSP_INVALID_SIGNING_CERT, "Invalid OCSP signing certificate in OCSP response."},

{SEC_ERROR_REVOKED_CERTIFICATE_CRL, "Certificate is revoked in issuer's certificate revocation list."},

{SEC_ERROR_REVOKED_CERTIFICATE_OCSP, "Issuer's OCSP responder reports certificate is revoked."},

{SEC_ERROR_CRL_INVALID_VERSION, "Issuer's Certificate Revocation List has an unknown version number."},

{SEC_ERROR_CRL_V1_CRITICAL_EXTENSION, "Issuer's V1 Certificate Revocation List has a critical extension."},

{SEC_ERROR_CRL_UNKNOWN_CRITICAL_EXTENSION, "Issuer's V2 Certificate Revocation List has an unknown critical extension."},

{SEC_ERROR_UNKNOWN_OBJECT_TYPE, "Unknown object type specified."},

{SEC_ERROR_INCOMPATIBLE_PKCS11, "PKCS #11 driver violates the spec in an incompatible way."},

{SEC_ERROR_NO_EVENT, "No new slot event is available at this time."},

{SEC_ERROR_CRL_ALREADY_EXISTS, "CRL already exists."},

{SEC_ERROR_NOT_INITIALIZED, "NSS is not initialized."},

{SEC_ERROR_TOKEN_NOT_LOGGED_IN, "The operation failed because the PKCS#11 token is not logged in."},

{SEC_ERROR_OCSP_RESPONDER_CERT_INVALID, "Configured OCSP responder's certificate is invalid."},

{SEC_ERROR_OCSP_BAD_SIGNATURE, "OCSP response has an invalid signature."},

{SEC_ERROR_OUT_OF_SEARCH_LIMITS, "Cert validation search is out of search limits"},

{SEC_ERROR_INVALID_POLICY_MAPPING, "Policy mapping contains anypolicy"},

{SEC_ERROR_POLICY_VALIDATION_FAILED, "Cert chain fails policy validation"},

{SEC_ERROR_UNKNOWN_AIA_LOCATION_TYPE, "Unknown location type in cert AIA extension"},

{SEC_ERROR_BAD_HTTP_RESPONSE, "Server returned bad HTTP response"},

{SEC_ERROR_BAD_LDAP_RESPONSE, "Server returned bad LDAP response"},

{SEC_ERROR_FAILED_TO_ENCODE_DATA, "Failed to encode data with ASN1 encoder"},

{SEC_ERROR_BAD_INFO_ACCESS_LOCATION, "Bad information access location in cert extension"},

{SEC_ERROR_LIBPKIX_INTERNAL, "Libpkix internal error occured during cert validation."},

#if ( NSS_VMAJOR > 3 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR > 12 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR == 12 && NSS_VPATCH > 2 )
// following 3 errors got first used in NSS 3.12.3
// they were in the header even in 3.12.2 but there was missing the mapping in pk11err.c
// see also https://bugzilla.mozilla.org/show_bug.cgi?id=453364

{SEC_ERROR_PKCS11_GENERAL_ERROR, "A PKCS #11 module returned CKR_GENERAL_ERROR, indicating that an unrecoverable error has occurred."},

{SEC_ERROR_PKCS11_FUNCTION_FAILED, "A PKCS #11 module returned CKR_FUNCTION_FAILED, indicating that the requested function could not be performed.  Trying the same operation again might succeed."},

{SEC_ERROR_PKCS11_DEVICE_ERROR, "A PKCS #11 module returned CKR_DEVICE_ERROR, indicating that a problem has occurred with the token or slot."},

#endif

#if ( NSS_VMAJOR > 3 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR > 12 ) || ( NSS_VMAJOR == 3 && NSS_VMINOR == 12 && NSS_VPATCH > 3 )
// following 2 errors got added in NSS 3.12.4

{SEC_ERROR_BAD_INFO_ACCESS_METHOD, "Unknown information access method in certificate extension."},

{SEC_ERROR_CRL_IMPORT_FAILED, "Error attempting to import a CRL."},

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
