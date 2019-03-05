/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/cryptosign.hxx>
#include <svl/sigstruct.hxx>
#include <config_features.h>

#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <tools/datetime.hxx>
#include <tools/stream.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/random.hxx>
#include <comphelper/hash.hxx>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <sax/tools/converter.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/datetime.hxx>
#include <xmloff/xmluconv.hxx>
#include <tools/zcodec.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#if HAVE_FEATURE_NSS && !defined(_WIN32)
// NSS headers for PDF signing
#include <nss.h>
#include <cert.h>
#include <hasht.h>
#include <secerr.h>
#include <sechash.h>
#include <cms.h>
#include <cmst.h>

// We use curl for RFC3161 time stamp requests
#include <curl/curl.h>
#endif

#ifdef _WIN32
// WinCrypt headers for PDF signing
// Note: this uses Windows 7 APIs and requires the relevant data types
#include <prewin.h>
#include <wincrypt.h>
#include <postwin.h>
#include <comphelper/windowserrorstring.hxx>
#endif

#if HAVE_FEATURE_NSS
// Is this length truly the maximum possible, or just a number that
// seemed large enough when the author tested this (with some type of
// certificates)? I suspect the latter.

// Used to be 0x4000 = 16384, but a sample signed PDF (produced by
// some other software) provided by the customer has a signature
// content that is 30000 bytes. The SampleSignedPDFDocument.pdf from
// Adobe has one that is 21942 bytes. So let's be careful. Pity this
// can't be dynamic, at least not without restructuring the code. Also
// note that the checks in the code for this being too small
// apparently are broken, if this overflows you end up with an invalid
// PDF. Need to fix that.

#define MAX_SIGNATURE_CONTENT_LENGTH 50000
#endif

using namespace com::sun::star;

namespace {

#if HAVE_FEATURE_NSS
void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}
#endif // HAVE_FEATURE_NSS

#if HAVE_FEATURE_NSS && !defined(_WIN32)

char *PDFSigningPKCS7PasswordCallback(PK11SlotInfo * /*slot*/, PRBool /*retry*/, void *arg)
{
    return PL_strdup(static_cast<char *>(arg));
}

// ASN.1 used in the (much simpler) time stamp request. From RFC3161
// and other sources.

/*
AlgorithmIdentifier  ::=  SEQUENCE  {
     algorithm  OBJECT IDENTIFIER,
     parameters ANY DEFINED BY algorithm OPTIONAL  }
                   -- contains a value of the type
                   -- registered for use with the
                   -- algorithm object identifier value

MessageImprint ::= SEQUENCE  {
    hashAlgorithm AlgorithmIdentifier,
    hashedMessage OCTET STRING  }
*/

struct MessageImprint {
    SECAlgorithmID hashAlgorithm;
    SECItem hashedMessage;
};

/*
Extension  ::=  SEQUENCE  {
    extnID    OBJECT IDENTIFIER,
    critical  BOOLEAN DEFAULT FALSE,
    extnValue OCTET STRING  }
*/

struct Extension {
    SECItem const extnID;
    SECItem const critical;
    SECItem const extnValue;
};

/*
Extensions ::= SEQUENCE SIZE (1..MAX) OF Extension
*/

/*
TSAPolicyId ::= OBJECT IDENTIFIER

TimeStampReq ::= SEQUENCE  {
    version            INTEGER  { v1(1) },
    messageImprint     MessageImprint,
    --a hash algorithm OID and the hash value of the data to be
    --time-stamped
    reqPolicy          TSAPolicyId         OPTIONAL,
    nonce              INTEGER             OPTIONAL,
    certReq            BOOLEAN             DEFAULT FALSE,
    extensions     [0] IMPLICIT Extensions OPTIONAL  }
*/

struct TimeStampReq {
    SECItem version;
    MessageImprint messageImprint;
    SECItem reqPolicy;
    SECItem nonce;
    SECItem certReq;
    Extension *extensions;
};

/**
 * General name, defined by RFC 3280.
 */
struct GeneralName
{
    CERTName name;
};

/**
 * List of general names (only one for now), defined by RFC 3280.
 */
struct GeneralNames
{
    GeneralName names;
};

/**
 * Supplies different fields to identify a certificate, defined by RFC 5035.
 */
struct IssuerSerial
{
    GeneralNames issuer;
    SECItem serialNumber;
};

/**
 * Supplies different fields that are used to identify certificates, defined by
 * RFC 5035.
 */
struct ESSCertIDv2
{
    SECAlgorithmID hashAlgorithm;
    SECItem certHash;
    IssuerSerial issuerSerial;
};

/**
 * This attribute uses the ESSCertIDv2 structure, defined by RFC 5035.
 */
struct SigningCertificateV2
{
    ESSCertIDv2** certs;

    SigningCertificateV2()
        : certs(nullptr)
    {
    }
};

/**
 * GeneralName ::= CHOICE {
 *      otherName                       [0]     OtherName,
 *      rfc822Name                      [1]     IA5String,
 *      dNSName                         [2]     IA5String,
 *      x400Address                     [3]     ORAddress,
 *      directoryName                   [4]     Name,
 *      ediPartyName                    [5]     EDIPartyName,
 *      uniformResourceIdentifier       [6]     IA5String,
 *      iPAddress                       [7]     OCTET STRING,
 *      registeredID                    [8]     OBJECT IDENTIFIER
 * }
 */
const SEC_ASN1Template GeneralNameTemplate[] =
{
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(GeneralName)},
    {SEC_ASN1_INLINE, offsetof(GeneralName, name), CERT_NameTemplate, 0},
    {0, 0, nullptr, 0}
};

/**
 * GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
 */
const SEC_ASN1Template GeneralNamesTemplate[] =
{
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(GeneralNames)},
    {SEC_ASN1_INLINE | SEC_ASN1_CONTEXT_SPECIFIC | 4, offsetof(GeneralNames, names), GeneralNameTemplate, 0},
    {0, 0, nullptr, 0}
};

/**
 * IssuerSerial ::= SEQUENCE {
 *     issuer GeneralNames,
 *     serialNumber CertificateSerialNumber
 * }
 */
const SEC_ASN1Template IssuerSerialTemplate[] =
{
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(IssuerSerial)},
    {SEC_ASN1_INLINE, offsetof(IssuerSerial, issuer), GeneralNamesTemplate, 0},
    {SEC_ASN1_INTEGER, offsetof(IssuerSerial, serialNumber), nullptr, 0},
    {0, 0, nullptr, 0}
};


/**
 * Hash ::= OCTET STRING
 *
 * ESSCertIDv2 ::= SEQUENCE {
 *     hashAlgorithm AlgorithmIdentifier DEFAULT {algorithm id-sha256},
 *     certHash Hash,
 *     issuerSerial IssuerSerial OPTIONAL
 * }
 */

SEC_ASN1_MKSUB(SECOID_AlgorithmIDTemplate)

const SEC_ASN1Template ESSCertIDv2Template[] =
{
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(ESSCertIDv2)},
    {SEC_ASN1_INLINE | SEC_ASN1_XTRN, offsetof(ESSCertIDv2, hashAlgorithm), SEC_ASN1_SUB(SECOID_AlgorithmIDTemplate), 0},
    {SEC_ASN1_OCTET_STRING, offsetof(ESSCertIDv2, certHash), nullptr, 0},
    {SEC_ASN1_INLINE | SEC_ASN1_XTRN, offsetof(ESSCertIDv2, issuerSerial), IssuerSerialTemplate, 0},
    {0, 0, nullptr, 0}
};

/**
 * SigningCertificateV2 ::= SEQUENCE {
 * }
 */
const SEC_ASN1Template SigningCertificateV2Template[] =
{
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(SigningCertificateV2)},
    {SEC_ASN1_SEQUENCE_OF, offsetof(SigningCertificateV2, certs), ESSCertIDv2Template, 0},
    {0, 0, nullptr, 0}
};

struct PKIStatusInfo {
    SECItem status;
    SECItem statusString;
    SECItem failInfo;
};

const SEC_ASN1Template PKIStatusInfo_Template[] =
{
    { SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(PKIStatusInfo) },
    { SEC_ASN1_INTEGER, offsetof(PKIStatusInfo, status), nullptr, 0 },
    { SEC_ASN1_CONSTRUCTED | SEC_ASN1_SEQUENCE | SEC_ASN1_OPTIONAL, offsetof(PKIStatusInfo, statusString), nullptr, 0 },
    { SEC_ASN1_BIT_STRING | SEC_ASN1_OPTIONAL, offsetof(PKIStatusInfo, failInfo), nullptr, 0 },
    { 0, 0, nullptr, 0 }
};

const SEC_ASN1Template Any_Template[] =
{
    { SEC_ASN1_ANY, 0, nullptr, sizeof(SECItem) }
};

struct TimeStampResp {
    PKIStatusInfo status;
    SECItem timeStampToken;
};

const SEC_ASN1Template TimeStampResp_Template[] =
{
    { SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(TimeStampResp) },
    { SEC_ASN1_INLINE, offsetof(TimeStampResp, status), PKIStatusInfo_Template, 0 },
    { SEC_ASN1_ANY | SEC_ASN1_OPTIONAL, offsetof(TimeStampResp, timeStampToken), Any_Template, 0 },
    { 0, 0, nullptr, 0 }
};

const SEC_ASN1Template MessageImprint_Template[] =
{
    { SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(MessageImprint) },
    { SEC_ASN1_INLINE, offsetof(MessageImprint, hashAlgorithm), SECOID_AlgorithmIDTemplate, 0 },
    { SEC_ASN1_OCTET_STRING, offsetof(MessageImprint, hashedMessage), nullptr, 0 },
    { 0, 0, nullptr, 0 }
};

const SEC_ASN1Template Extension_Template[] =
{
    { SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(Extension) },
    { SEC_ASN1_OBJECT_ID, offsetof(Extension, extnID), nullptr, 0 },
    { SEC_ASN1_BOOLEAN, offsetof(Extension, critical), nullptr, 0 },
    { SEC_ASN1_OCTET_STRING, offsetof(Extension, extnValue), nullptr, 0 },
    { 0, 0, nullptr, 0 }
};

const SEC_ASN1Template Extensions_Template[] =
{
    { SEC_ASN1_SEQUENCE_OF, 0, Extension_Template, 0 }
};

const SEC_ASN1Template TimeStampReq_Template[] =
{
    { SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(TimeStampReq) },
    { SEC_ASN1_INTEGER, offsetof(TimeStampReq, version), nullptr, 0 },
    { SEC_ASN1_INLINE, offsetof(TimeStampReq, messageImprint), MessageImprint_Template, 0 },
    { SEC_ASN1_OBJECT_ID | SEC_ASN1_OPTIONAL, offsetof(TimeStampReq, reqPolicy), nullptr, 0 },
    { SEC_ASN1_INTEGER | SEC_ASN1_OPTIONAL, offsetof(TimeStampReq, nonce), nullptr, 0 },
    { SEC_ASN1_BOOLEAN | SEC_ASN1_OPTIONAL, offsetof(TimeStampReq, certReq), nullptr, 0 },
    { SEC_ASN1_OPTIONAL | SEC_ASN1_CONTEXT_SPECIFIC | 0, offsetof(TimeStampReq, extensions), Extensions_Template, 0 },
    { 0, 0, nullptr, 0 }
};

size_t AppendToBuffer(char const *ptr, size_t size, size_t nmemb, void *userdata)
{
    OStringBuffer *pBuffer = static_cast<OStringBuffer*>(userdata);
    pBuffer->append(ptr, size*nmemb);

    return size*nmemb;
}

OUString PKIStatusToString(int n)
{
    switch (n)
    {
    case 0: return OUString("granted");
    case 1: return OUString("grantedWithMods");
    case 2: return OUString("rejection");
    case 3: return OUString("waiting");
    case 4: return OUString("revocationWarning");
    case 5: return OUString("revocationNotification");
    default: return "unknown (" + OUString::number(n) + ")";
    }
}

OUString PKIStatusInfoToString(const PKIStatusInfo& rStatusInfo)
{
    OUString result;

    result += "{status=";
    if (rStatusInfo.status.len == 1)
        result += PKIStatusToString(rStatusInfo.status.data[0]);
    else
        result += "unknown (len=" + OUString::number(rStatusInfo.status.len);

    // FIXME: Perhaps look at rStatusInfo.statusString.data but note
    // that we of course can't assume it contains proper UTF-8. After
    // all, it is data from an external source. Also, RFC3161 claims
    // it should be a SEQUENCE (1..MAX) OF UTF8String, but another
    // source claimed it would be a single UTF8String, hmm?

    // FIXME: Worth it to decode failInfo to cleartext, probably not at least as long as this is only for a SAL_INFO

    result += "}";

    return result;
}

// SEC_StringToOID() and NSS_CMSSignerInfo_AddUnauthAttr() are
// not exported from libsmime, so copy them here. Sigh.

SECStatus
my_SEC_StringToOID(SECItem *to, const char *from, PRUint32 len)
{
    PRUint32 decimal_numbers = 0;
    PRUint32 result_bytes = 0;
    SECStatus rv;
    PRUint8 result[1024];

    static const PRUint32 max_decimal = 0xffffffff / 10;
    static const char OIDstring[] = {"OID."};

    if (!from || !to) {
        PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }
    if (!len) {
        len = PL_strlen(from);
    }
    if (len >= 4 && !PL_strncasecmp(from, OIDstring, 4)) {
        from += 4; /* skip leading "OID." if present */
        len  -= 4;
    }
    if (!len) {
bad_data:
        PORT_SetError(SEC_ERROR_BAD_DATA);
        return SECFailure;
    }
    do {
        PRUint32 decimal = 0;
        while (len > 0 && rtl::isAsciiDigit(static_cast<unsigned char>(*from))) {
            PRUint32 addend = *from++ - '0';
            --len;
            if (decimal > max_decimal)  /* overflow */
                goto bad_data;
            decimal = (decimal * 10) + addend;
            if (decimal < addend)   /* overflow */
                goto bad_data;
        }
        if (len != 0 && *from != '.') {
            goto bad_data;
        }
        if (decimal_numbers == 0) {
            if (decimal > 2)
                goto bad_data;
            result[0] = decimal * 40;
            result_bytes = 1;
        } else if (decimal_numbers == 1) {
            if (decimal > 40)
                goto bad_data;
            result[0] += decimal;
        } else {
            /* encode the decimal number,  */
            PRUint8 * rp;
            PRUint32 num_bytes = 0;
            PRUint32 tmp = decimal;
            while (tmp) {
                num_bytes++;
                tmp >>= 7;
            }
            if (!num_bytes )
                ++num_bytes;  /* use one byte for a zero value */
            if (num_bytes + result_bytes > sizeof result)
                goto bad_data;
            tmp = num_bytes;
            rp = result + result_bytes - 1;
            rp[tmp] = static_cast<PRUint8>(decimal & 0x7f);
            decimal >>= 7;
            while (--tmp > 0) {
                rp[tmp] = static_cast<PRUint8>(decimal | 0x80);
                decimal >>= 7;
            }
            result_bytes += num_bytes;
        }
        ++decimal_numbers;
        if (len > 0) { /* skip trailing '.' */
            ++from;
            --len;
        }
    } while (len > 0);
    /* now result contains result_bytes of data */
    if (to->data && to->len >= result_bytes) {
        to->len = result_bytes;
        PORT_Memcpy(to->data, result, to->len);
        rv = SECSuccess;
    } else {
        SECItem result_item = {siBuffer, nullptr, 0 };
        result_item.data = result;
        result_item.len  = result_bytes;
        rv = SECITEM_CopyItem(nullptr, to, &result_item);
    }
    return rv;
}

NSSCMSAttribute *
my_NSS_CMSAttributeArray_FindAttrByOidTag(NSSCMSAttribute **attrs, SECOidTag oidtag, PRBool only)
{
    SECOidData *oid;
    NSSCMSAttribute *attr1, *attr2;

    if (attrs == nullptr)
        return nullptr;

    oid = SECOID_FindOIDByTag(oidtag);
    if (oid == nullptr)
        return nullptr;

    while ((attr1 = *attrs++) != nullptr) {
    if (attr1->type.len == oid->oid.len && PORT_Memcmp (attr1->type.data,
                                oid->oid.data,
                                oid->oid.len) == 0)
        break;
    }

    if (attr1 == nullptr)
        return nullptr;

    if (!only)
        return attr1;

    while ((attr2 = *attrs++) != nullptr) {
    if (attr2->type.len == oid->oid.len && PORT_Memcmp (attr2->type.data,
                                oid->oid.data,
                                oid->oid.len) == 0)
        break;
    }

    if (attr2 != nullptr)
        return nullptr;

    return attr1;
}

SECStatus
my_NSS_CMSArray_Add(PLArenaPool *poolp, void ***array, void *obj)
{
    int n = 0;
    void **dest;

    PORT_Assert(array != NULL);
    if (array == nullptr)
        return SECFailure;

    if (*array == nullptr) {
        dest = static_cast<void **>(PORT_ArenaAlloc(poolp, 2 * sizeof(void *)));
    } else {
        void **p = *array;
        while (*p++)
            n++;
        dest = static_cast<void **>(PORT_ArenaGrow (poolp,
                      *array,
                      (n + 1) * sizeof(void *),
                      (n + 2) * sizeof(void *)));
    }

    if (dest == nullptr)
        return SECFailure;

    dest[n] = obj;
    dest[n+1] = nullptr;
    *array = dest;
    return SECSuccess;
}

SECOidTag
my_NSS_CMSAttribute_GetType(NSSCMSAttribute *attr)
{
    SECOidData *typetag;

    typetag = SECOID_FindOID(&(attr->type));
    if (typetag == nullptr)
        return SEC_OID_UNKNOWN;

    return typetag->offset;
}

SECStatus
my_NSS_CMSAttributeArray_AddAttr(PLArenaPool *poolp, NSSCMSAttribute ***attrs, NSSCMSAttribute *attr)
{
    NSSCMSAttribute *oattr;
    void *mark;
    SECOidTag type;

    mark = PORT_ArenaMark(poolp);

    /* find oidtag of attr */
    type = my_NSS_CMSAttribute_GetType(attr);

    /* see if we have one already */
    oattr = my_NSS_CMSAttributeArray_FindAttrByOidTag(*attrs, type, PR_FALSE);
    PORT_Assert (oattr == NULL);
    if (oattr != nullptr)
        goto loser; /* XXX or would it be better to replace it? */

    /* no, shove it in */
    if (my_NSS_CMSArray_Add(poolp, reinterpret_cast<void ***>(attrs), static_cast<void *>(attr)) != SECSuccess)
        goto loser;

    PORT_ArenaUnmark(poolp, mark);
    return SECSuccess;

loser:
    PORT_ArenaRelease(poolp, mark);
    return SECFailure;
}

SECStatus
my_NSS_CMSSignerInfo_AddUnauthAttr(NSSCMSSignerInfo *signerinfo, NSSCMSAttribute *attr)
{
    return my_NSS_CMSAttributeArray_AddAttr(signerinfo->cmsg->poolp, &(signerinfo->unAuthAttr), attr);
}

SECStatus
my_NSS_CMSSignerInfo_AddAuthAttr(NSSCMSSignerInfo *signerinfo, NSSCMSAttribute *attr)
{
    return my_NSS_CMSAttributeArray_AddAttr(signerinfo->cmsg->poolp, &(signerinfo->authAttr), attr);
}

NSSCMSMessage *CreateCMSMessage(const PRTime* time,
                                NSSCMSSignedData **cms_sd,
                                NSSCMSSignerInfo **cms_signer,
                                CERTCertificate *cert,
                                SECItem *digest)
{
    NSSCMSMessage *result = NSS_CMSMessage_Create(nullptr);
    if (!result)
    {
        SAL_WARN("svl.crypto", "NSS_CMSMessage_Create failed");
        return nullptr;
    }

    *cms_sd = NSS_CMSSignedData_Create(result);
    if (!*cms_sd)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignedData_Create failed");
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    NSSCMSContentInfo *cms_cinfo = NSS_CMSMessage_GetContentInfo(result);
    if (NSS_CMSContentInfo_SetContent_SignedData(result, cms_cinfo, *cms_sd) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSContentInfo_SetContent_SignedData failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    cms_cinfo = NSS_CMSSignedData_GetContentInfo(*cms_sd);

    // Attach NULL data as detached data
    if (NSS_CMSContentInfo_SetContent_Data(result, cms_cinfo, nullptr, PR_TRUE) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSContentInfo_SetContent_Data failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    *cms_signer = NSS_CMSSignerInfo_Create(result, cert, SEC_OID_SHA256);
    if (!*cms_signer)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignerInfo_Create failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (time && NSS_CMSSignerInfo_AddSigningTime(*cms_signer, *time) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignerInfo_AddSigningTime failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignerInfo_IncludeCerts(*cms_signer, NSSCMSCM_CertChain, certUsageEmailSigner) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignerInfo_IncludeCerts failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_AddCertificate(*cms_sd, cert) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignedData_AddCertificate failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_AddSignerInfo(*cms_sd, *cms_signer) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignedData_AddSignerInfo failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_SetDigestValue(*cms_sd, SEC_OID_SHA256, digest) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSSignedData_SetDigestValue failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    return result;
}

#endif // HAVE_FEATURE_NSS && !_WIN32

} // Anonymous namespace

#ifdef _WIN32
namespace
{

/// Counts how many bytes are needed to encode a given length.
size_t GetDERLengthOfLength(size_t nLength)
{
    size_t nRet = 1;

    if(nLength > 127)
    {
        while (nLength >> (nRet * 8))
            ++nRet;
        // Long form means one additional byte: the length of the length and
        // the length itself.
        ++nRet;
    }
    return nRet;
}

/// Writes the length part of the header.
void WriteDERLength(SvStream& rStream, size_t nLength)
{
    size_t nLengthOfLength = GetDERLengthOfLength(nLength);
    if (nLengthOfLength == 1)
    {
        // We can use the short form.
        rStream.WriteUInt8(nLength);
        return;
    }

    // 0x80 means that the we use the long form: the first byte is the length
    // of length with the highest bit set to 1, not the actual length.
    rStream.WriteUInt8(0x80 | (nLengthOfLength - 1));
    for (size_t i = 1; i < nLengthOfLength; ++i)
        rStream.WriteUInt8(nLength >> ((nLengthOfLength - i - 1) * 8));
}

const unsigned nASN1_INTEGER = 0x02;
const unsigned nASN1_OCTET_STRING = 0x04;
const unsigned nASN1_NULL = 0x05;
const unsigned nASN1_OBJECT_IDENTIFIER = 0x06;
const unsigned nASN1_SEQUENCE = 0x10;
/// An explicit tag on a constructed value.
const unsigned nASN1_TAGGED_CONSTRUCTED = 0xa0;
const unsigned nASN1_CONSTRUCTED = 0x20;

/// Create payload for the 'signing-certificate' signed attribute.
bool CreateSigningCertificateAttribute(void const * pDerEncoded, int nDerEncoded, PCCERT_CONTEXT pCertContext, SvStream& rEncodedCertificate)
{
    // CryptEncodeObjectEx() does not support encoding arbitrary ASN.1
    // structures, like SigningCertificateV2 from RFC 5035, so let's build it
    // manually.

    // Count the certificate hash and put it to aHash.
    // 2.16.840.1.101.3.4.2.1, i.e. sha256.
    std::vector<unsigned char> aSHA256{0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01};

    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContextW(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        SAL_WARN("svl.crypto", "CryptAcquireContext() failed");
        return false;
    }

    HCRYPTHASH hHash = 0;
    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
    {
        SAL_WARN("svl.crypto", "CryptCreateHash() failed");
        return false;
    }

    if (!CryptHashData(hHash, static_cast<const BYTE*>(pDerEncoded), nDerEncoded, 0))
    {
        SAL_WARN("svl.crypto", "CryptHashData() failed");
        return false;
    }

    DWORD nHash = 0;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, nullptr, &nHash, 0))
    {
        SAL_WARN("svl.crypto", "CryptGetHashParam() failed to provide the hash length");
        return false;
    }

    std::vector<unsigned char> aHash(nHash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, aHash.data(), &nHash, 0))
    {
        SAL_WARN("svl.crypto", "CryptGetHashParam() failed to provide the hash");
        return false;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    // Collect info for IssuerSerial.
    BYTE* pIssuer = pCertContext->pCertInfo->Issuer.pbData;
    DWORD nIssuer = pCertContext->pCertInfo->Issuer.cbData;
    BYTE* pSerial = pCertContext->pCertInfo->SerialNumber.pbData;
    DWORD nSerial = pCertContext->pCertInfo->SerialNumber.cbData;
    // pSerial is LE, aSerial is BE.
    std::vector<BYTE> aSerial(nSerial);
    for (size_t i = 0; i < nSerial; ++i)
        aSerial[i] = *(pSerial + nSerial - i - 1);

    // We now have all the info to count the lengths.
    // The layout of the payload is:
    // SEQUENCE: SigningCertificateV2
    //     SEQUENCE: SEQUENCE OF ESSCertIDv2
    //         SEQUENCE: ESSCertIDv2
    //             SEQUENCE: AlgorithmIdentifier
    //                 OBJECT: algorithm
    //                 NULL: parameters
    //             OCTET STRING: certHash
    //             SEQUENCE: IssuerSerial
    //                 SEQUENCE: GeneralNames
    //                     cont [ 4 ]: Name
    //                         SEQUENCE: Issuer blob
    //                 INTEGER: CertificateSerialNumber

    size_t nAlgorithm = 1 + GetDERLengthOfLength(aSHA256.size()) + aSHA256.size();
    size_t nParameters = 1 + GetDERLengthOfLength(1);
    size_t nAlgorithmIdentifier = 1 + GetDERLengthOfLength(nAlgorithm + nParameters) + nAlgorithm + nParameters;
    size_t nCertHash = 1 + GetDERLengthOfLength(aHash.size()) + aHash.size();
    size_t nName = 1 + GetDERLengthOfLength(nIssuer) + nIssuer;
    size_t nGeneralNames = 1 + GetDERLengthOfLength(nName) + nName;
    size_t nCertificateSerialNumber = 1 + GetDERLengthOfLength(nSerial) + nSerial;
    size_t nIssuerSerial = 1 + GetDERLengthOfLength(nGeneralNames + nCertificateSerialNumber) + nGeneralNames + nCertificateSerialNumber;
    size_t nESSCertIDv2 = 1 + GetDERLengthOfLength(nAlgorithmIdentifier + nCertHash + nIssuerSerial) + nAlgorithmIdentifier + nCertHash + nIssuerSerial;
    size_t nESSCertIDv2s = 1 + GetDERLengthOfLength(nESSCertIDv2) + nESSCertIDv2;

    // Write SigningCertificateV2.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nESSCertIDv2s);
    // Write SEQUENCE OF ESSCertIDv2.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nESSCertIDv2);
    // Write ESSCertIDv2.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nAlgorithmIdentifier + nCertHash + nIssuerSerial);
    // Write AlgorithmIdentifier.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nAlgorithm + nParameters);
    // Write algorithm.
    rEncodedCertificate.WriteUInt8(nASN1_OBJECT_IDENTIFIER);
    WriteDERLength(rEncodedCertificate, aSHA256.size());
    rEncodedCertificate.WriteBytes(aSHA256.data(), aSHA256.size());
    // Write parameters.
    rEncodedCertificate.WriteUInt8(nASN1_NULL);
    rEncodedCertificate.WriteUInt8(0);
    // Write certHash.
    rEncodedCertificate.WriteUInt8(nASN1_OCTET_STRING);
    WriteDERLength(rEncodedCertificate, aHash.size());
    rEncodedCertificate.WriteBytes(aHash.data(), aHash.size());
    // Write IssuerSerial.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nGeneralNames + nCertificateSerialNumber);
    // Write GeneralNames.
    rEncodedCertificate.WriteUInt8(nASN1_SEQUENCE | nASN1_CONSTRUCTED);
    WriteDERLength(rEncodedCertificate, nName);
    // Write Name.
    rEncodedCertificate.WriteUInt8(nASN1_TAGGED_CONSTRUCTED | 4);
    WriteDERLength(rEncodedCertificate, nIssuer);
    rEncodedCertificate.WriteBytes(pIssuer, nIssuer);
    // Write CertificateSerialNumber.
    rEncodedCertificate.WriteUInt8(nASN1_INTEGER);
    WriteDERLength(rEncodedCertificate, nSerial);
    rEncodedCertificate.WriteBytes(aSerial.data(), aSerial.size());

    return true;
}
} // anonymous namespace
#endif //_WIN32

namespace svl {

namespace crypto {

static int AsHex(char ch)
{
    int nRet = 0;
    if (rtl::isAsciiDigit(static_cast<unsigned char>(ch)))
        nRet = ch - '0';
    else
    {
        if (ch >= 'a' && ch <= 'f')
            nRet = ch - 'a';
        else if (ch >= 'A' && ch <= 'F')
            nRet = ch - 'A';
        else
            return -1;
        nRet += 10;
    }
    return nRet;
}

std::vector<unsigned char> DecodeHexString(const OString& rHex)
{
    std::vector<unsigned char> aRet;
    size_t nHexLen = rHex.getLength();
    {
        int nByte = 0;
        int nCount = 2;
        for (size_t i = 0; i < nHexLen; ++i)
        {
            nByte = nByte << 4;
            sal_Int8 nParsed = AsHex(rHex[i]);
            if (nParsed == -1)
            {
                SAL_WARN("svl.crypto", "DecodeHexString: invalid hex value");
                return aRet;
            }
            nByte += nParsed;
            --nCount;
            if (!nCount)
            {
                aRet.push_back(nByte);
                nCount = 2;
                nByte = 0;
            }
        }
    }

    return aRet;
}


#if defined(SVL_CRYPTO_NSS) || defined(SVL_CRYPTO_MSCRYPTO)

bool Signing::Sign(OStringBuffer& rCMSHexBuffer)
{
    // Create the PKCS#7 object.
    css::uno::Sequence<sal_Int8> aDerEncoded = m_xCertificate->getEncoded();
    if (!aDerEncoded.hasElements())
    {
        SAL_WARN("svl.crypto", "Crypto::Signing: empty certificate");
        return false;
    }

#ifndef _WIN32

    CERTCertificate *cert = CERT_DecodeCertFromPackage(reinterpret_cast<char *>(aDerEncoded.getArray()), aDerEncoded.getLength());

    if (!cert)
    {
        SAL_WARN("svl.crypto", "CERT_DecodeCertFromPackage failed");
        return false;
    }

    std::vector<unsigned char> aHashResult;
    {
        comphelper::Hash aHash(comphelper::HashType::SHA256);

        for (const auto& pair : m_dataBlocks)
            aHash.update(static_cast<const unsigned char*>(pair.first), pair.second);

        aHashResult = aHash.finalize();
    }
    SECItem digest;
    digest.data = aHashResult.data();
    digest.len = aHashResult.size();

#ifdef DBG_UTIL
    {
        FILE *out = fopen("PDFWRITER.hash.data", "wb");
        fwrite(aHashResult.data(), SHA256_LENGTH, 1, out);
        fclose(out);
    }
#endif

    PRTime now = PR_Now();
    NSSCMSSignedData *cms_sd;
    NSSCMSSignerInfo *cms_signer;
    NSSCMSMessage *cms_msg = CreateCMSMessage(nullptr, &cms_sd, &cms_signer, cert, &digest);
    if (!cms_msg)
        return false;

    OString pass(OUStringToOString( m_aSignPassword, RTL_TEXTENCODING_UTF8 ));

    TimeStampReq src;
    OStringBuffer response_buffer;
    TimeStampResp response;
    SECItem response_item;
    NSSCMSAttribute timestamp;
    SECItem values[2];
    SECItem *valuesp[2];
    valuesp[0] = values;
    valuesp[1] = nullptr;
    SECOidData typetag;

    if( !m_aSignTSA.isEmpty() )
    {
        // Create another CMS message with the same contents as cms_msg, because it doesn't seem
        // possible to encode a message twice (once to get something to timestamp, and then after
        // adding the timestamp attribute).

        NSSCMSSignedData *ts_cms_sd;
        NSSCMSSignerInfo *ts_cms_signer;
        NSSCMSMessage *ts_cms_msg = CreateCMSMessage(&now, &ts_cms_sd, &ts_cms_signer, cert, &digest);
        if (!ts_cms_msg)
        {
            return false;
        }

        SECItem ts_cms_output;
        ts_cms_output.data = nullptr;
        ts_cms_output.len = 0;
        PLArenaPool *ts_arena = PORT_NewArena(10000);
        NSSCMSEncoderContext *ts_cms_ecx;
        ts_cms_ecx = NSS_CMSEncoder_Start(ts_cms_msg, nullptr, nullptr, &ts_cms_output, ts_arena, PDFSigningPKCS7PasswordCallback,
                                          const_cast<sal_Char*>(pass.getStr()), nullptr, nullptr, nullptr, nullptr);

        if (NSS_CMSEncoder_Finish(ts_cms_ecx) != SECSuccess)
        {
            SAL_WARN("svl.crypto", "NSS_CMSEncoder_Finish failed");
            return false;
        }

        // I have compared the ts_cms_output produced here with the cms_output produced below, with
        // the DONTCALLADDUNAUTHATTR env var set (i.e. without actually calling
        // my_NSS_CMSSignerInfo_AddUnauthAttr()), and they are identical.

#ifdef DBG_UTIL
        {
            FILE *out = fopen("PDFWRITER.ts_cms.data", "wb");
            fwrite(ts_cms_output.data, ts_cms_output.len, 1, out);
            fclose(out);
        }
#endif

        std::vector<unsigned char> aTsHashResult = comphelper::Hash::calculateHash(ts_cms_signer->encDigest.data, ts_cms_signer->encDigest.len, comphelper::HashType::SHA256);
        SECItem ts_digest;
        ts_digest.type = siBuffer;
        ts_digest.data = aTsHashResult.data();
        ts_digest.len = aTsHashResult.size();

#ifdef DBG_UTIL
        {
            FILE *out = fopen("PDFWRITER.ts_hash.data", "wb");
            fwrite(aTsHashResult.data(), SHA256_LENGTH, 1, out);
            fclose(out);
        }
#endif

        unsigned char cOne = 1;
        src.version.type = siUnsignedInteger;
        src.version.data = &cOne;
        src.version.len = sizeof(cOne);

        src.messageImprint.hashAlgorithm.algorithm.data = nullptr;
        src.messageImprint.hashAlgorithm.parameters.data = nullptr;
        SECOID_SetAlgorithmID(nullptr, &src.messageImprint.hashAlgorithm, SEC_OID_SHA256, nullptr);
        src.messageImprint.hashedMessage = ts_digest;

        src.reqPolicy.type = siBuffer;
        src.reqPolicy.data = nullptr;
        src.reqPolicy.len = 0;

        unsigned int nNonce = comphelper::rng::uniform_uint_distribution(0, SAL_MAX_UINT32);
        src.nonce.type = siUnsignedInteger;
        src.nonce.data = reinterpret_cast<unsigned char*>(&nNonce);
        src.nonce.len = sizeof(nNonce);

        src.certReq.type = siUnsignedInteger;
        src.certReq.data = &cOne;
        src.certReq.len = sizeof(cOne);

        src.extensions = nullptr;

        SECItem* timestamp_request = SEC_ASN1EncodeItem(nullptr, nullptr, &src, TimeStampReq_Template);
        if (timestamp_request == nullptr)
        {
            SAL_WARN("svl.crypto", "SEC_ASN1EncodeItem failed");
            return false;
        }

        if (timestamp_request->data == nullptr)
        {
            SAL_WARN("svl.crypto", "SEC_ASN1EncodeItem succeeded but got NULL data");
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("svl.crypto", "request length=" << timestamp_request->len);

#ifdef DBG_UTIL
        {
            FILE *out = fopen("PDFWRITER.timestampreq.data", "wb");
            fwrite(timestamp_request->data, timestamp_request->len, 1, out);
            fclose(out);
        }
#endif

        // Send time stamp request to TSA server, receive response

        CURL* curl = curl_easy_init();
        CURLcode rc;
        struct curl_slist* slist = nullptr;

        if (!curl)
        {
            SAL_WARN("svl.crypto", "curl_easy_init failed");
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("svl.crypto", "Setting curl to verbose: " << (curl_easy_setopt(curl, CURLOPT_VERBOSE, 1) == CURLE_OK ? "OK" : "FAIL"));

        if ((rc = curl_easy_setopt(curl, CURLOPT_URL, OUStringToOString(m_aSignTSA, RTL_TEXTENCODING_UTF8).getStr())) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_URL) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        slist = curl_slist_append(slist, "Content-Type: application/timestamp-query");
        slist = curl_slist_append(slist, "Accept: application/timestamp-reply");

        if ((rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_HTTPHEADER) failed: " << curl_easy_strerror(rc));
            curl_slist_free_all(slist);
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(timestamp_request->len))) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, timestamp_request->data)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_POSTFIELDSIZE or CURLOPT_POSTFIELDS) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer)) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AppendToBuffer)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_WRITEDATA or CURLOPT_WRITEFUNCTION) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_POST, 1)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_POST) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        char error_buffer[CURL_ERROR_SIZE];
        if ((rc = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_ERRORBUFFER) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        // Use a ten second timeout
        if ((rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10)) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10)) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_setopt(CURLOPT_TIMEOUT or CURLOPT_CONNECTTIMEOUT) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if (curl_easy_perform(curl) != CURLE_OK)
        {
            SAL_WARN("svl.crypto", "curl_easy_perform failed: " << error_buffer);
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("svl.crypto", "PDF signing: got response, length=" << response_buffer.getLength());

#ifdef DBG_UTIL
        {
            FILE *out = fopen("PDFWRITER.reply.data", "wb");
            fwrite(response_buffer.getStr(), response_buffer.getLength(), 1, out);
            fclose(out);
        }
#endif

        curl_slist_free_all(slist);
        curl_easy_cleanup(curl);
        SECITEM_FreeItem(timestamp_request, PR_TRUE);

        memset(&response, 0, sizeof(response));

        response_item.type = siBuffer;
        response_item.data = reinterpret_cast<unsigned char*>(const_cast<char*>(response_buffer.getStr()));
        response_item.len = response_buffer.getLength();

        if (SEC_ASN1DecodeItem(nullptr, &response, TimeStampResp_Template, &response_item) != SECSuccess)
        {
            SAL_WARN("svl.crypto", "SEC_ASN1DecodeItem failed");
            return false;
        }

        SAL_INFO("svl.crypto", "TimeStampResp received and decoded, status=" << PKIStatusInfoToString(response.status));

        if (response.status.status.len != 1 ||
            (response.status.status.data[0] != 0 && response.status.status.data[0] != 1))
        {
            SAL_WARN("svl.crypto", "Timestamp request was not granted");
            return false;
        }

        // timestamp.type filled in below

        // Not sure if we actually need two entries in the values array, now when valuesp is an
        // array too, the pointer to the values array followed by a null pointer. But I don't feel
        // like experimenting.
        values[0] = response.timeStampToken;
        values[1].type = siBuffer;
        values[1].data = nullptr;
        values[1].len = 0;

        timestamp.values = valuesp;

        typetag.oid.data = nullptr;
        // id-aa-timeStampToken OBJECT IDENTIFIER ::= { iso(1)
        // member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs-9(9)
        // smime(16) aa(2) 14 }
        if (my_SEC_StringToOID(&typetag.oid, "1.2.840.113549.1.9.16.2.14", 0) != SECSuccess)
        {
            SAL_WARN("svl.crypto", "SEC_StringToOID failed");
            return false;
        }
        typetag.offset = SEC_OID_UNKNOWN; // ???
        typetag.desc = "id-aa-timeStampToken";
        typetag.mechanism = CKM_SHA_1; // ???
        typetag.supportedExtension = UNSUPPORTED_CERT_EXTENSION; // ???
        timestamp.typeTag = &typetag;

        timestamp.type = typetag.oid; // ???

        timestamp.encoded = PR_TRUE; // ???

#ifdef DBG_UTIL
        if (getenv("DONTCALLADDUNAUTHATTR"))
            ;
        else
#endif
        if (my_NSS_CMSSignerInfo_AddUnauthAttr(cms_signer, &timestamp) != SECSuccess)
        {
            SAL_WARN("svl.crypto", "NSS_CMSSignerInfo_AddUnauthAttr failed");
            return false;
        }
    }

    // Add the signing certificate as a signed attribute.
    ESSCertIDv2* aCertIDs[2];
    ESSCertIDv2 aCertID;
    // Write ESSCertIDv2.hashAlgorithm.
    aCertID.hashAlgorithm.algorithm.data = nullptr;
    aCertID.hashAlgorithm.parameters.data = nullptr;
    SECOID_SetAlgorithmID(nullptr, &aCertID.hashAlgorithm, SEC_OID_SHA256, nullptr);
    // Write ESSCertIDv2.certHash.
    SECItem aCertHashItem;
    auto pDerEncoded = reinterpret_cast<const unsigned char *>(aDerEncoded.getArray());
    std::vector<unsigned char> aCertHashResult = comphelper::Hash::calculateHash(pDerEncoded, aDerEncoded.getLength(), comphelper::HashType::SHA256);
    aCertHashItem.type = siBuffer;
    aCertHashItem.data = aCertHashResult.data();
    aCertHashItem.len = aCertHashResult.size();
    aCertID.certHash = aCertHashItem;
    // Write ESSCertIDv2.issuerSerial.
    IssuerSerial aSerial;
    GeneralName aName;
    aName.name = cert->issuer;
    aSerial.issuer.names = aName;
    aSerial.serialNumber = cert->serialNumber;
    aCertID.issuerSerial = aSerial;
    // Write SigningCertificateV2.certs.
    aCertIDs[0] = &aCertID;
    aCertIDs[1] = nullptr;
    SigningCertificateV2 aCertificate;
    aCertificate.certs = &aCertIDs[0];
    SECItem* pEncodedCertificate = SEC_ASN1EncodeItem(nullptr, nullptr, &aCertificate, SigningCertificateV2Template);
    if (!pEncodedCertificate)
    {
        SAL_WARN("svl.crypto", "SEC_ASN1EncodeItem() failed");
        return false;
    }

    NSSCMSAttribute aAttribute;
    SECItem aAttributeValues[2];
    SECItem* pAttributeValues[2];
    pAttributeValues[0] = aAttributeValues;
    pAttributeValues[1] = nullptr;
    aAttributeValues[0] = *pEncodedCertificate;
    aAttributeValues[1].type = siBuffer;
    aAttributeValues[1].data = nullptr;
    aAttributeValues[1].len = 0;
    aAttribute.values = pAttributeValues;

    SECOidData aOidData;
    aOidData.oid.data = nullptr;
    /*
     * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
     * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
     *   smime(16) id-aa(2) 47 }
     */
    if (my_SEC_StringToOID(&aOidData.oid, "1.2.840.113549.1.9.16.2.47", 0) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "my_SEC_StringToOID() failed");
        return false;
    }
    aOidData.offset = SEC_OID_UNKNOWN;
    aOidData.desc = "id-aa-signingCertificateV2";
    aOidData.mechanism = CKM_SHA_1;
    aOidData.supportedExtension = UNSUPPORTED_CERT_EXTENSION;
    aAttribute.typeTag = &aOidData;
    aAttribute.type = aOidData.oid;
    aAttribute.encoded = PR_TRUE;

    if (my_NSS_CMSSignerInfo_AddAuthAttr(cms_signer, &aAttribute) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "my_NSS_CMSSignerInfo_AddAuthAttr() failed");
        return false;
    }

    SECItem cms_output;
    cms_output.data = nullptr;
    cms_output.len = 0;
    PLArenaPool *arena = PORT_NewArena(10000);
    NSSCMSEncoderContext *cms_ecx;

    // Possibly it would work to even just pass NULL for the password callback function and its
    // argument here. After all, at least with the hardware token and associated software I tested
    // with, the software itself pops up a dialog asking for the PIN (password). But I am not going
    // to test it and risk locking up my token...

    cms_ecx = NSS_CMSEncoder_Start(cms_msg, nullptr, nullptr, &cms_output, arena, PDFSigningPKCS7PasswordCallback,
                                   const_cast<sal_Char*>(pass.getStr()), nullptr, nullptr, nullptr, nullptr);

    if (!cms_ecx)
    {
        SAL_WARN("svl.crypto", "NSS_CMSEncoder_Start failed");
        return false;
    }

    if (NSS_CMSEncoder_Finish(cms_ecx) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "NSS_CMSEncoder_Finish failed");
        return false;
    }

#ifdef DBG_UTIL
    {
        FILE *out = fopen("PDFWRITER.cms.data", "wb");
        fwrite(cms_output.data, cms_output.len, 1, out);
        fclose(out);
    }
#endif

    if (cms_output.len*2 > MAX_SIGNATURE_CONTENT_LENGTH)
    {
        SAL_WARN("svl.crypto", "Signature requires more space (" << cms_output.len*2 << ") than we reserved (" << MAX_SIGNATURE_CONTENT_LENGTH << ")");
        NSS_CMSMessage_Destroy(cms_msg);
        return false;
    }

    for (unsigned int i = 0; i < cms_output.len ; i++)
        appendHex(cms_output.data[i], rCMSHexBuffer);

    SECITEM_FreeItem(pEncodedCertificate, PR_TRUE);
    NSS_CMSMessage_Destroy(cms_msg);

    return true;

#else // _WIN32
    PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, reinterpret_cast<const BYTE*>(aDerEncoded.getArray()), aDerEncoded.getLength());
    if (pCertContext == nullptr)
    {
        SAL_WARN("svl.crypto", "CertCreateCertificateContext failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    CRYPT_SIGN_MESSAGE_PARA aPara;

    memset(&aPara, 0, sizeof(aPara));
    aPara.cbSize = sizeof(aPara);
    aPara.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    aPara.pSigningCert = pCertContext;
    aPara.HashAlgorithm.pszObjId = const_cast<LPSTR>(szOID_NIST_sha256);
    aPara.HashAlgorithm.Parameters.cbData = 0;
    aPara.cMsgCert = 1;
    aPara.rgpMsgCert = &pCertContext;

    NCRYPT_KEY_HANDLE hCryptKey = 0;
    DWORD dwFlags = CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG;
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE* phCryptProvOrNCryptKey = &hCryptKey;
    DWORD nKeySpec;
    BOOL bFreeNeeded;

    if (!CryptAcquireCertificatePrivateKey(pCertContext,
                                           dwFlags,
                                           nullptr,
                                           phCryptProvOrNCryptKey,
                                           &nKeySpec,
                                           &bFreeNeeded))
    {
        SAL_WARN("svl.crypto", "CryptAcquireCertificatePrivateKey failed: " << WindowsErrorString(GetLastError()));
        CertFreeCertificateContext(pCertContext);
        return false;
    }
    assert(!bFreeNeeded);

    CMSG_SIGNER_ENCODE_INFO aSignerInfo;

    memset(&aSignerInfo, 0, sizeof(aSignerInfo));
    aSignerInfo.cbSize = sizeof(aSignerInfo);
    aSignerInfo.pCertInfo = pCertContext->pCertInfo;
    aSignerInfo.hNCryptKey = hCryptKey;
    aSignerInfo.dwKeySpec = nKeySpec;
    aSignerInfo.HashAlgorithm.pszObjId = const_cast<LPSTR>(szOID_NIST_sha256);
    aSignerInfo.HashAlgorithm.Parameters.cbData = 0;

    // Add the signing certificate as a signed attribute.
    CRYPT_INTEGER_BLOB aCertificateBlob;
    SvMemoryStream aEncodedCertificate;
    if (!CreateSigningCertificateAttribute(aDerEncoded.getArray(), aDerEncoded.getLength(), pCertContext, aEncodedCertificate))
    {
        SAL_WARN("svl.crypto", "CreateSigningCertificateAttribute() failed");
        return false;
    }
    aCertificateBlob.pbData = const_cast<BYTE*>(static_cast<const BYTE*>(aEncodedCertificate.GetData()));
    aCertificateBlob.cbData = aEncodedCertificate.GetSize();
    CRYPT_ATTRIBUTE aCertificateAttribute;
    /*
     * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
     * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
     *   smime(16) id-aa(2) 47 }
     */
    aCertificateAttribute.pszObjId = const_cast<LPSTR>("1.2.840.113549.1.9.16.2.47");
    aCertificateAttribute.cValue = 1;
    aCertificateAttribute.rgValue = &aCertificateBlob;
    aSignerInfo.cAuthAttr = 1;
    aSignerInfo.rgAuthAttr = &aCertificateAttribute;

    CMSG_SIGNED_ENCODE_INFO aSignedInfo;
    memset(&aSignedInfo, 0, sizeof(aSignedInfo));
    aSignedInfo.cbSize = sizeof(aSignedInfo);
    aSignedInfo.cSigners = 1;
    aSignedInfo.rgSigners = &aSignerInfo;

    CERT_BLOB aCertBlob;

    aCertBlob.cbData = pCertContext->cbCertEncoded;
    aCertBlob.pbData = pCertContext->pbCertEncoded;

    aSignedInfo.cCertEncoded = 1;
    aSignedInfo.rgCertEncoded = &aCertBlob;

    HCRYPTMSG hMsg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                          CMSG_DETACHED_FLAG,
                                          CMSG_SIGNED,
                                          &aSignedInfo,
                                          nullptr,
                                          nullptr);
    if (!hMsg)
    {
        SAL_WARN("svl.crypto", "CryptMsgOpenToEncode failed: " << WindowsErrorString(GetLastError()));
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    for (size_t i = 0; i < m_dataBlocks.size(); ++i)
    {
        const bool last = (i == m_dataBlocks.size() - 1);
        if (!CryptMsgUpdate(hMsg, static_cast<const BYTE *>(m_dataBlocks[i].first), m_dataBlocks[i].second, last))
        {
            SAL_WARN("svl.crypto", "CryptMsgUpdate failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }
    }

    PCRYPT_TIMESTAMP_CONTEXT pTsContext = nullptr;

    if( !m_aSignTSA.isEmpty() )
    {
        HCRYPTMSG hDecodedMsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                                     CMSG_DETACHED_FLAG,
                                                     CMSG_SIGNED,
                                                     NULL,
                                                     nullptr,
                                                     nullptr);
        if (!hDecodedMsg)
        {
            SAL_WARN("svl.crypto", "CryptMsgOpenToDecode failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        DWORD nTsSigLen = 0;

        if (!CryptMsgGetParam(hMsg, CMSG_BARE_CONTENT_PARAM, 0, nullptr, &nTsSigLen))
        {
            SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_BARE_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        SAL_INFO("svl.crypto", "nTsSigLen=" << nTsSigLen);

        std::unique_ptr<BYTE[]> pTsSig(new BYTE[nTsSigLen]);

        if (!CryptMsgGetParam(hMsg, CMSG_BARE_CONTENT_PARAM, 0, pTsSig.get(), &nTsSigLen))
        {
            SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_BARE_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        if (!CryptMsgUpdate(hDecodedMsg, pTsSig.get(), nTsSigLen, TRUE))
        {
            SAL_WARN("svl.crypto", "CryptMsgUpdate failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        DWORD nDecodedSignerInfoLen = 0;
        if (!CryptMsgGetParam(hDecodedMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &nDecodedSignerInfoLen))
        {
            SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_SIGNER_INFO_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        std::unique_ptr<BYTE[]> pDecodedSignerInfoBuf(new BYTE[nDecodedSignerInfoLen]);

        if (!CryptMsgGetParam(hDecodedMsg, CMSG_SIGNER_INFO_PARAM, 0, pDecodedSignerInfoBuf.get(), &nDecodedSignerInfoLen))
        {
            SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_SIGNER_INFO_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        CMSG_SIGNER_INFO *pDecodedSignerInfo = reinterpret_cast<CMSG_SIGNER_INFO *>(pDecodedSignerInfoBuf.get());

        CRYPT_TIMESTAMP_PARA aTsPara;
        unsigned int nNonce = comphelper::rng::uniform_uint_distribution(0, SAL_MAX_UINT32);

        aTsPara.pszTSAPolicyId = nullptr;
        aTsPara.fRequestCerts = TRUE;
        aTsPara.Nonce.cbData = sizeof(nNonce);
        aTsPara.Nonce.pbData = reinterpret_cast<BYTE *>(&nNonce);
        aTsPara.cExtension = 0;
        aTsPara.rgExtension = nullptr;

        if (!CryptRetrieveTimeStamp(o3tl::toW(m_aSignTSA.getStr()),
                     0,
                     10000,
                     szOID_NIST_sha256,
                     &aTsPara,
                     pDecodedSignerInfo->EncryptedHash.pbData,
                     pDecodedSignerInfo->EncryptedHash.cbData,
                     &pTsContext,
                     nullptr,
                     nullptr))
        {
            SAL_WARN("svl.crypto", "CryptRetrieveTimeStamp failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        SAL_INFO("svl.crypto", "Time stamp size is " << pTsContext->cbEncoded << " bytes");

#ifdef DBG_UTIL
        {
            FILE *out = fopen("PDFWRITER.tstoken.data", "wb");
            fwrite(pTsContext->pbEncoded, pTsContext->cbEncoded, 1, out);
            fclose(out);
        }
#endif

        // I tried to use CryptMsgControl() with CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR to add the
        // timestamp, but that failed with "The parameter is incorrect". Probably it is too late to
        // modify the message once its data has already been encoded as part of the
        // CryptMsgGetParam() with CMSG_BARE_CONTENT_PARAM above. So close the message and re-do its
        // creation steps, but now with an amended aSignerInfo.

        CRYPT_INTEGER_BLOB aTimestampBlob;
        aTimestampBlob.cbData = pTsContext->cbEncoded;
        aTimestampBlob.pbData = pTsContext->pbEncoded;

        CRYPT_ATTRIBUTE aTimestampAttribute;
        aTimestampAttribute.pszObjId = const_cast<LPSTR>(
            "1.2.840.113549.1.9.16.2.14");
        aTimestampAttribute.cValue = 1;
        aTimestampAttribute.rgValue = &aTimestampBlob;

        aSignerInfo.cUnauthAttr = 1;
        aSignerInfo.rgUnauthAttr = &aTimestampAttribute;

        CryptMsgClose(hMsg);

        hMsg = CryptMsgOpenToEncode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                    CMSG_DETACHED_FLAG,
                                    CMSG_SIGNED,
                                    &aSignedInfo,
                                    nullptr,
                                    nullptr);

        for (size_t i = 0; i < m_dataBlocks.size(); ++i)
        {
            const bool last = (i == m_dataBlocks.size() - 1);
            if (!hMsg ||
                !CryptMsgUpdate(hMsg, static_cast<const BYTE *>(m_dataBlocks[i].first), m_dataBlocks[i].second, last))
            {
                SAL_WARN("svl.crypto", "Re-creating the message failed: " << WindowsErrorString(GetLastError()));
                CryptMemFree(pTsContext);
                CryptMsgClose(hDecodedMsg);
                CryptMsgClose(hMsg);
                CertFreeCertificateContext(pCertContext);
                return false;
            }
        }

        CryptMsgClose(hDecodedMsg);
    }

    DWORD nSigLen = 0;

    if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, nullptr, &nSigLen))
    {
        SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
        if (pTsContext)
            CryptMemFree(pTsContext);
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    if (nSigLen*2 > MAX_SIGNATURE_CONTENT_LENGTH)
    {
        SAL_WARN("svl.crypto", "Signature requires more space (" << nSigLen*2 << ") than we reserved (" << MAX_SIGNATURE_CONTENT_LENGTH << ")");
        if (pTsContext)
            CryptMemFree(pTsContext);
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    SAL_INFO("svl.crypto", "Signature size is " << nSigLen << " bytes");
    std::unique_ptr<BYTE[]> pSig(new BYTE[nSigLen]);

    if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, pSig.get(), &nSigLen))
    {
        SAL_WARN("svl.crypto", "CryptMsgGetParam(CMSG_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
        if (pTsContext)
            CryptMemFree(pTsContext);
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

#ifdef DBG_UTIL
    {
        FILE *out = fopen("PDFWRITER.signature.data", "wb");
        fwrite(pSig.get(), nSigLen, 1, out);
        fclose(out);
    }
#endif

    // Release resources
    if (pTsContext)
        CryptMemFree(pTsContext);
    CryptMsgClose(hMsg);
    CertFreeCertificateContext(pCertContext);

    for (unsigned int i = 0; i < nSigLen ; i++)
        appendHex(pSig[i], rCMSHexBuffer);

    return true;
#endif
}
#else
bool Signing::Sign(OStringBuffer&)
{
    return false;
}
#endif //!SVL_CRYPTO_NSS && !SVL_CRYPTO_MSCRYPTO


namespace
{
#ifdef SVL_CRYPTO_NSS
/// Similar to NSS_CMSAttributeArray_FindAttrByOidTag(), but works directly with a SECOidData.
NSSCMSAttribute* CMSAttributeArray_FindAttrByOidData(NSSCMSAttribute** attrs, SECOidData const * oid, PRBool only)
{
    NSSCMSAttribute* attr1, *attr2;

    if (attrs == nullptr)
        return nullptr;

    if (oid == nullptr)
        return nullptr;

    while ((attr1 = *attrs++) != nullptr)
    {
        if (attr1->type.len == oid->oid.len && PORT_Memcmp(attr1->type.data,
                oid->oid.data,
                oid->oid.len) == 0)
            break;
    }

    if (attr1 == nullptr)
        return nullptr;

    if (!only)
        return attr1;

    while ((attr2 = *attrs++) != nullptr)
    {
        if (attr2->type.len == oid->oid.len && PORT_Memcmp(attr2->type.data,
                oid->oid.data,
                oid->oid.len) == 0)
            break;
    }

    if (attr2 != nullptr)
        return nullptr;

    return attr1;
}

/// Same as SEC_StringToOID(), which is private to us.
SECStatus StringToOID(SECItem* to, const char* from, PRUint32 len)
{
    PRUint32 decimal_numbers = 0;
    PRUint32 result_bytes = 0;
    SECStatus rv;
    PRUint8 result[1024];

    static const PRUint32 max_decimal = 0xffffffff / 10;
    static const char OIDstring[] = {"OID."};

    if (!from || !to)
    {
        PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }
    if (!len)
    {
        len = PL_strlen(from);
    }
    if (len >= 4 && !PL_strncasecmp(from, OIDstring, 4))
    {
        from += 4; /* skip leading "OID." if present */
        len  -= 4;
    }
    if (!len)
    {
bad_data:
        PORT_SetError(SEC_ERROR_BAD_DATA);
        return SECFailure;
    }
    do
    {
        PRUint32 decimal = 0;
        while (len > 0 && rtl::isAsciiDigit(static_cast<unsigned char>(*from)))
        {
            PRUint32 addend = *from++ - '0';
            --len;
            if (decimal > max_decimal)  /* overflow */
                goto bad_data;
            decimal = (decimal * 10) + addend;
            if (decimal < addend)   /* overflow */
                goto bad_data;
        }
        if (len != 0 && *from != '.')
        {
            goto bad_data;
        }
        if (decimal_numbers == 0)
        {
            if (decimal > 2)
                goto bad_data;
            result[0] = decimal * 40;
            result_bytes = 1;
        }
        else if (decimal_numbers == 1)
        {
            if (decimal > 40)
                goto bad_data;
            result[0] += decimal;
        }
        else
        {
            /* encode the decimal number,  */
            PRUint8* rp;
            PRUint32 num_bytes = 0;
            PRUint32 tmp = decimal;
            while (tmp)
            {
                num_bytes++;
                tmp >>= 7;
            }
            if (!num_bytes)
                ++num_bytes;  /* use one byte for a zero value */
            if (static_cast<size_t>(num_bytes) + result_bytes > sizeof result)
                goto bad_data;
            tmp = num_bytes;
            rp = result + result_bytes - 1;
            rp[tmp] = static_cast<PRUint8>(decimal & 0x7f);
            decimal >>= 7;
            while (--tmp > 0)
            {
                rp[tmp] = static_cast<PRUint8>(decimal | 0x80);
                decimal >>= 7;
            }
            result_bytes += num_bytes;
        }
        ++decimal_numbers;
        if (len > 0)   /* skip trailing '.' */
        {
            ++from;
            --len;
        }
    }
    while (len > 0);
    /* now result contains result_bytes of data */
    if (to->data && to->len >= result_bytes)
    {
        to->len = result_bytes;
        PORT_Memcpy(to->data, result, to->len);
        rv = SECSuccess;
    }
    else
    {
        SECItem result_item = {siBuffer, nullptr, 0 };
        result_item.data = result;
        result_item.len  = result_bytes;
        rv = SECITEM_CopyItem(nullptr, to, &result_item);
    }
    return rv;
}
#elif defined SVL_CRYPTO_MSCRYPTO
/// Verifies a non-detached signature using CryptoAPI.
bool VerifyNonDetachedSignature(const std::vector<unsigned char>& aData, const std::vector<BYTE>& rExpectedHash)
{
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContextW(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        SAL_WARN("svl.crypto", "CryptAcquireContext() failed");
        return false;
    }

    HCRYPTHASH hHash = 0;
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
        SAL_WARN("svl.crypto", "CryptCreateHash() failed");
        return false;
    }

    if (!CryptHashData(hHash, aData.data(), aData.size(), 0))
    {
        SAL_WARN("svl.crypto", "CryptHashData() failed");
        return false;
    }

    DWORD nActualHash = 0;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, nullptr, &nActualHash, 0))
    {
        SAL_WARN("svl.crypto", "CryptGetHashParam() failed to provide the hash length");
        return false;
    }

    std::vector<unsigned char> aActualHash(nActualHash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, aActualHash.data(), &nActualHash, 0))
    {
        SAL_WARN("svl.crypto", "CryptGetHashParam() failed to provide the hash");
        return false;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return aActualHash.size() == rExpectedHash.size() &&
           !std::memcmp(aActualHash.data(), rExpectedHash.data(), aActualHash.size());
}

OUString GetSubjectName(PCCERT_CONTEXT pCertContext)
{
    OUString subjectName;

    // Get Subject name size.
    DWORD dwData = CertGetNameStringW(pCertContext,
                                      CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                      0,
                                      nullptr,
                                      nullptr,
                                      0);
    if (!dwData)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CertGetNameString failed");
        return subjectName;
    }

    // Allocate memory for subject name.
    LPWSTR szName = static_cast<LPWSTR>(
        LocalAlloc(LPTR, dwData * sizeof(WCHAR)));
    if (!szName)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: Unable to allocate memory for subject name");
        return subjectName;
    }

    // Get subject name.
    if (!CertGetNameStringW(pCertContext,
                            CERT_NAME_SIMPLE_DISPLAY_TYPE,
                            0,
                            nullptr,
                            szName,
                            dwData))
    {
        LocalFree(szName);
        SAL_WARN("svl.crypto", "ValidateSignature: CertGetNameString failed");
        return subjectName;
    }

    subjectName = o3tl::toU(szName);
    LocalFree(szName);

    return subjectName;
}

#endif
}

bool Signing::Verify(const std::vector<unsigned char>& aData,
                     const bool bNonDetached,
                     const std::vector<unsigned char>& aSignature,
                     SignatureInformation& rInformation)
{
#ifdef SVL_CRYPTO_NSS
    // Validate the signature. No need to call NSS_Init() here, assume that the
    // caller did that already.

    SECItem aSignatureItem;
    aSignatureItem.data = const_cast<unsigned char*>(aSignature.data());
    aSignatureItem.len = aSignature.size();
    NSSCMSMessage* pCMSMessage = NSS_CMSMessage_CreateFromDER(&aSignatureItem,
                                 /*cb=*/nullptr,
                                 /*cb_arg=*/nullptr,
                                 /*pwfn=*/nullptr,
                                 /*pwfn_arg=*/nullptr,
                                 /*decrypt_key_cb=*/nullptr,
                                 /*decrypt_key_cb_arg=*/nullptr);
    if (!NSS_CMSMessage_IsSigned(pCMSMessage))
    {
        SAL_WARN("svl.crypto", "ValidateSignature: message is not signed");
        return false;
    }

    NSSCMSContentInfo* pCMSContentInfo = NSS_CMSMessage_ContentLevel(pCMSMessage, 0);
    if (!pCMSContentInfo)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: NSS_CMSMessage_ContentLevel() failed");
        return false;
    }

    auto pCMSSignedData = static_cast<NSSCMSSignedData*>(NSS_CMSContentInfo_GetContent(pCMSContentInfo));
    if (!pCMSSignedData)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: NSS_CMSContentInfo_GetContent() failed");
        return false;
    }

    // Import certificates from the signed data temporarily, so it'll be
    // possible to verify the signature, even if we didn't have the certificate
    // previously.
    std::vector<CERTCertificate*> aDocumentCertificates;
    for (size_t i = 0; pCMSSignedData->rawCerts[i]; ++i)
        aDocumentCertificates.push_back(CERT_NewTempCertificate(CERT_GetDefaultCertDB(), pCMSSignedData->rawCerts[i], nullptr, 0, 0));

    NSSCMSSignerInfo* pCMSSignerInfo = NSS_CMSSignedData_GetSignerInfo(pCMSSignedData, 0);
    if (!pCMSSignerInfo)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: NSS_CMSSignedData_GetSignerInfo() failed");
        return false;
    }

    SECItem aAlgorithm = NSS_CMSSignedData_GetDigestAlgs(pCMSSignedData)[0]->algorithm;
    SECOidTag eOidTag = SECOID_FindOIDTag(&aAlgorithm);

    // Map a sign algorithm to a digest algorithm.
    // See NSS_CMSUtil_MapSignAlgs(), which is private to us.
    switch (eOidTag)
    {
    case SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA1;
        break;
    case SEC_OID_PKCS1_SHA256_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA256;
        break;
    case SEC_OID_PKCS1_SHA512_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA512;
        break;
    default:
        break;
    }

    HASH_HashType eHashType = HASH_GetHashTypeByOidTag(eOidTag);
    HASHContext* pHASHContext = HASH_Create(eHashType);
    if (!pHASHContext)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: HASH_Create() failed");
        return false;
    }

    // We have a hash, update it with the byte ranges.
    HASH_Update(pHASHContext, aData.data(), aData.size());

    // Find out what is the expected length of the hash.
    unsigned int nMaxResultLen = 0;
    switch (eOidTag)
    {
    case SEC_OID_SHA1:
        nMaxResultLen = msfilter::SHA1_HASH_LENGTH;
        rInformation.nDigestID = xml::crypto::DigestID::SHA1;
        break;
    case SEC_OID_SHA256:
        nMaxResultLen = msfilter::SHA256_HASH_LENGTH;
        rInformation.nDigestID = xml::crypto::DigestID::SHA256;
        break;
    case SEC_OID_SHA512:
        nMaxResultLen = msfilter::SHA512_HASH_LENGTH;
        rInformation.nDigestID = xml::crypto::DigestID::SHA512;
        break;
    default:
        SAL_WARN("svl.crypto", "ValidateSignature: unrecognized algorithm");
        return false;
    }

    auto pActualResultBuffer = static_cast<unsigned char*>(PORT_Alloc(nMaxResultLen));
    unsigned int nActualResultLen;
    HASH_End(pHASHContext, pActualResultBuffer, &nActualResultLen, nMaxResultLen);

    CERTCertificate* pCertificate = NSS_CMSSignerInfo_GetSigningCertificate(pCMSSignerInfo, CERT_GetDefaultCertDB());
    if (!pCertificate)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: NSS_CMSSignerInfo_GetSigningCertificate() failed");
        return false;
    }
    else
    {
        uno::Sequence<sal_Int8> aDerCert(pCertificate->derCert.len);
        for (size_t i = 0; i < pCertificate->derCert.len; ++i)
            aDerCert[i] = pCertificate->derCert.data[i];
        OUStringBuffer aBuffer;
        comphelper::Base64::encode(aBuffer, aDerCert);
        rInformation.ouX509Certificate = aBuffer.makeStringAndClear();
        rInformation.ouSubject = OUString(pCertificate->subjectName, PL_strlen(pCertificate->subjectName), RTL_TEXTENCODING_UTF8);
    }

    PRTime nSigningTime;
    // This may fail, in which case the date should be taken from the PDF's dictionary's "M" key,
    // so not critical for PDF at least.
    if (NSS_CMSSignerInfo_GetSigningTime(pCMSSignerInfo, &nSigningTime) == SECSuccess)
    {
        // First convert the UTC UNIX timestamp to a tools::DateTime.
        // nSigningTime is in microseconds.
        DateTime aDateTime = DateTime::CreateFromUnixTime(static_cast<double>(nSigningTime) / 1000000);

        // Then convert to a local UNO DateTime.
        aDateTime.ConvertToLocalTime();
        rInformation.stDateTime = aDateTime.GetUNODateTime();
        if (rInformation.ouDateTime.isEmpty())
        {
            OUStringBuffer rBuffer;
            rBuffer.append(static_cast<sal_Int32>(aDateTime.GetYear()));
            rBuffer.append('-');
            if (aDateTime.GetMonth() < 10)
                rBuffer.append('0');
            rBuffer.append(static_cast<sal_Int32>(aDateTime.GetMonth()));
            rBuffer.append('-');
            if (aDateTime.GetDay() < 10)
                rBuffer.append('0');
            rBuffer.append(static_cast<sal_Int32>(aDateTime.GetDay()));
            rInformation.ouDateTime = rBuffer.makeStringAndClear();
        }
    }

    // Check if we have a signing certificate attribute.
    SECOidData aOidData;
    aOidData.oid.data = nullptr;
    /*
     * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
     * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
     *   smime(16) id-aa(2) 47 }
     */
    if (StringToOID(&aOidData.oid, "1.2.840.113549.1.9.16.2.47", 0) != SECSuccess)
    {
        SAL_WARN("svl.crypto", "StringToOID() failed");
        return false;
    }
    aOidData.offset = SEC_OID_UNKNOWN;
    aOidData.desc = "id-aa-signingCertificateV2";
    aOidData.mechanism = CKM_SHA_1;
    aOidData.supportedExtension = UNSUPPORTED_CERT_EXTENSION;
    NSSCMSAttribute* pAttribute = CMSAttributeArray_FindAttrByOidData(pCMSSignerInfo->authAttr, &aOidData, PR_TRUE);
    if (pAttribute)
        rInformation.bHasSigningCertificate = true;

    SECItem* pContentInfoContentData = pCMSSignedData->contentInfo.content.data;
    if (bNonDetached && pContentInfoContentData && pContentInfoContentData->data)
    {
        // Not a detached signature.
        if (!std::memcmp(pActualResultBuffer, pContentInfoContentData->data, nMaxResultLen) && nActualResultLen == pContentInfoContentData->len)
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }
    else
    {
        // Detached, the usual case.
        SECItem aActualResultItem;
        aActualResultItem.data = pActualResultBuffer;
        aActualResultItem.len = nActualResultLen;
        if (NSS_CMSSignerInfo_Verify(pCMSSignerInfo, &aActualResultItem, nullptr) == SECSuccess)
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }

    // Everything went fine
    PORT_Free(pActualResultBuffer);
    HASH_Destroy(pHASHContext);
    NSS_CMSSignerInfo_Destroy(pCMSSignerInfo);
    for (auto pDocumentCertificate : aDocumentCertificates)
        CERT_DestroyCertificate(pDocumentCertificate);

    return true;

#elif defined SVL_CRYPTO_MSCRYPTO
    // Open a message for decoding.
    HCRYPTMSG hMsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                          CMSG_DETACHED_FLAG,
                                          0,
                                          NULL,
                                          nullptr,
                                          nullptr);
    if (!hMsg)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgOpenToDecode() failed");
        return false;
    }

    // Update the message with the encoded header blob.
    if (!CryptMsgUpdate(hMsg, aSignature.data(), aSignature.size(), TRUE))
    {
        SAL_WARN("svl.crypto", "ValidateSignature, CryptMsgUpdate() for the header failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // Update the message with the content blob.
    if (!CryptMsgUpdate(hMsg, aData.data(), aData.size(), FALSE))
    {
        SAL_WARN("svl.crypto", "ValidateSignature, CryptMsgUpdate() for the content failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    if (!CryptMsgUpdate(hMsg, nullptr, 0, TRUE))
    {
        SAL_WARN("svl.crypto", "ValidateSignature, CryptMsgUpdate() for the last content failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // Get the CRYPT_ALGORITHM_IDENTIFIER from the message.
    DWORD nDigestID = 0;
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_HASH_ALGORITHM_PARAM, 0, nullptr, &nDigestID))
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    std::unique_ptr<BYTE[]> pDigestBytes(new BYTE[nDigestID]);
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_HASH_ALGORITHM_PARAM, 0, pDigestBytes.get(), &nDigestID))
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    auto pDigestID = reinterpret_cast<CRYPT_ALGORITHM_IDENTIFIER*>(pDigestBytes.get());
    if (OString(szOID_NIST_sha256) == pDigestID->pszObjId)
        rInformation.nDigestID = xml::crypto::DigestID::SHA256;
    else if (OString(szOID_RSA_SHA1RSA) == pDigestID->pszObjId || OString(szOID_OIWSEC_sha1) == pDigestID->pszObjId)
        rInformation.nDigestID = xml::crypto::DigestID::SHA1;
    else
        // Don't error out here, we can still verify the message digest correctly, just the digest ID won't be set.
        SAL_WARN("svl.crypto", "ValidateSignature: unhandled algorithm identifier '"<<pDigestID->pszObjId<<"'");

    // Get the signer CERT_INFO from the message.
    DWORD nSignerCertInfo = 0;
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, nullptr, &nSignerCertInfo))
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed");
        return false;
    }
    std::unique_ptr<BYTE[]> pSignerCertInfoBuf(new BYTE[nSignerCertInfo]);
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, pSignerCertInfoBuf.get(), &nSignerCertInfo))
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed");
        return false;
    }
    PCERT_INFO pSignerCertInfo = reinterpret_cast<PCERT_INFO>(pSignerCertInfoBuf.get());

    // Open a certificate store in memory using CERT_STORE_PROV_MSG, which
    // initializes it with the certificates from the message.
    HCERTSTORE hStoreHandle = CertOpenStore(CERT_STORE_PROV_MSG,
                                            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                            NULL,
                                            0,
                                            hMsg);
    if (!hStoreHandle)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CertOpenStore() failed");
        return false;
    }

    // Find the signer's certificate in the store.
    PCCERT_CONTEXT pSignerCertContext = CertGetSubjectCertificateFromStore(hStoreHandle,
                                        PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                        pSignerCertInfo);
    if (!pSignerCertContext)
    {
        SAL_WARN("svl.crypto", "ValidateSignature: CertGetSubjectCertificateFromStore() failed");
        return false;
    }
    else
    {
        // Write rInformation.ouX509Certificate.
        uno::Sequence<sal_Int8> aDerCert(pSignerCertContext->cbCertEncoded);
        for (size_t i = 0; i < pSignerCertContext->cbCertEncoded; ++i)
            aDerCert[i] = pSignerCertContext->pbCertEncoded[i];
        OUStringBuffer aBuffer;
        comphelper::Base64::encode(aBuffer, aDerCert);
        rInformation.ouX509Certificate = aBuffer.makeStringAndClear();
        rInformation.ouSubject = GetSubjectName(pSignerCertContext);
    }

    if (bNonDetached)
    {
        // Not a detached signature.
        DWORD nContentParam = 0;
        if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, nullptr, &nContentParam))
        {
            SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed");
            return false;
        }

        std::vector<BYTE> aContentParam(nContentParam);
        if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, aContentParam.data(), &nContentParam))
        {
            SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() failed");
            return false;
        }

        if (VerifyNonDetachedSignature(aData, aContentParam))
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }
    else
    {
        // Detached, the usual case.
        // Use the CERT_INFO from the signer certificate to verify the signature.
        if (CryptMsgControl(hMsg, 0, CMSG_CTRL_VERIFY_SIGNATURE, pSignerCertContext->pCertInfo))
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }

    // Check if we have a signing certificate attribute.
    DWORD nSignedAttributes = 0;
    if (CryptMsgGetParam(hMsg, CMSG_SIGNER_AUTH_ATTR_PARAM, 0, nullptr, &nSignedAttributes))
    {
        std::unique_ptr<BYTE[]> pSignedAttributesBuf(new BYTE[nSignedAttributes]);
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_AUTH_ATTR_PARAM, 0, pSignedAttributesBuf.get(), &nSignedAttributes))
        {
            SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() authenticated failed");
            return false;
        }
        auto pSignedAttributes = reinterpret_cast<PCRYPT_ATTRIBUTES>(pSignedAttributesBuf.get());
        for (size_t nAttr = 0; nAttr < pSignedAttributes->cAttr; ++nAttr)
        {
            CRYPT_ATTRIBUTE& rAttr = pSignedAttributes->rgAttr[nAttr];
            /*
             * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
             * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
             *   smime(16) id-aa(2) 47 }
             */
            if (OString("1.2.840.113549.1.9.16.2.47") == rAttr.pszObjId)
            {
                rInformation.bHasSigningCertificate = true;
                break;
            }
        }
    }

    // Get the unauthorized attributes.
    nSignedAttributes = 0;
    if (CryptMsgGetParam(hMsg, CMSG_SIGNER_UNAUTH_ATTR_PARAM, 0, nullptr, &nSignedAttributes))
    {
        std::unique_ptr<BYTE[]> pSignedAttributesBuf(new BYTE[nSignedAttributes]);
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_UNAUTH_ATTR_PARAM, 0, pSignedAttributesBuf.get(), &nSignedAttributes))
        {
            SAL_WARN("svl.crypto", "ValidateSignature: CryptMsgGetParam() unauthenticated failed");
            return false;
        }
        auto pSignedAttributes = reinterpret_cast<PCRYPT_ATTRIBUTES>(pSignedAttributesBuf.get());
        for (size_t nAttr = 0; nAttr < pSignedAttributes->cAttr; ++nAttr)
        {
            CRYPT_ATTRIBUTE& rAttr = pSignedAttributes->rgAttr[nAttr];
            // Timestamp blob
            if (OString("1.2.840.113549.1.9.16.2.14") == rAttr.pszObjId)
            {
                PCRYPT_TIMESTAMP_CONTEXT pTsContext;
                if (!CryptVerifyTimeStampSignature(rAttr.rgValue->pbData, rAttr.rgValue->cbData, nullptr, 0, nullptr, &pTsContext, nullptr, nullptr))
                {
                    SAL_WARN("svl.crypto", "CryptMsgUpdate failed: " << WindowsErrorString(GetLastError()));
                    break;
                }

                DateTime aDateTime = DateTime::CreateFromWin32FileDateTime(pTsContext->pTimeStamp->ftTime.dwLowDateTime, pTsContext->pTimeStamp->ftTime.dwHighDateTime);

                // Then convert to a local UNO DateTime.
                aDateTime.ConvertToLocalTime();
                rInformation.stDateTime = aDateTime.GetUNODateTime();
                if (rInformation.ouDateTime.isEmpty())
                {
                    OUStringBuffer rBuffer;
                    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetYear()));
                    rBuffer.append('-');
                    if (aDateTime.GetMonth() < 10)
                        rBuffer.append('0');
                    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetMonth()));
                    rBuffer.append('-');
                    if (aDateTime.GetDay() < 10)
                        rBuffer.append('0');
                    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetDay()));
                    rInformation.ouDateTime = rBuffer.makeStringAndClear();
                }
                break;
            }
        }
    }

    CertCloseStore(hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG);
    CryptMsgClose(hMsg);
    return true;
#else
    // Not implemented.
    (void)aData;
    (void)bNonDetached;
    (void)aSignature;
    (void)rInformation;
    return false;
#endif
}

bool Signing::Verify(SvStream& rStream,
                     const std::vector<std::pair<size_t, size_t>>& aByteRanges,
                     const bool bNonDetached,
                     const std::vector<unsigned char>& aSignature,
                     SignatureInformation& rInformation)
{
#if defined(SVL_CRYPTO_NSS) || defined(SVL_CRYPTO_MSCRYPTO)

    std::vector<unsigned char> buffer;

    // Copy the byte ranges into a single buffer.
    for (const auto& rByteRange : aByteRanges)
    {
        rStream.Seek(rByteRange.first);
        const size_t size = buffer.size();
        buffer.resize(size + rByteRange.second);
        rStream.ReadBytes(buffer.data() + size, rByteRange.second);
    }

    return Verify(buffer, bNonDetached, aSignature, rInformation);

#else
    // Not implemented.
    (void)rStream;
    (void)aByteRanges;
    (void)bNonDetached;
    (void)aSignature;
    (void)rInformation;
    return false;
#endif
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
