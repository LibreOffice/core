/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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


#include "xsecparser.hxx"
#include <xsecctl.hxx>
#include <xmlsignaturehelper.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlimp.hxx>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <sal/log.hxx>

class XSecParser::Context
{
    protected:
        friend class XSecParser;
        XSecParser & m_rParser;
    private:
        std::unique_ptr<SvXMLNamespaceMap> m_pOldNamespaceMap;

    public:
        Context(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const /*nNamespace*/, OUString const& /*rName*/);

        virtual void Characters(OUString const& /*rChars*/)
        {
        }
};

// it's possible that an unsupported element has an Id attribute and a
// ds:Reference digesting it - probably this means XSecController needs to know
// about it. (For known elements, the Id attribute is only processed according
// to the schema.)
class XSecParser::UnknownContext
    : public XSecParser::Context
{
    public:
        UnknownContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }
};

auto XSecParser::Context::CreateChildContext(
    std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
class XSecParser::ReferencedContextImpl
    : public XSecParser::Context
{
    protected:
        bool m_isReferenced;

    public:
        ReferencedContextImpl(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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

class XSecParser::LoPGPOwnerContext
    : public XSecParser::Context
{
    private:
        OUString m_Value;

    public:
        LoPGPOwnerContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setGpgOwner(m_Value);
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::DsPGPKeyPacketContext
    : public XSecParser::Context
{
    private:
        OUString m_Value;

    public:
        DsPGPKeyPacketContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setGpgCertificate(m_Value);
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::DsPGPKeyIDContext
    : public XSecParser::Context
{
    private:
        OUString m_Value;

    public:
        DsPGPKeyIDContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setGpgKeyID(m_Value);
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::DsPGPDataContext
    : public XSecParser::Context
{
    public:
        DsPGPDataContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& /*xAttrs*/) override
        {
            m_rParser.m_pXSecController->switchGpgSignature();
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "PGPKeyID")
            {
                return std::make_unique<DsPGPKeyIDContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "PGPKeyPacket")
            {
                return std::make_unique<DsPGPKeyPacketContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_LO_EXT && rName == "PGPOwner")
            {
                return std::make_unique<LoPGPOwnerContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsX509CertificateContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509CertificateContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class XSecParser::DsX509SerialNumberContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509SerialNumberContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class XSecParser::DsX509IssuerNameContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsX509IssuerNameContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class XSecParser::DsX509IssuerSerialContext
    : public XSecParser::Context
{
    private:
        OUString & m_rX509IssuerName;
        OUString & m_rX509SerialNumber;

    public:
        DsX509IssuerSerialContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rIssuerName, OUString & rSerialNumber)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rX509IssuerName(rIssuerName)
            , m_rX509SerialNumber(rSerialNumber)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

/// can't be sure what is supposed to happen here because the spec is clear as mud
class XSecParser::DsX509DataContext
    : public XSecParser::Context
{
    private:
        // sigh... "No ordering is implied by the above constraints."
        // so store the ball of mud in vectors and try to figure it out later.
        std::vector<std::pair<OUString, OUString>> m_X509IssuerSerials;
        std::vector<OUString> m_X509Certificates;

    public:
        DsX509DataContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->setX509Data(m_X509IssuerSerials, m_X509Certificates);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsKeyInfoContext
    : public XSecParser::Context
{
    public:
        DsKeyInfoContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "X509Data")
            {
                return std::make_unique<DsX509DataContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            if (nNamespace == XML_NAMESPACE_DS && rName == "PGPData")
            {
                return std::make_unique<DsPGPDataContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: ds:KeyName, ds:KeyValue, ds:RetrievalMethod, ds:SPKIData, ds:MgmtData
            // (old code would read ds:Transform inside ds:RetrievalMethod but
            // presumably that was a bug)
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }

};

class XSecParser::DsSignatureValueContext
    : public XSecParser::Context
{
    private:
        OUString m_Value;

    public:
        DsSignatureValueContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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

class XSecParser::DsDigestValueContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DsDigestValueContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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

class XSecParser::DsDigestMethodContext
    : public XSecParser::Context
{
    private:
        sal_Int32 & m_rReferenceDigestID;

    public:
        DsDigestMethodContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                sal_Int32 & rReferenceDigestID)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rReferenceDigestID(rReferenceDigestID)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString ouAlgorithm = xAttrs->getValueByName("Algorithm");

            SAL_WARN_IF( ouAlgorithm.isEmpty(), "xmlsecurity.helper", "no Algorithm in Reference" );
            if (!ouAlgorithm.isEmpty())
            {
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
        }
};

class XSecParser::DsTransformContext
    : public XSecParser::Context
{
    private:
        bool & m_rIsC14N;

    public:
        DsTransformContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool & rIsC14N)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rIsC14N(rIsC14N)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            OUString ouAlgorithm = xAttrs->getValueByName("Algorithm");

            if (ouAlgorithm == ALGO_C14N)
                /*
                 * a xml stream
                 */
            {
                m_rIsC14N = true;
            }
        }
};

class XSecParser::DsTransformsContext
    : public XSecParser::Context
{
    private:
        bool & m_rIsC14N;

    public:
        DsTransformsContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool & rIsC14N)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rIsC14N(rIsC14N)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "Transform")
            {
                return std::make_unique<DsTransformContext>(m_rParser, std::move(pOldNamespaceMap), m_rIsC14N);
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsReferenceContext
    : public XSecParser::Context
{
    private:
        OUString m_URI;
        OUString m_Type;
        OUString m_DigestValue;
        bool m_IsC14N = false;
        // Relevant for ODF. The digest algorithm selected by the DigestMethod
        // element's Algorithm attribute. @see css::xml::crypto::DigestID.
        sal_Int32 m_nReferenceDigestID = css::xml::crypto::DigestID::SHA1;

    public:
        DsReferenceContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsSignatureMethodContext
    : public XSecParser::Context
{
    public:
        DsSignatureMethodContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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

class XSecParser::DsSignedInfoContext
    : public XSecParser::Context
{
    public:
        DsSignedInfoContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesEncapsulatedX509CertificateContext
    : public XSecParser::Context
{
    private:
        OUString m_Value;

    public:
        XadesEncapsulatedX509CertificateContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual void EndElement() override
        {
            m_rParser.m_pXSecController->addEncapsulatedX509Certificate(m_Value);
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::XadesCertificateValuesContext
    : public XSecParser::Context
{
    public:
        XadesCertificateValuesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "EncapsulatedX509Certificate")
            {
                return std::make_unique<XadesEncapsulatedX509CertificateContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: xades:OtherCertificate
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesUnsignedSignaturePropertiesContext
    : public XSecParser::Context
{
    public:
        XadesUnsignedSignaturePropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "CertificateValues")
            {
                return std::make_unique<XadesCertificateValuesContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing:
            // xades:CounterSignature
            //  ^ old code would read a ds:Signature inside it?
            // xades:SignatureTimeStamp
            // xades:CompleteCertificateRefs
            // xades:CompleteRevocationRefs
            // xades:AttributeCertificateRefs
            // xades:AttributeRevocationRefs
            // xades:SigAndRefsTimeStamp
            // xades:RefsOnlyTimeStamp
            // xades:RevocationValues
            // xades:AttrAuthoritiesCertValues
            //  ^ old code: was equivalent to CertificateValues ???
            // xades:AttributeRevocationValues
            // xades:ArchiveTimeStamp
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesUnsignedPropertiesContext
    : public XSecParser::Context
{
    public:
        XadesUnsignedPropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            m_rParser.HandleIdAttr(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "UnsignedSignatureProperties")
            {
                return std::make_unique<XadesUnsignedSignaturePropertiesContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            // missing: xades:UnsignedDataObjectProperties
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::LoSignatureLineIdContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        OUString m_Value;

    public:
        LoSignatureLineIdContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                m_rParser.m_pXSecController->setSignatureLineId(m_Value);
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureLineId");
            }
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::LoSignatureLineValidImageContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        OUString m_Value;

    public:
        LoSignatureLineValidImageContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                m_rParser.m_pXSecController->setValidSignatureImage(m_Value);
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureLineValidImage");
            }
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::LoSignatureLineInvalidImageContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        OUString m_Value;

    public:
        LoSignatureLineInvalidImageContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual void EndElement() override
        {
            if (m_isReferenced)
            {
                m_rParser.m_pXSecController->setInvalidSignatureImage(m_Value);
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureLineInvalidImage");
            }
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_Value += rChars;
        }
};

class XSecParser::LoSignatureLineContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        LoSignatureLineContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_LO_EXT && rName == "SignatureLineId")
            {
                return std::make_unique<LoSignatureLineIdContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_LO_EXT && rName == "SignatureLineValidImage")
            {
                return std::make_unique<LoSignatureLineValidImageContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_LO_EXT && rName == "SignatureLineInvalidImage")
            {
                return std::make_unique<LoSignatureLineInvalidImageContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesCertDigestContext
    : public XSecParser::Context
{
    private:
        OUString & m_rDigestValue;
        sal_Int32 & m_rReferenceDigestID;

    public:
        XadesCertDigestContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rDigestValue, sal_Int32 & rReferenceDigestID)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rDigestValue(rDigestValue)
            , m_rReferenceDigestID(rReferenceDigestID)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesCertContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        sal_Int32 m_nReferenceDigestID = css::xml::crypto::DigestID::SHA1;
        OUString m_CertDigest;
        OUString m_X509IssuerName;
        OUString m_X509SerialNumber;

    public:
        XadesCertContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesSigningCertificateContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        XadesSigningCertificateContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                bool const isReferenced)
            : ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), isReferenced)
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "Cert")
            {
                return std::make_unique<XadesCertContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesSigningTimeContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        OUString m_Value;

    public:
        XadesSigningTimeContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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

class XSecParser::XadesSignedSignaturePropertiesContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        XadesSignedSignaturePropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            if (nNamespace == XML_NAMESPACE_LO_EXT && rName == "SignatureLine")
            {
                return std::make_unique<LoSignatureLineContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            // missing: xades:SignaturePolicyIdentifier, xades:SignatureProductionPlace, xades:SignerRole
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesSignedPropertiesContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        XadesSignedPropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SignedSignatureProperties")
            {
                return std::make_unique<XadesSignedSignaturePropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            // missing: xades:SignedDataObjectProperties
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::XadesQualifyingPropertiesContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        XadesQualifyingPropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "SignedProperties")
            {
                return std::make_unique<XadesSignedPropertiesContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            if (nNamespace == XML_NAMESPACE_XADES132 && rName == "UnsignedProperties")
            {
                return std::make_unique<XadesUnsignedPropertiesContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DcDateContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DcDateContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class XSecParser::DcDescriptionContext
    : public XSecParser::Context
{
    private:
        OUString & m_rValue;

    public:
        DcDescriptionContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
                OUString & rValue)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
            , m_rValue(rValue)
        {
        }

        virtual void Characters(OUString const& rChars) override
        {
            m_rValue += rChars;
        }
};

class XSecParser::DsSignaturePropertyContext
    : public XSecParser::ReferencedContextImpl
{
    private:
        enum class SignatureProperty { Unknown, Date, Description };
        SignatureProperty m_Property = SignatureProperty::Unknown;
        OUString m_Id;
        OUString m_Value;

    public:
        DsSignaturePropertyContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
                    case SignatureProperty::Date:
                        m_rParser.m_pXSecController->setDate(m_Id, m_Value);
                        break;
                    case SignatureProperty::Description:
                        m_rParser.m_pXSecController->setDescription(m_Id, m_Value);
                        break;
                }
            }
            else
            {
                SAL_INFO("xmlsecurity.helper", "ignoring unsigned SignatureProperty");
            }
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DC && rName == "date")
            {
                m_Property = SignatureProperty::Date;
                return std::make_unique<DcDateContext>(m_rParser, std::move(pOldNamespaceMap), m_Value);
            }
            if (nNamespace == XML_NAMESPACE_DC && rName == "description")
            {
                m_Property = SignatureProperty::Description;
                return std::make_unique<DcDescriptionContext>(m_rParser, std::move(pOldNamespaceMap), m_Value);
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsSignaturePropertiesContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        DsSignaturePropertiesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "SignatureProperty")
            {
                return std::make_unique<DsSignaturePropertyContext>(m_rParser, std::move(pOldNamespaceMap), m_isReferenced);
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsObjectContext
    : public XSecParser::ReferencedContextImpl
{
    public:
        DsObjectContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            // init with "false" here - the Signature element can't be referenced by its child
            : XSecParser::ReferencedContextImpl(rParser, std::move(pOldNamespaceMap), false)
        {
        }

        virtual void StartElement(
            css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs) override
        {
            CheckIdAttrReferenced(xAttrs);
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            // missing: ds:Manifest
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsSignatureContext
    : public XSecParser::Context
{
    public:
        DsSignatureContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
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
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
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
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};

class XSecParser::DsigSignaturesContext
    : public XSecParser::Context
{
    public:
        DsigSignaturesContext(XSecParser & rParser,
                std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap)
            : XSecParser::Context(rParser, std::move(pOldNamespaceMap))
        {
        }

        virtual std::unique_ptr<Context> CreateChildContext(
            std::unique_ptr<SvXMLNamespaceMap> pOldNamespaceMap,
            sal_uInt16 const nNamespace, OUString const& rName) override
        {
            if (nNamespace == XML_NAMESPACE_DS && rName == "Signature")
            {
                return std::make_unique<DsSignatureContext>(m_rParser, std::move(pOldNamespaceMap));
            }
            return XSecParser::Context::CreateChildContext(std::move(pOldNamespaceMap), nNamespace, rName);
        }
};


XSecParser::XSecParser(XMLSignatureHelper& rXMLSignatureHelper,
    XSecController* pXSecController)
    : m_pNamespaceMap(new SvXMLNamespaceMap)
    , m_pXSecController(pXSecController)
    , m_rXMLSignatureHelper(rXMLSignatureHelper)
{
    using namespace xmloff::token;
    m_pNamespaceMap->Add( GetXMLToken(XML_XML), GetXMLToken(XML_N_XML), XML_NAMESPACE_XML );
    m_pNamespaceMap->Add( "_dsig_ooo", GetXMLToken(XML_N_DSIG_OOO), XML_NAMESPACE_DSIG_OOO );
    m_pNamespaceMap->Add( "_dsig", GetXMLToken(XML_N_DSIG), XML_NAMESPACE_DSIG );
    m_pNamespaceMap->Add( "_ds", GetXMLToken(XML_N_DS), XML_NAMESPACE_DS );
    m_pNamespaceMap->Add( "_xades132", GetXMLToken(XML_N_XADES132), XML_NAMESPACE_XADES132);
    m_pNamespaceMap->Add( "_xades141", GetXMLToken(XML_N_XADES141), XML_NAMESPACE_XADES141);
    m_pNamespaceMap->Add( "_dc", GetXMLToken(XML_N_DC), XML_NAMESPACE_DC );
    m_pNamespaceMap->Add( "_office_libo",
                         GetXMLToken(XML_N_LO_EXT), XML_NAMESPACE_LO_EXT);
}

OUString XSecParser::HandleIdAttr(css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs)
{
    OUString ouIdAttr = getIdAttr(xAttrs);
    if (!ouIdAttr.isEmpty())
    {
        m_pXSecController->collectToVerify( ouIdAttr );
    }
    return ouIdAttr;
}

OUString XSecParser::getIdAttr(const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
{
    OUString ouIdAttr = xAttribs->getValueByName("id");

    if (ouIdAttr.isEmpty())
    {
        ouIdAttr = xAttribs->getValueByName("Id");
    }

    return ouIdAttr;
}

/*
 * XDocumentHandler
 */
void SAL_CALL XSecParser::startDocument(  )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->startDocument();
    }
}

void SAL_CALL XSecParser::endDocument(  )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->endDocument();
    }
}

void SAL_CALL XSecParser::startElement(
    const OUString& rName,
    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
{
    assert(m_pNamespaceMap);
    std::unique_ptr<SvXMLNamespaceMap> pRewindMap(
        SvXMLImport::processNSAttributes(m_pNamespaceMap, nullptr, xAttribs));

    OUString localName;
    sal_uInt16 const nPrefix(m_pNamespaceMap->GetKeyByAttrName(rName, &localName));

    std::unique_ptr<Context> pContext;

    if (m_ContextStack.empty())
    {
        if ((nPrefix == XML_NAMESPACE_DSIG || nPrefix == XML_NAMESPACE_DSIG_OOO)
            && localName == "document-signatures")
        {
            pContext.reset(new DsigSignaturesContext(*this, std::move(pRewindMap)));
        }
        else
        {
            throw css::xml::sax::SAXException(
                "xmlsecurity: unexpected root element", nullptr,
                css::uno::Any());
        }
    }
    else
    {
        pContext = m_ContextStack.top()->CreateChildContext(
                std::move(pRewindMap), nPrefix, localName);
    }

    m_ContextStack.push(std::move(pContext));
    assert(!pRewindMap);

    try
    {
        m_ContextStack.top()->StartElement(xAttribs);

        if (m_xNextHandler.is())
        {
            m_xNextHandler->startElement(rName, xAttribs);
        }
    }
    catch (css::uno::Exception& )
    {//getCaughtException MUST be the first line in the catch block
        css::uno::Any exc =  cppu::getCaughtException();
        throw css::xml::sax::SAXException(
            "xmlsecurity: Exception in XSecParser::startElement",
            nullptr, exc);
    }
    catch (...)
    {
        throw css::xml::sax::SAXException(
            "xmlsecurity: unexpected exception in XSecParser::startElement", nullptr,
            css::uno::Any());
    }
}

void SAL_CALL XSecParser::endElement(const OUString& rName)
{
    assert(!m_ContextStack.empty()); // this should be checked by sax parser?

    try
    {
        m_ContextStack.top()->EndElement();

        if (m_xNextHandler.is())
        {
            m_xNextHandler->endElement(rName);
        }
    }
    catch (css::uno::Exception& )
    {//getCaughtException MUST be the first line in the catch block
        css::uno::Any exc =  cppu::getCaughtException();
        throw css::xml::sax::SAXException(
            "xmlsecurity: Exception in XSecParser::endElement",
            nullptr, exc);
    }
    catch (...)
    {
        throw css::xml::sax::SAXException(
            "xmlsecurity: unexpected exception in XSecParser::endElement", nullptr,
            css::uno::Any());
    }

    if (m_ContextStack.top()->m_pOldNamespaceMap)
    {
        m_pNamespaceMap = std::move(m_ContextStack.top()->m_pOldNamespaceMap);
    }
    m_ContextStack.pop();
}

void SAL_CALL XSecParser::characters(const OUString& rChars)
{
    assert(!m_ContextStack.empty()); // this should be checked by sax parser?
    m_ContextStack.top()->Characters(rChars);

    if (m_xNextHandler.is())
    {
        m_xNextHandler->characters(rChars);
    }
}

void SAL_CALL XSecParser::ignorableWhitespace( const OUString& aWhitespaces )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->ignorableWhitespace( aWhitespaces );
    }
}

void SAL_CALL XSecParser::processingInstruction( const OUString& aTarget, const OUString& aData )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->processingInstruction(aTarget, aData);
    }
}

void SAL_CALL XSecParser::setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
{
    if (m_xNextHandler.is())
    {
        m_xNextHandler->setDocumentLocator( xLocator );
    }
}

/*
 * XInitialization
 */
void SAL_CALL XSecParser::initialize(
    const css::uno::Sequence< css::uno::Any >& aArguments )
{
    aArguments[0] >>= m_xNextHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
