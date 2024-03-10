/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "ooxmlsecparser.hxx"
#include <xmlsignaturehelper.hxx>
#include <xsecctl.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>

#include <sal/log.hxx>

using namespace com::sun::star;

class OOXMLSecParser::Context
{
    protected:
        friend class OOXMLSecParser;
        OOXMLSecParser & m_rParser;
    private:
        std::optional<SvXMLNamespaceMap> m_pOldNamespaceMap;

    public:
        Context(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : m_rParser(rParser)
            , m_pOldNamespaceMap(std::move(pOldNamespaceMap))
        {
        }

        virtual ~Context() = default;

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& /*xAttrs*/)
        {
        }

        virtual void EndElement()
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const /*nNamespace*/, OUString const& /*rName*/);

        virtual void Characters(OUString const& /*rChars*/)
        {
        }
};

// it's possible that an unsupported element has an Id attribute and a
// ds:Reference digesting it - probably this means XSecController needs to know
// about it. (For known elements, the Id attribute is only processed according
// to the schema.)
class OOXMLSecParser::UnknownContext
    : public OOXMLSecParser::Context
{
    public:
        UnknownContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }
};

auto OOXMLSecParser::Context::CreateChildContext(
    std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
    sal_uInt16 const /*nNamespace*/, OUString const& /*rName*/)
-> std::unique_ptr<Context>
{
    // default: create new base context
    return std::make_unique<UnknownContext>(m_rParser, std::move(pOldNamespaceMap));
}

/**
note: anything in ds:Object should be trusted *only* if there is a ds:Reference
      to it so it is signed (exception: the xades:EncapsulatedX509Certificate).
      ds:SignedInfo precedes all ds:Object.

      There may be multiple ds:Signature for purpose of counter-signatures
      but the way XAdES describes these, only the ds:SignatureValue element
      would be referenced, so requiring a ds:Reference for anything in
      ds:Object shouldn't cause issues.
 */
class OOXMLSecParser::ReferencedContextImpl
    : public OOXMLSecParser::Context
{
    protected:
        bool m_isReferenced;

    public:
        ReferencedContextImpl(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_isReferenced(isReferenced)
        {
        }

        OUString CheckIdAttrReferenced(css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs)
        {
            OUString const id(m_rParser.HandleIdAttr(xAttrs));
            if (!id.isEmpty() && m_rParser.m_pXSecController->haveReferenceForId(id))
            {
                m_isReferenced = true;
            }
            return id;
        }
};

class OOXMLSecParser::DsX509CertificateContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509CertificateContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::DsX509SerialNumberContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509SerialNumberContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::DsX509IssuerNameContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509IssuerNameContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::DsX509IssuerSerialContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rX509IssuerName;
        OUString & m_rX509SerialNumber;

    public:
        DsX509IssuerSerialContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rIssuerName, OUString& rSerialNumber)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rX509IssuerName(rIssuerName)
            , m_rX509SerialNumber(rSerialNumber)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509IssuerName")
            {
                return std::make_unique<DsX509IssuerNameContext>(m_rParser, std::move(pOldNamespaceMap), m_rX509IssuerName);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509SerialNumber")
            {
                return std::make_unique<DsX509SerialNumberContext>(m_rParser, std::move(pOldNamespaceMap), m_rX509SerialNumber);
            }
            // missing: ds:X509SKI, ds:X509SubjectName, ds:X509CRL
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

/// can't be sure what is supposed to happen here because the spec is clear as mud
class OOXMLSecParser::DsX509DataContext
    : public OOXMLSecParser::Context
{
    private:
        // sigh... "No ordering is implied by the above constraints."
        // so store the ball of mud in vectors and try to figure it out later.
        std::vector<std::pair<OUString, OUString>> m_X509IssuerSerials;
        std::vector<OUString> m_X509Certificates;

    public:
        DsX509DataContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setX509Data(m_X509IssuerSerials, m_X509Certificates);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509IssuerSerial")
            {
                m_X509IssuerSerials.emplace_back();
                return std::make_unique<DsX509IssuerSerialContext>(m_rParser, std::move(pOldNamespaceMap), m_X509IssuerSerials.back().first, m_X509IssuerSerials.back().second);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509Certificate")
            {
                m_X509Certificates.emplace_back();
                return std::make_unique<DsX509CertificateContext>(m_rParser, std::move(pOldNamespaceMap), m_X509Certificates.back());
            }
            // missing: ds:X509SKI, ds:X509SubjectName, ds:X509CRL
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsKeyInfoContext
    : public OOXMLSecParser::Context
{
    public:
        DsKeyInfoContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509Data")
            {
                return std::make_unique<DsX509DataContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: ds:PGPData
            // missing: ds:KeyName, ds:KeyValue, ds:RetrievalMethod, ds:SPKIData, ds:MgmtData
            // (old code would read ds:Transform inside ds:RetrievalMethod but
            // presumably that was a bug)
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }

};

class OOXMLSecParser::DsSignatureValueContext
    : public OOXMLSecParser::Context
{
    private:
        OUString m_Value;

    public:
        DsSignatureValueContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setSignatureValue(m_Value);
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class OOXMLSecParser::DsDigestValueContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsDigestValueContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString & rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& /*xAttrs*/) override
        {
            m_rValue.clear();
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::DsDigestMethodContext
    : public OOXMLSecParser::Context
{
    private:
        sal_Int32 & m_rReferenceDigestID;

    public:
        DsDigestMethodContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                sal_Int32& rReferenceDigestID)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rReferenceDigestID(rReferenceDigestID)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString ouAlgorithm = xAttrs->getValueByName("Algorithm");

            SAL_WARN_IF( ouAlgorithm.isEmpty(), "xmlsecurity.helper", "no Algorithm in Reference" );
            if (ouAlgorithm.isEmpty())
                return;

            SAL_WARN_IF( ouAlgorithm != ALGO_XMLDSIGSHA1
                         && ouAlgorithm != ALGO_XMLDSIGSHA256
                         && ouAlgorithm != ALGO_XMLDSIGSHA512,
                         "xmlsecurity.helper", "Algorithm neither SHA1, SHA256 nor SHA512");
            if (ouAlgorithm == ALGO_XMLDSIGSHA1)
                m_rReferenceDigestID = css::xml::crypto::DigestID::SHA1;
            else if (ouAlgorithm == ALGO_XMLDSIGSHA256)
                m_rReferenceDigestID = css::xml::crypto::DigestID::SHA256;
            else if (ouAlgorithm == ALGO_XMLDSIGSHA512)
                m_rReferenceDigestID = css::xml::crypto::DigestID::SHA512;
            else
                m_rReferenceDigestID = 0;
        }
};

class OOXMLSecParser::DsTransformContext
    : public OOXMLSecParser::Context
{
    private:
        bool & m_rIsC14N;

    public:
        DsTransformContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool& rIsC14N)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rIsC14N(rIsC14N)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString aAlgorithm = xAttrs->getValueByName("Algorithm");

            if (aAlgorithm == ALGO_RELATIONSHIP)
            {
                m_rIsC14N = true;
            }
        }
};

class OOXMLSecParser::DsTransformsContext
    : public OOXMLSecParser::Context
{
    private:
        bool & m_rIsC14N;

    public:
        DsTransformsContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool& rIsC14N)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rIsC14N(rIsC14N)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "Transform")
            {
                return std::make_unique<DsTransformContext>(m_rParser, std::move(pOldNamespaceMap), m_rIsC14N);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsReferenceContext
    : public OOXMLSecParser::Context
{
    private:
        OUString m_URI;
        OUString m_Type;
        OUString m_DigestValue;
        bool m_IsC14N = false;
        // Relevant for ODF. The digest algorithm selected by the DigestMethod
        // element's Algorithm attribute. @see css::xml::crypto::DigestID.
        sal_Int32 m_nReferenceDigestID = css::xml::crypto::DigestID::SHA256;

    public:
        DsReferenceContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);

            m_URI = xAttrs->getValueByName("URI");
            SAL_WARN_IF(m_URI.isEmpty(), "xmlsecurity.helper", "URI is empty");
            // Remember the type of this reference.
            m_Type = xAttrs->getValueByName("Type");
        }

        virtual void EndElement() override
        {
            if (m_URI.startsWith("#"))
            {
                /*
                * remove the first character '#' from the attribute value
                */
                m_rParser.m_pXSecController->addReference(m_URI.copy(1), m_nReferenceDigestID, m_Type);
            }
            else
            {
                if (m_IsC14N) // this is determined by nested ds:Transform
                {
                    m_rParser.m_pXSecController->addStreamReference(m_URI, false, m_nReferenceDigestID);
                }
                else
            /*
            * it must be an octet stream
            */
                {
                    m_rParser.m_pXSecController->addStreamReference(m_URI, true, m_nReferenceDigestID);
                }
            }

            m_rParser.m_pXSecController->setDigestValue(m_nReferenceDigestID, m_DigestValue);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "Transforms")
            {
                return std::make_unique<DsTransformsContext>(m_rParser, std::move(pOldNamespaceMap), m_IsC14N);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "DigestMethod")
            {
                return std::make_unique<DsDigestMethodContext>(m_rParser, std::move(pOldNamespaceMap), m_nReferenceDigestID);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "DigestValue")
            {
                return std::make_unique<DsDigestValueContext>(m_rParser, std::move(pOldNamespaceMap), m_DigestValue);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsSignatureMethodContext
    : public OOXMLSecParser::Context
{
    public:
        DsSignatureMethodContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString ouAlgorithm = xAttrs->getValueByName("Algorithm");
            if (ouAlgorithm == ALGO_ECDSASHA1 || ouAlgorithm == ALGO_ECDSASHA256
                || ouAlgorithm == ALGO_ECDSASHA512)
            {
                m_rParser.m_pXSecController->setSignatureMethod(svl::crypto::SignatureMethodAlgorithm::ECDSA);
            }
        }
};

class OOXMLSecParser::DsSignedInfoContext
    : public OOXMLSecParser::Context
{
    public:
        DsSignedInfoContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setReferenceCount();
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignatureMethod")
            {
                return std::make_unique<DsSignatureMethodContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "Reference")
            {
                return std::make_unique<DsReferenceContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: ds:CanonicalizationMethod
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesCertDigestContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rDigestValue;
        sal_Int32 & m_rReferenceDigestID;

    public:
        XadesCertDigestContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rDigestValue, sal_Int32& rReferenceDigestID)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rDigestValue(rDigestValue)
            , m_rReferenceDigestID(rReferenceDigestID)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "DigestMethod")
            {
                return std::make_unique<DsDigestMethodContext>(m_rParser, std::move(pOldNamespaceMap), m_rReferenceDigestID);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "DigestValue")
            {
                return std::make_unique<DsDigestValueContext>(m_rParser, std::move(pOldNamespaceMap), m_rDigestValue);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesCertContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    private:
        sal_Int32 m_nReferenceDigestID = css::xml::crypto::DigestID::SHA1;
        OUString m_CertDigest;
        OUString m_X509IssuerName;
        OUString m_X509SerialNumber;

    public:
        XadesCertContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                m_rParser.m_pXSecController->setX509CertDigest(m_CertDigest, m_nReferenceDigestID, m_X509IssuerName, m_X509SerialNumber);
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned xades:Cert");
            }
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "CertDigest")
            {
                return std::make_unique<XadesCertDigestContext>(m_rParser, std::move(pOldNamespaceMap), m_CertDigest, m_nReferenceDigestID);
            }
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "IssuerSerial")
            {
                return std::make_unique<DsX509IssuerSerialContext>(m_rParser, std::move(pOldNamespaceMap), m_X509IssuerName, m_X509SerialNumber);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesSigningCertificateContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        XadesSigningCertificateContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "Cert")
            {
                return std::make_unique<XadesCertContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesSigningTimeContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    private:
        OUString m_Value;

    public:
        XadesSigningTimeContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                m_rParser.m_pXSecController->setDate("", m_Value);
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SigningTime");
            }
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class OOXMLSecParser::XadesSignedSignaturePropertiesContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        XadesSignedSignaturePropertiesContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SigningTime")
            {
                return std::make_unique<XadesSigningTimeContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SigningCertificate")
            {
                return std::make_unique<XadesSigningCertificateContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            // missing: xades:SignaturePolicyIdentifier, xades:SignatureProductionPlace, xades:SignerRole
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesSignedPropertiesContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        XadesSignedPropertiesContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SignedSignatureProperties")
            {
                return std::make_unique<XadesSignedSignaturePropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            // missing: xades:SignedDataObjectProperties
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::XadesQualifyingPropertiesContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        XadesQualifyingPropertiesContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SignedProperties")
            {
                return std::make_unique<XadesSignedPropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            // missing: xades:UnsignedSignatureProperties
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::MsodigsigSetupIDContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        MsodigsigSetupIDContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::MsodigsigSignatureCommentsContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        MsodigsigSignatureCommentsContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::MsodigsigSignatureInfoV1Context
    : public OOXMLSecParser::ReferencedContextImpl
{
    private:
        OUString m_SetupID;
        OUString m_SignatureComments;

    public:
        MsodigsigSignatureInfoV1Context(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_MSODIGSIG && rName == "SetupID")
            {
                return std::make_unique<MsodigsigSetupIDContext>(m_rParser, std::move(pOldNamespaceMap), m_SetupID);
            }
            if (nNamespace == XML_NAMESPACE_MSODIGSIG && rName == "SignatureComments")
            {
                return std::make_unique<MsodigsigSignatureCommentsContext>(m_rParser, std::move(pOldNamespaceMap), m_SignatureComments);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                if (!m_SetupID.isEmpty())
                {
                    m_rParser.m_pXSecController->setSignatureLineId(m_SetupID);
                }
                if (!m_SignatureComments.isEmpty())
                {
                    m_rParser.m_pXSecController->setDescription("", m_SignatureComments);

                }
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureInfoV1");
            }
        }
};

class OOXMLSecParser::MdssiValueContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        MdssiValueContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class OOXMLSecParser::MdssiSignatureTimeContext
    : public OOXMLSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        MdssiSignatureTimeContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                OUString& rValue)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_MDSSI && rName == "Value")
            {
                return std::make_unique<MdssiValueContext>(m_rParser, std::move(pOldNamespaceMap), m_rValue);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};


class OOXMLSecParser::DsSignaturePropertyContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    private:
        enum class SignatureProperty { Unknown, Date, Info };
        SignatureProperty m_Property = SignatureProperty::Unknown;
        OUString m_Id;
        OUString m_Value;

    public:
        DsSignaturePropertyContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_Id = CheckIdAttrReferenced(xAttrs);
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                switch (m_Property)
                {
                    case SignatureProperty::Unknown:
                        SAL_INFO("xmlsecurity.helper", "Unknown property in ds:Object ignored");
                        break;
                    case SignatureProperty::Info:
                        break; // handled by child context
                    case SignatureProperty::Date:
                        m_rParser.m_pXSecController->setDate(m_Id, m_Value);
                        break;
                }
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureProperty");
            }
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_MDSSI && rName == "SignatureTime")
            {
                m_Property = SignatureProperty::Date;
                return std::make_unique<MdssiSignatureTimeContext>(m_rParser, std::move(pOldNamespaceMap), m_Value);
            }
            if (nNamespace == XML_NAMESPACE_MSODIGSIG && rName == "SignatureInfoV1")
            {
                return std::make_unique<MsodigsigSignatureInfoV1Context>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsSignaturePropertiesContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        DsSignaturePropertiesContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignatureProperty")
            {
                return std::make_unique<DsSignaturePropertyContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsManifestContext
    : public OOXMLSecParser::ReferencedContextImpl
{
    public:
        DsManifestContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

#if 0
        ???
        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setReferenceCount();
        }
#endif

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "Reference")
            {
                return std::make_unique<DsReferenceContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: ds:CanonicalizationMethod
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsObjectContext
    : public OOXMLSecParser::ReferencedContextImpl
{
        enum class Mode { Default, ValidSignatureLineImage, InvalidSignatureLineImage };
        Mode m_Mode = Mode::Default;
        OUString m_Value;

    public:
        DsObjectContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            // init with "false" here - the Signature element can't be referenced by its child
            : OOXMLSecParser::ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), false)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString const id(CheckIdAttrReferenced(xAttrs));
            if (id == "idValidSigLnImg")
            {
                m_Mode = Mode::ValidSignatureLineImage;
            }
            else if (id == "idInvalidSigLnImg")
            {
                m_Mode = Mode::InvalidSignatureLineImage;
            }
        }

        virtual void EndElement() override
        {
            switch (m_Mode)
            {
                case Mode::ValidSignatureLineImage:
                    if (m_isReferenced)
                    {
                        m_rParser.m_pXSecController->setValidSignatureImage(m_Value);
                    }
                    else
                    {
                        SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureLineValidImage");
                    }
                    break;
                case Mode::InvalidSignatureLineImage:
                    if (m_isReferenced)
                    {
                        m_rParser.m_pXSecController->setInvalidSignatureImage(m_Value);
                    }
                    else
                    {
                        SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureLineInvalidImage");
                    }
                    break;
                case Mode::Default:
                    break;
            }
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignatureProperties")
            {
                return std::make_unique<DsSignaturePropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "QualifyingProperties")
            {
                return std::make_unique<XadesQualifyingPropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "Manifest")
            {
                return std::make_unique<DsManifestContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class OOXMLSecParser::DsSignatureContext
    : public OOXMLSecParser::Context
{
    public:
        DsSignatureContext(OOXMLSecParser& rParser,
                std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap)
            : OOXMLSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString const ouIdAttr(m_rParser.HandleIdAttr(xAttrs));
            m_rParser.m_rXMLSignatureHelper.StartVerifySignatureElement();
            m_rParser.m_pXSecController->addSignature();
            if (!ouIdAttr.isEmpty())
            {
                m_rParser.m_pXSecController->setId( ouIdAttr );
            }
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::optional<SvXMLNamespaceMap>&& pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignedInfo")
            {
                return std::make_unique<DsSignedInfoContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignatureValue")
            {
                return std::make_unique<DsSignatureValueContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "KeyInfo")
            {
                return std::make_unique<DsKeyInfoContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "Object")
            {
                return std::make_unique<DsObjectContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            return OOXMLSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};


OOXMLSecParser::OOXMLSecParser(XMLSignatureHelper& rXMLSignatureHelper, XSecController* pXSecController)
    : m_pNamespaceMap(SvXMLNamespaceMap())
    , m_pXSecController(pXSecController)
    ,m_rXMLSignatureHelper(rXMLSignatureHelper)
{
    using namespace xmloff::token;
    m_pNamespaceMap->Add( GetXMLToken(XML_XML), GetXMLToken(XML_N_XML), XML_NAMESPACE_XML );
    m_pNamespaceMap->Add( "_ds", GetXMLToken(XML_N_DS), XML_NAMESPACE_DS );
    m_pNamespaceMap->Add( "_xades132", GetXMLToken(XML_N_XADES132), XML_NAMESPACE_XADES132);
    m_pNamespaceMap->Add( "_xades141", GetXMLToken(XML_N_XADES141), XML_NAMESPACE_XADES141);
    m_pNamespaceMap->Add( "_dc", GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
    m_pNamespaceMap->Add( "_mdssi", NS_MDSSI, XML_NAMESPACE_MDSSI );
    m_pNamespaceMap->Add( "_msodigsig", "http://schemas.microsoft.com/office/2006/digsig", XML_NAMESPACE_MSODIGSIG );
    m_pNamespaceMap->Add( "_office_libo",
                         GetXMLToken(XML_N_LO_EXT), XML_NAMESPACE_LO_EXT);
}

OOXMLSecParser::~OOXMLSecParser()
{
}

OUString OOXMLSecParser::HandleIdAttr(css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs)
{
    OUString const aId = xAttrs->getValueByName("Id");
    if (!aId.isEmpty())
    {
        m_pXSecController->collectToVerify(aId);
    }
    return aId;
}

void SAL_CALL OOXMLSecParser::startDocument()
{
    if (m_xNextHandler.is())
        m_xNextHandler->startDocument();
}

void SAL_CALL OOXMLSecParser::endDocument()
{
    if (m_xNextHandler.is())
        m_xNextHandler->endDocument();
}

void SAL_CALL OOXMLSecParser::startElement(const OUString& rName, const uno::Reference<xml::sax::XAttributeList>& xAttribs)
{
    assert(m_pNamespaceMap);
    std::optional<SvXMLNamespaceMap> pRewindMap(
        SvXMLImport::processNSAttributes(m_pNamespaceMap, nullptr, xAttribs));

    OUString localName;
    sal_uInt16 const nPrefix(m_pNamespaceMap->GetKeyByAttrName(rName, &localName));

    std::unique_ptr<Context> pContext;

    if (m_ContextStack.empty())
    {
        if (nPrefix != XML_NAMESPACE_DS || localName != "Signature")
        {
            throw css::xml::sax::SAXException(
                "xmlsecurity: unexpected root element", nullptr,
                css::uno::Any());
        }

        pContext.reset(new DsSignatureContext(*this, std::move(pRewindMap)));

    }
    else
    {
        pContext = m_ContextStack.top()->CreateChildContext(
                std::move(pRewindMap), nPrefix, localName);
    }

    m_ContextStack.push(std::move(pContext));

    m_ContextStack.top()->StartElement(xAttribs);

    if (m_xNextHandler.is())
    {
        m_xNextHandler->startElement(rName, xAttribs);
    }

}

void SAL_CALL OOXMLSecParser::endElement(const OUString& rName)
{
    assert(!m_ContextStack.empty()); // this should be checked by sax parser?

    m_ContextStack.top()->EndElement();

    if (m_xNextHandler.is())
    {
        m_xNextHandler->endElement(rName);
    }

    if (m_ContextStack.top()->m_pOldNamespaceMap)
    {
        m_pNamespaceMap = std::move(m_ContextStack.top()->m_pOldNamespaceMap);
    }
    m_ContextStack.pop();
}

void SAL_CALL OOXMLSecParser::characters(const OUString& rChars)
{
    assert(!m_ContextStack.empty()); // this should be checked by sax parser?
    m_ContextStack.top()->Characters(rChars);

    if (m_xNextHandler.is())
        m_xNextHandler->characters(rChars);
}

void SAL_CALL OOXMLSecParser::ignorableWhitespace(const OUString& rWhitespace)
{
    if (m_xNextHandler.is())
        m_xNextHandler->ignorableWhitespace(rWhitespace);
}

void SAL_CALL OOXMLSecParser::processingInstruction(const OUString& rTarget, const OUString& rData)
{
    if (m_xNextHandler.is())
        m_xNextHandler->processingInstruction(rTarget, rData);
}

void SAL_CALL OOXMLSecParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& xLocator)
{
    if (m_xNextHandler.is())
        m_xNextHandler->setDocumentLocator(xLocator);
}

void SAL_CALL OOXMLSecParser::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    rArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
