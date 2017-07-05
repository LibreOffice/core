/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/cryptosign.hxx>

#include <rtl/character.hxx>
#include <comphelper/random.hxx>

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

namespace {

static void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}

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

typedef struct {
    SECAlgorithmID hashAlgorithm;
    SECItem hashedMessage;
} MessageImprint;

/*
Extension  ::=  SEQUENCE  {
    extnID    OBJECT IDENTIFIER,
    critical  BOOLEAN DEFAULT FALSE,
    extnValue OCTET STRING  }
*/

typedef struct {
    SECItem extnID;
    SECItem critical;
    SECItem extnValue;
} Extension;

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

typedef struct {
    SECItem version;
    MessageImprint messageImprint;
    SECItem reqPolicy;
    SECItem nonce;
    SECItem certReq;
    Extension *extensions;
} TimeStampReq;

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

typedef struct {
    SECItem status;
    SECItem statusString;
    SECItem failInfo;
} PKIStatusInfo;

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

typedef struct {
    PKIStatusInfo status;
    SECItem timeStampToken;
} TimeStampResp;

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

size_t AppendToBuffer(char *ptr, size_t size, size_t nmemb, void *userdata)
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

    static const PRUint32 max_decimal = (0xffffffff / 10);
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
        PRUint32 addend = (*from++ - '0');
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
        rp[tmp] = (PRUint8)(decimal & 0x7f);
        decimal >>= 7;
        while (--tmp > 0) {
        rp[tmp] = (PRUint8)(decimal | 0x80);
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
        PORT_Memcpy(to->data, result, to->len = result_bytes);
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
        SAL_WARN("vcl.pdfwriter", "NSS_CMSMessage_Create failed");
        return nullptr;
    }

    *cms_sd = NSS_CMSSignedData_Create(result);
    if (!*cms_sd)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignedData_Create failed");
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    NSSCMSContentInfo *cms_cinfo = NSS_CMSMessage_GetContentInfo(result);
    if (NSS_CMSContentInfo_SetContent_SignedData(result, cms_cinfo, *cms_sd) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSContentInfo_SetContent_SignedData failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    cms_cinfo = NSS_CMSSignedData_GetContentInfo(*cms_sd);

    // Attach NULL data as detached data
    if (NSS_CMSContentInfo_SetContent_Data(result, cms_cinfo, nullptr, PR_TRUE) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSContentInfo_SetContent_Data failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    *cms_signer = NSS_CMSSignerInfo_Create(result, cert, SEC_OID_SHA256);
    if (!*cms_signer)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignerInfo_Create failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (time && NSS_CMSSignerInfo_AddSigningTime(*cms_signer, *time) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignerInfo_AddSigningTime failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignerInfo_IncludeCerts(*cms_signer, NSSCMSCM_CertChain, certUsageEmailSigner) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignerInfo_IncludeCerts failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_AddCertificate(*cms_sd, cert) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignedData_AddCertificate failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_AddSignerInfo(*cms_sd, *cms_signer) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignedData_AddSignerInfo failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    if (NSS_CMSSignedData_SetDigestValue(*cms_sd, SEC_OID_SHA256, digest) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSSignedData_SetDigestValue failed");
        NSS_CMSSignedData_Destroy(*cms_sd);
        NSS_CMSMessage_Destroy(result);
        return nullptr;
    }

    return result;
}

} // Anonymous namespace

namespace comphelper {

namespace crypto {

bool Signing::Sign(OStringBuffer& rCMSHexBuffer)
{
    // Create the PKCS#7 object.
    css::uno::Sequence<sal_Int8> aDerEncoded = m_xCertificate->getEncoded();
    if (!aDerEncoded.hasElements())
    {
        SAL_WARN("comphelper", "Crypto::Signing: empty certificate");
        return false;
    }

#ifndef _WIN32

    CERTCertificate *cert = CERT_DecodeCertFromPackage(reinterpret_cast<char *>(aDerEncoded.getArray()), aDerEncoded.getLength());

    if (!cert)
    {
        SAL_WARN("vcl.pdfwriter", "CERT_DecodeCertFromPackage failed");
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
            SAL_WARN("vcl.pdfwriter", "NSS_CMSEncoder_Finish failed");
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
            SAL_WARN("vcl.pdfwriter", "SEC_ASN1EncodeItem failed");
            return false;
        }

        if (timestamp_request->data == nullptr)
        {
            SAL_WARN("vcl.pdfwriter", "SEC_ASN1EncodeItem succeeded but got NULL data");
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "request length=" << timestamp_request->len);

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
            SAL_WARN("vcl.pdfwriter", "curl_easy_init failed");
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "Setting curl to verbose: " << (curl_easy_setopt(curl, CURLOPT_VERBOSE, 1) == CURLE_OK ? "OK" : "FAIL"));

        if ((rc = curl_easy_setopt(curl, CURLOPT_URL, OUStringToOString(m_aSignTSA, RTL_TEXTENCODING_UTF8).getStr())) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_URL) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        slist = curl_slist_append(slist, "Content-Type: application/timestamp-query");
        slist = curl_slist_append(slist, "Accept: application/timestamp-reply");

        if ((rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_HTTPHEADER) failed: " << curl_easy_strerror(rc));
            curl_slist_free_all(slist);
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(timestamp_request->len))) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, timestamp_request->data)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_POSTFIELDSIZE or CURLOPT_POSTFIELDS) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer)) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AppendToBuffer)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_WRITEDATA or CURLOPT_WRITEFUNCTION) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if ((rc = curl_easy_setopt(curl, CURLOPT_POST, 1)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_POST) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        char error_buffer[CURL_ERROR_SIZE];
        if ((rc = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_ERRORBUFFER) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        // Use a ten second timeout
        if ((rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10)) != CURLE_OK ||
            (rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10)) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_setopt(CURLOPT_TIMEOUT or CURLOPT_CONNECTTIMEOUT) failed: " << curl_easy_strerror(rc));
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        if (curl_easy_perform(curl) != CURLE_OK)
        {
            SAL_WARN("vcl.pdfwriter", "curl_easy_perform failed: " << error_buffer);
            curl_easy_cleanup(curl);
            SECITEM_FreeItem(timestamp_request, PR_TRUE);
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "PDF signing: got response, length=" << response_buffer.getLength());

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
            SAL_WARN("vcl.pdfwriter", "SEC_ASN1DecodeItem failed");
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "TimeStampResp received and decoded, status=" << PKIStatusInfoToString(response.status));

        if (response.status.status.len != 1 ||
            (response.status.status.data[0] != 0 && response.status.status.data[0] != 1))
        {
            SAL_WARN("vcl.pdfwriter", "Timestamp request was not granted");
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
            SAL_WARN("vcl.pdfwriter", "SEC_StringToOID failed");
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
            SAL_WARN("vcl.pdfwriter", "NSS_CMSSignerInfo_AddUnauthAttr failed");
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
        SAL_WARN("vcl.pdfwriter", "SEC_ASN1EncodeItem() failed");
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
        SAL_WARN("vcl.pdfwriter", "my_SEC_StringToOID() failed");
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
        SAL_WARN("vcl.pdfwriter", "my_NSS_CMSSignerInfo_AddAuthAttr() failed");
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
        SAL_WARN("vcl.pdfwriter", "NSS_CMSEncoder_Start failed");
        return false;
    }

    if (NSS_CMSEncoder_Finish(cms_ecx) != SECSuccess)
    {
        SAL_WARN("vcl.pdfwriter", "NSS_CMSEncoder_Finish failed");
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
        SAL_WARN("vcl.pdfwriter", "Signature requires more space (" << cms_output.len*2 << ") than we reserved (" << MAX_SIGNATURE_CONTENT_LENGTH << ")");
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
        SAL_WARN("vcl.pdfwriter", "CertCreateCertificateContext failed: " << WindowsErrorString(GetLastError()));
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

    HCRYPTPROV hCryptProv;
    DWORD nKeySpec;
    BOOL bFreeNeeded;

    if (!CryptAcquireCertificatePrivateKey(pCertContext,
                                           CRYPT_ACQUIRE_CACHE_FLAG,
                                           nullptr,
                                           &hCryptProv,
                                           &nKeySpec,
                                           &bFreeNeeded))
    {
        SAL_WARN("vcl.pdfwriter", "CryptAcquireCertificatePrivateKey failed: " << WindowsErrorString(GetLastError()));
        CertFreeCertificateContext(pCertContext);
        return false;
    }
    assert(!bFreeNeeded);

    CMSG_SIGNER_ENCODE_INFO aSignerInfo;

    memset(&aSignerInfo, 0, sizeof(aSignerInfo));
    aSignerInfo.cbSize = sizeof(aSignerInfo);
    aSignerInfo.pCertInfo = pCertContext->pCertInfo;
    aSignerInfo.hCryptProv = hCryptProv;
    aSignerInfo.dwKeySpec = nKeySpec;
    aSignerInfo.HashAlgorithm.pszObjId = const_cast<LPSTR>(szOID_NIST_sha256);
    aSignerInfo.HashAlgorithm.Parameters.cbData = 0;

    // Add the signing certificate as a signed attribute.
    CRYPT_INTEGER_BLOB aCertificateBlob;
    SvMemoryStream aEncodedCertificate;
    if (!CreateSigningCertificateAttribute(rContext, pCertContext, aEncodedCertificate))
    {
        SAL_WARN("vcl.pdfwriter", "CreateSigningCertificateAttribute() failed");
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
        SAL_WARN("vcl.pdfwriter", "CryptMsgOpenToEncode failed: " << WindowsErrorString(GetLastError()));
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    if (!CryptMsgUpdate(hMsg, static_cast<const BYTE *>(rContext.m_pByteRange1), rContext.m_nByteRange1, FALSE) ||
        !CryptMsgUpdate(hMsg, static_cast<const BYTE *>(rContext.m_pByteRange2), rContext.m_nByteRange2, TRUE))
    {
        SAL_WARN("vcl.pdfwriter", "CryptMsgUpdate failed: " << WindowsErrorString(GetLastError()));
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    PCRYPT_TIMESTAMP_CONTEXT pTsContext = nullptr;

    if( !rContext.m_aSignTSA.isEmpty() )
    {
        HCRYPTMSG hDecodedMsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                                     CMSG_DETACHED_FLAG,
                                                     CMSG_SIGNED,
                                                     NULL,
                                                     nullptr,
                                                     nullptr);
        if (!hDecodedMsg)
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgOpenToDecode failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        DWORD nTsSigLen = 0;

        if (!CryptMsgGetParam(hMsg, CMSG_BARE_CONTENT_PARAM, 0, nullptr, &nTsSigLen))
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_BARE_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "nTsSigLen=" << nTsSigLen);

        std::unique_ptr<BYTE[]> pTsSig(new BYTE[nTsSigLen]);

        if (!CryptMsgGetParam(hMsg, CMSG_BARE_CONTENT_PARAM, 0, pTsSig.get(), &nTsSigLen))
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_BARE_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        if (!CryptMsgUpdate(hDecodedMsg, pTsSig.get(), nTsSigLen, TRUE))
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgUpdate failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        DWORD nDecodedSignerInfoLen = 0;
        if (!CryptMsgGetParam(hDecodedMsg, CMSG_SIGNER_INFO_PARAM, 0, nullptr, &nDecodedSignerInfoLen))
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_SIGNER_INFO_PARAM) failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        std::unique_ptr<BYTE[]> pDecodedSignerInfoBuf(new BYTE[nDecodedSignerInfoLen]);

        if (!CryptMsgGetParam(hDecodedMsg, CMSG_SIGNER_INFO_PARAM, 0, pDecodedSignerInfoBuf.get(), &nDecodedSignerInfoLen))
        {
            SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_SIGNER_INFO_PARAM) failed: " << WindowsErrorString(GetLastError()));
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

        if (!CryptRetrieveTimeStamp(SAL_W(rContext.m_aSignTSA.getStr()),
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
            SAL_WARN("vcl.pdfwriter", "CryptRetrieveTimeStamp failed: " << WindowsErrorString(GetLastError()));
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        SAL_INFO("vcl.pdfwriter", "Time stamp size is " << pTsContext->cbEncoded << " bytes");

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
        if (!hMsg ||
            !CryptMsgUpdate(hMsg, static_cast<const BYTE *>(rContext.m_pByteRange1), rContext.m_nByteRange1, FALSE) ||
            !CryptMsgUpdate(hMsg, static_cast<const BYTE *>(rContext.m_pByteRange1), rContext.m_nByteRange2, TRUE))
        {
            SAL_WARN("vcl.pdfwriter", "Re-creating the message failed: " << WindowsErrorString(GetLastError()));
            CryptMemFree(pTsContext);
            CryptMsgClose(hDecodedMsg);
            CryptMsgClose(hMsg);
            CertFreeCertificateContext(pCertContext);
            return false;
        }

        CryptMsgClose(hDecodedMsg);
    }

    DWORD nSigLen = 0;

    if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, nullptr, &nSigLen))
    {
        SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
        if (pTsContext)
            CryptMemFree(pTsContext);
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    if (nSigLen*2 > MAX_SIGNATURE_CONTENT_LENGTH)
    {
        SAL_WARN("vcl.pdfwriter", "Signature requires more space (" << nSigLen*2 << ") than we reserved (" << MAX_SIGNATURE_CONTENT_LENGTH << ")");
        if (pTsContext)
            CryptMemFree(pTsContext);
        CryptMsgClose(hMsg);
        CertFreeCertificateContext(pCertContext);
        return false;
    }

    SAL_INFO("vcl.pdfwriter", "Signature size is " << nSigLen << " bytes");
    std::unique_ptr<BYTE[]> pSig(new BYTE[nSigLen]);

    if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, pSig.get(), &nSigLen))
    {
        SAL_WARN("vcl.pdfwriter", "CryptMsgGetParam(CMSG_CONTENT_PARAM) failed: " << WindowsErrorString(GetLastError()));
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

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
