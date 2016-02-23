/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/classificationhelper.hxx>

#include <map>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

#include <sfx2/objsh.hxx>
#include <o3tl/make_unique.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <config_folders.h>

using namespace com::sun::star;

namespace
{

/// Represents one category of a classification policy.
class SfxClassificationCategory
{
public:
    std::map<OUString, OUString> m_aLabels;
};

/// Parses a policy XML conforming to the TSCP BAF schema.
class SfxClassificationParser : public cppu::WeakImplHelper<xml::sax::XDocumentHandler>
{
public:
    std::map<OUString, SfxClassificationCategory> m_aCategories;

    OUString m_aPolicyAuthorityName;
    bool m_bInPolicyAuthorityName;
    OUString m_aPolicyName;
    bool m_bInPolicyName;
    OUString m_aProgramID;
    bool m_bInProgramID;
    OUString m_aScale;
    bool m_bInScale;
    OUString m_aConfidentalityValue;
    bool m_bInConfidentalityValue;
    OUString m_aIdentifier;
    bool m_bInIdentifier;
    OUString m_aValue;
    bool m_bInValue;

    /// Pointer to a value in m_aCategories, the currently parsed category.
    SfxClassificationCategory* m_pCategory;

    SfxClassificationParser();
    virtual ~SfxClassificationParser();

    virtual void SAL_CALL startDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL startElement(const OUString& aName, const uno::Reference<xml::sax::XAttributeList>& xAttribs)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL endElement(const OUString& aName) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL characters(const OUString& aChars) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDocumentLocator(const uno::Reference<xml::sax::XLocator>& xLocator)
    throw (xml::sax::SAXException, uno::RuntimeException, std::exception) override;
};

SfxClassificationParser::SfxClassificationParser()
    : m_bInPolicyAuthorityName(false)
    , m_bInPolicyName(false)
    , m_bInProgramID(false)
    , m_bInConfidentalityValue(false)
    , m_bInIdentifier(false)
    , m_bInValue(false)
    , m_pCategory(nullptr)
{
}

SfxClassificationParser::~SfxClassificationParser()
{
}

void SAL_CALL SfxClassificationParser::startDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL SfxClassificationParser::endDocument() throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL SfxClassificationParser::startElement(const OUString& rName, const uno::Reference<xml::sax::XAttributeList>& xAttribs)
throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (rName == "baf:PolicyAuthorityName")
    {
        m_aPolicyAuthorityName.clear();
        m_bInPolicyAuthorityName = true;
    }
    else if (rName == "baf:PolicyName")
    {
        m_aPolicyName.clear();
        m_bInPolicyName = true;
    }
    else if (rName == "baf:ProgramID")
    {
        m_aProgramID.clear();
        m_bInProgramID = true;
    }
    else if (rName == "baf:BusinessAuthorizationCategory")
    {
        OUString aName = xAttribs->getValueByName("Name");
        if (!m_pCategory && !aName.isEmpty())
        {
            // Create a new category and initialize it with the data that's true for all categories.
            SfxClassificationCategory& rCategory = m_aCategories[aName];
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Name"] = m_aPolicyAuthorityName;
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Policy:Name"] = m_aPolicyName;

            // Also initialize defaults.
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Identifier"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Country"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Policy:Identifier"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Name"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Identifier"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Locator"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Name"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier:OID"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Locator"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Locator"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:MarkingPrecedence"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-summary"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:2"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:3"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:4"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:2"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:3"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:4"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:document-footer"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:document-header"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:document-watermark"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-first-line-of-text"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-last-line-of-text"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-subject-prefix"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-subject-suffix"] = "";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Authorization:StartValidity"] = "None";
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Authorization:StopValidity"] = "None";
            m_pCategory = &rCategory;
        }
    }
    else if (rName == "baf:Scale")
    {
        m_aScale.clear();
        m_bInScale = true;
    }
    else if (rName == "baf:ConfidentalityValue")
    {
        m_aConfidentalityValue.clear();
        m_bInConfidentalityValue = true;
    }
    else if (rName == "baf:Identifier")
    {
        m_aIdentifier.clear();
        m_bInIdentifier = true;
    }
    else if (rName == "baf:Value")
    {
        m_aValue.clear();
        m_bInValue = true;
    }
}

void SAL_CALL SfxClassificationParser::endElement(const OUString& rName) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (rName == "baf:PolicyAuthorityName")
        m_bInPolicyAuthorityName = false;
    else if (rName == "baf:PolicyName")
        m_bInPolicyName = false;
    else if (rName == "baf:ProgramID")
        m_bInProgramID = false;
    else if (rName == "baf:BusinessAuthorizationCategory")
        m_pCategory = nullptr;
    else if (rName == "baf:Scale")
    {
        m_bInScale = false;
        if (m_pCategory)
            m_pCategory->m_aLabels["urn:bails:IntellectualProperty:Impact:Scale"] = m_aScale;
    }
    else if (rName == "baf:ConfidentalityValue")
    {
        m_bInConfidentalityValue = false;
        if (m_pCategory)
        {
            std::map<OUString, OUString>& rLabels = m_pCategory->m_aLabels;
            rLabels["urn:bails:IntellectualProperty:Impact:Level:Confidentiality"] = m_aConfidentalityValue;
            // Set the two other type of levels as well, if they're not set
            // yet: they're optional in BAF, but not in BAILS.
            if (rLabels.find("urn:bails:IntellectualProperty:Impact:Level:Integrity") == rLabels.end())
                rLabels["urn:bails:IntellectualProperty:Impact:Level:Integrity"] = m_aConfidentalityValue;
            if (rLabels.find("urn:bails:IntellectualProperty:Impact:Level:Availability") == rLabels.end())
                rLabels["urn:bails:IntellectualProperty:Impact:Level:Availability"] = m_aConfidentalityValue;
        }
    }
    else if (rName == "baf:Identifier")
        m_bInIdentifier = false;
    else if (rName == "baf:Value")
    {
        if (m_pCategory)
        {
            if (m_aIdentifier == "Document: Header")
                m_pCategory->m_aLabels["urn:bails:IntellectualProperty:Marking:document-header"] = m_aValue;
            else if (m_aIdentifier == "Document: Footer")
                m_pCategory->m_aLabels["urn:bails:IntellectualProperty:Marking:document-footer"] = m_aValue;
            else if (m_aIdentifier == "Document: Watermark")
                m_pCategory->m_aLabels["urn:bails:IntellectualProperty:Marking:document-watermark"] = m_aValue;
        }
    }
}

void SAL_CALL SfxClassificationParser::characters(const OUString& rChars) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
    if (m_bInPolicyAuthorityName)
        m_aPolicyAuthorityName += rChars;
    else if (m_bInPolicyName)
        m_aPolicyName += rChars;
    else if (m_bInProgramID)
        m_aProgramID += rChars;
    else if (m_bInScale)
        m_aScale += rChars;
    else if (m_bInConfidentalityValue)
        m_aConfidentalityValue += rChars;
    else if (m_bInIdentifier)
        m_aIdentifier += rChars;
    else if (m_bInValue)
        m_aValue += rChars;
}

void SAL_CALL SfxClassificationParser::ignorableWhitespace(const OUString& /*rWhitespace*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL SfxClassificationParser::processingInstruction(const OUString& /*rTarget*/, const OUString& /*rData*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL SfxClassificationParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& /*xLocator*/) throw (xml::sax::SAXException, uno::RuntimeException, std::exception)
{
}

} // anonymous namespace

/// Implementation details of SfxClassificationHelper.
class SfxClassificationHelper::Impl
{
public:
    std::map<OUString, OUString> m_aLabels;
    /// Possible categories of a policy to choose from.
    std::map<OUString, SfxClassificationCategory> m_aCategories;

    void parsePolicy();
};

void SfxClassificationHelper::Impl::parsePolicy()
{
    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/classification/example.xml");
    rtl::Bootstrap::expandMacros(aPath);
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aPath, StreamMode::READ);
    uno::Reference<io::XInputStream> xInputStream(new utl::OStreamWrapper(*pStream));
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(comphelper::getProcessComponentContext());
    rtl::Reference<SfxClassificationParser> xClassificationParser(new SfxClassificationParser());
    uno::Reference<xml::sax::XDocumentHandler> xHandler(xClassificationParser.get());
    xParser->setDocumentHandler(xHandler);
    try
    {
        xParser->parseStream(aParserInput);
    }
    catch (const xml::sax::SAXParseException& rException)
    {
        SAL_WARN("sfx.view", "parsePolicy() failed: " << rException.Message);
    }
    m_aCategories = xClassificationParser->m_aCategories;
}

bool SfxClassificationHelper::IsClassified(SfxObjectShell& rObjectShell)
{
    uno::Reference<document::XDocumentProperties> xDocumentProperties = rObjectShell.getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return false;

    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties = xPropertySet->getPropertySetInfo()->getProperties();
    for (const beans::Property& rProperty : aProperties)
    {
        if (rProperty.Name.startsWith("urn:bails:"))
            return true;
    }

    return false;
}

SfxClassificationHelper::SfxClassificationHelper(SfxObjectShell& rObjectShell)
    : m_pImpl(o3tl::make_unique<Impl>())
{
    uno::Reference<document::XDocumentProperties> xDocumentProperties = rObjectShell.getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return;

    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties = xPropertySet->getPropertySetInfo()->getProperties();
    for (const beans::Property& rProperty : aProperties)
    {
        if (!rProperty.Name.startsWith("urn:bails:"))
            continue;

        uno::Any aAny = xPropertySet->getPropertyValue(rProperty.Name);
        OUString aValue;
        if (aAny >>= aValue)
            m_pImpl->m_aLabels[rProperty.Name] = aValue;
    }
}

SfxClassificationHelper::~SfxClassificationHelper()
{
}

OUString SfxClassificationHelper::GetBACName()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Name");
    if (it != m_pImpl->m_aLabels.end())
        return it->second;

    return OUString();
}

bool SfxClassificationHelper::HasImpactLevel()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Scale");
    if (it == m_pImpl->m_aLabels.end())
        return false;

    it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    if (it == m_pImpl->m_aLabels.end())
        return false;

    return true;
}

basegfx::BColor SfxClassificationHelper::GetImpactLevelColor()
{
    basegfx::BColor aRet;

    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Scale");
    if (it == m_pImpl->m_aLabels.end())
        return aRet;
    OUString aScale = it->second;

    it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    if (it == m_pImpl->m_aLabels.end())
        return aRet;
    OUString aLevel = it->second;

    // The spec defines two valid scale values: FIPS-199 and UK-Cabinet.
    if (aScale == "UK-Cabinet")
    {
        static std::map<OUString, basegfx::BColor> aColors;
        if (aColors.empty())
        {
            // Green -> brown -> orange -> red.
            aColors["0"] = basegfx::BColor(0.0, 0.5, 0.0);
            aColors["1"] = basegfx::BColor(0.5, 0.5, 0.0);
            aColors["2"] = basegfx::BColor(1.0, 0.5, 0.0);
            aColors["3"] = basegfx::BColor(0.5, 0.0, 0.0);
        }
        std::map<OUString, basegfx::BColor>::iterator itColor = aColors.find(aLevel);
        if (itColor == aColors.end())
            return aRet;
        aRet = itColor->second;
    }
    else if (aScale == "FIPS-199")
    {
        static std::map<OUString, basegfx::BColor> aColors;
        if (aColors.empty())
        {
            // Green -> orange -> red.
            aColors["Low"] = basegfx::BColor(0.0, 0.5, 0.0);
            aColors["Moderate"] = basegfx::BColor(1.0, 0.5, 0.0);
            aColors["High"] = basegfx::BColor(0.5, 0.0, 0.0);
        }
        std::map<OUString, basegfx::BColor>::iterator itColor = aColors.find(aLevel);
        if (itColor == aColors.end())
            return aRet;
        aRet = itColor->second;
    }

    return aRet;
}

OUString SfxClassificationHelper::GetDocumentWatermark()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Marking:document-watermark");
    if (it != m_pImpl->m_aLabels.end())
        return it->second;

    return OUString();
}

void SfxClassificationHelper::SetBACName(const OUString& rName)
{
    if (m_pImpl->m_aCategories.empty())
        m_pImpl->parsePolicy();

    std::map<OUString, SfxClassificationCategory>::iterator it = m_pImpl->m_aCategories.find(rName);
    if (it == m_pImpl->m_aCategories.end())
    {
        SAL_WARN("sfx.view", "'" << rName << "' is not a recognized category name");
        return;
    }

    m_pImpl->m_aLabels = it->second.m_aLabels;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
