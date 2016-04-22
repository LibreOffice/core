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
#include <algorithm>
#include <iterator>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <sfx2/objsh.hxx>
#include <o3tl/make_unique.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/layout.hxx>
#include <config_folders.h>

using namespace com::sun::star;

namespace
{

const OUString& PROP_BACNAME()
{
    static OUString sProp("urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Name");
    return sProp;
}

const OUString& PROP_STARTVALIDITY()
{
    static OUString sProp("urn:bails:IntellectualProperty:Authorization:StartValidity");
    return sProp;
}

const OUString& PROP_NONE()
{
    static OUString sProp("None");
    return sProp;
}

const OUString& PROP_IMPACTSCALE()
{
    static OUString sProp("urn:bails:IntellectualProperty:Impact:Scale");
    return sProp;
}

const OUString& PROP_IMPACTLEVEL()
{
    static OUString sProp("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    return sProp;
}

/// Represents one category of a classification policy.
class SfxClassificationCategory
{
public:
    /// PROP_BACNAME() is stored separately for easier lookup.
    OUString m_aName;
    std::map<OUString, OUString> m_aLabels;
};

/// Parses a policy XML conforming to the TSCP BAF schema.
class SfxClassificationParser : public cppu::WeakImplHelper<xml::sax::XDocumentHandler>
{
public:
    std::vector<SfxClassificationCategory> m_aCategories;

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
    , m_bInScale(false)
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
            OUString aIdentifier = xAttribs->getValueByName("Identifier");

            // Create a new category and initialize it with the data that's true for all categories.
            m_aCategories.push_back(SfxClassificationCategory());
            SfxClassificationCategory& rCategory = m_aCategories.back();
            rCategory.m_aName = aName;
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Name"] = m_aPolicyAuthorityName;
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Policy:Name"] = m_aPolicyName;
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Identifier"] = m_aProgramID;
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier"] = aIdentifier;

            // Also initialize defaults.
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Identifier"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:PolicyAuthority:Country"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Policy:Identifier"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Name"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Locator"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Identifier:OID"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Locator"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:BusinessAuthorization:Locator"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:MarkingPrecedence"] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-summary"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:2"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:3"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-warning-statement:ext:4"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:2"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:3"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:general-distribution-statement:ext:4"].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCHEADER()].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCFOOTER()].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCWATERMARK()].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-first-line-of-text"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-last-line-of-text"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-subject-prefix"].clear();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Marking:email-subject-suffix"].clear();
            rCategory.m_aLabels[PROP_STARTVALIDITY()] = PROP_NONE();
            rCategory.m_aLabels["urn:bails:IntellectualProperty:Authorization:StopValidity"] = PROP_NONE();
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
            m_pCategory->m_aLabels[PROP_IMPACTSCALE()] = m_aScale;
    }
    else if (rName == "baf:ConfidentalityValue")
    {
        m_bInConfidentalityValue = false;
        if (m_pCategory)
        {
            std::map<OUString, OUString>& rLabels = m_pCategory->m_aLabels;
            rLabels[PROP_IMPACTLEVEL()] = m_aConfidentalityValue;
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
                m_pCategory->m_aLabels[SfxClassificationHelper::PROP_DOCHEADER()] = m_aValue;
            else if (m_aIdentifier == "Document: Footer")
                m_pCategory->m_aLabels[SfxClassificationHelper::PROP_DOCFOOTER()] = m_aValue;
            else if (m_aIdentifier == "Document: Watermark")
                m_pCategory->m_aLabels[SfxClassificationHelper::PROP_DOCWATERMARK()] = m_aValue;
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
    SfxClassificationCategory m_aCategory;
    /// Possible categories of a policy to choose from.
    std::vector<SfxClassificationCategory> m_aCategories;
    const uno::Reference<document::XDocumentProperties>& m_xDocumentProperties;

    explicit Impl(const uno::Reference<document::XDocumentProperties>& xDocumentProperties);
    void parsePolicy();
    /// Synchronize m_aLabels back to the document properties.
    void pushToDocumentProperties();
    /// Set the classification start date to the system time.
    void setStartValidity();
};

SfxClassificationHelper::Impl::Impl(const uno::Reference<document::XDocumentProperties>& xDocumentProperties)
    : m_xDocumentProperties(xDocumentProperties)
{
}

void SfxClassificationHelper::Impl::parsePolicy()
{
    uno::Reference<uno::XComponentContext> xComponentContext = comphelper::getProcessComponentContext();
    SvtPathOptions aOptions;
    OUString aPath = aOptions.GetClassificationPath();
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(aPath, StreamMode::READ);
    uno::Reference<io::XInputStream> xInputStream(new utl::OStreamWrapper(*pStream));
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(xComponentContext);
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

bool lcl_containsProperty(const uno::Sequence<beans::Property>& rProperties, const OUString& rName)
{
    return std::find_if(rProperties.begin(), rProperties.end(), [&](const beans::Property& rProperty)
    {
        return rProperty.Name == rName;
    }) != rProperties.end();
}

void SfxClassificationHelper::Impl::setStartValidity()
{
    std::map<OUString, OUString>::iterator it = m_aCategory.m_aLabels.find(PROP_STARTVALIDITY());
    if (it != m_aCategory.m_aLabels.end())
    {
        if (it->second == PROP_NONE())
        {
            // The policy left the start date unchanged, replace it with the system time.
            util::DateTime aDateTime = DateTime(DateTime::SYSTEM).GetUNODateTime();
            OUStringBuffer aBuffer = utl::toISO8601(aDateTime);
            it->second = aBuffer.toString();
        }
    }
}

void SfxClassificationHelper::Impl::pushToDocumentProperties()
{
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = m_xDocumentProperties->getUserDefinedProperties();
    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties = xPropertySet->getPropertySetInfo()->getProperties();
    std::map<OUString, OUString> aLabels = m_aCategory.m_aLabels;
    aLabels[PROP_BACNAME()] = m_aCategory.m_aName;
    for (const auto& rLabel : aLabels)
    {
        try
        {
            if (lcl_containsProperty(aProperties, rLabel.first))
                xPropertySet->setPropertyValue(rLabel.first, uno::makeAny(rLabel.second));
            else
                xPropertyContainer->addProperty(rLabel.first, beans::PropertyAttribute::REMOVABLE, uno::makeAny(rLabel.second));
        }
        catch (const uno::Exception& rException)
        {
            SAL_WARN("sfx.view", "pushDocumentProperties() failed for property " << rLabel.first << ": " << rException.Message);
        }
    }
}

bool SfxClassificationHelper::IsClassified(const uno::Reference<document::XDocumentProperties>& xDocumentProperties)
{
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

SfxClassificationCheckPasteResult SfxClassificationHelper::CheckPaste(const uno::Reference<document::XDocumentProperties>& xSource,
        const uno::Reference<document::XDocumentProperties>& xDestination)
{
    bool bSourceClassified = SfxClassificationHelper::IsClassified(xSource);
    if (!bSourceClassified)
        // No classification on the source side. Return early, regardless the
        // state of the destination side.
        return SfxClassificationCheckPasteResult::None;

    bool bDestinationClassified = SfxClassificationHelper::IsClassified(xDestination);
    if (bSourceClassified && !bDestinationClassified)
    {
        // Paste from a classified document to a non-classified one -> deny.
        return SfxClassificationCheckPasteResult::TargetDocNotClassified;
    }

    // Remaining case: paste between two classified documents.
    SfxClassificationHelper aSource(xSource);
    SfxClassificationHelper aDestination(xDestination);
    if (aSource.GetImpactScale() != aDestination.GetImpactScale())
        // It's possible to compare them if they have the same scale.
        return SfxClassificationCheckPasteResult::None;

    if (aSource.GetImpactLevel() > aDestination.GetImpactLevel())
        // Paste from a doc that has higher classification -> deny.
        return SfxClassificationCheckPasteResult::DocClassificationTooLow;

    return SfxClassificationCheckPasteResult::None;
}

bool SfxClassificationHelper::ShowPasteInfo(SfxClassificationCheckPasteResult eResult)
{
    switch (eResult)
    {
    case SfxClassificationCheckPasteResult::None:
    {
        return true;
    }
    break;
    case SfxClassificationCheckPasteResult::TargetDocNotClassified:
    {
        if (!Application::IsHeadlessModeEnabled())
            ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SfxResId(STR_TARGET_DOC_NOT_CLASSIFIED), VCL_MESSAGE_INFO)->Execute();
        return false;
    }
    break;
    case SfxClassificationCheckPasteResult::DocClassificationTooLow:
    {
        if (!Application::IsHeadlessModeEnabled())
            ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SfxResId(STR_DOC_CLASSIFICATION_TOO_LOW), VCL_MESSAGE_INFO)->Execute();
        return false;
    }
    break;
    }

    return true;
}

SfxClassificationHelper::SfxClassificationHelper(const uno::Reference<document::XDocumentProperties>& xDocumentProperties)
    : m_pImpl(o3tl::make_unique<Impl>(xDocumentProperties))
{
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
        {
            if (rProperty.Name == PROP_BACNAME())
                m_pImpl->m_aCategory.m_aName = aValue;
            else
                m_pImpl->m_aCategory.m_aLabels[rProperty.Name] = aValue;
        }
    }
}

SfxClassificationHelper::~SfxClassificationHelper()
{
}

const OUString& SfxClassificationHelper::GetBACName()
{
    return m_pImpl->m_aCategory.m_aName;
}

bool SfxClassificationHelper::HasImpactLevel()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTSCALE());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
        return false;

    it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTLEVEL());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
        return false;

    return true;
}

bool SfxClassificationHelper::HasDocumentHeader()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(SfxClassificationHelper::PROP_DOCHEADER());
    if (it == m_pImpl->m_aCategory.m_aLabels.end() || it->second.isEmpty())
        return false;

    return true;
}

bool SfxClassificationHelper::HasDocumentFooter()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(SfxClassificationHelper::PROP_DOCFOOTER());
    if (it == m_pImpl->m_aCategory.m_aLabels.end() || it->second.isEmpty())
        return false;

    return true;
}

basegfx::BColor SfxClassificationHelper::GetImpactLevelColor()
{
    basegfx::BColor aRet;

    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTSCALE());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
        return aRet;
    OUString aScale = it->second;

    it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTLEVEL());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
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

sal_Int32 SfxClassificationHelper::GetImpactLevel()
{
    sal_Int32 nRet = -1;

    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTSCALE());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
        return nRet;
    OUString aScale = it->second;

    it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTLEVEL());
    if (it == m_pImpl->m_aCategory.m_aLabels.end())
        return nRet;
    OUString aLevel = it->second;

    if (aScale == "UK-Cabinet")
    {
        sal_Int32 nValue = aLevel.toInt32();
        if (nValue < 0 || nValue > 3)
            return nRet;
        nRet = nValue;
    }
    else if (aScale == "FIPS-199")
    {
        static std::map<OUString, sal_Int32> aValues;
        if (aValues.empty())
        {
            aValues["Low"] = 0;
            aValues["Moderate"] = 1;
            aValues["High"] = 2;
        }
        std::map<OUString, sal_Int32>::iterator itValues = aValues.find(aLevel);
        if (itValues == aValues.end())
            return nRet;
        nRet = itValues->second;
    }

    return nRet;
}

OUString SfxClassificationHelper::GetImpactScale()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(PROP_IMPACTSCALE());
    if (it != m_pImpl->m_aCategory.m_aLabels.end())
        return it->second;

    return OUString();
}

OUString SfxClassificationHelper::GetDocumentWatermark()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aCategory.m_aLabels.find(SfxClassificationHelper::PROP_DOCWATERMARK());
    if (it != m_pImpl->m_aCategory.m_aLabels.end())
        return it->second;

    return OUString();
}

std::vector<OUString> SfxClassificationHelper::GetBACNames()
{
    if (m_pImpl->m_aCategories.empty())
        m_pImpl->parsePolicy();

    std::vector<OUString> aRet;
    std::transform(m_pImpl->m_aCategories.begin(), m_pImpl->m_aCategories.end(), std::back_inserter(aRet), [](const SfxClassificationCategory& rCategory)
    {
        return rCategory.m_aName;
    });
    return aRet;
}

void SfxClassificationHelper::SetBACName(const OUString& rName)
{
    if (m_pImpl->m_aCategories.empty())
        m_pImpl->parsePolicy();

    std::vector<SfxClassificationCategory>::iterator it = std::find_if(m_pImpl->m_aCategories.begin(), m_pImpl->m_aCategories.end(), [&](const SfxClassificationCategory& rCategory)
    {
        return rCategory.m_aName == rName;
    });
    if (it == m_pImpl->m_aCategories.end())
    {
        SAL_WARN("sfx.view", "'" << rName << "' is not a recognized category name");
        return;
    }

    m_pImpl->m_aCategory = *it;

    m_pImpl->setStartValidity();
    m_pImpl->pushToDocumentProperties();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    UpdateInfobar(*pViewFrame);
}

void SfxClassificationHelper::UpdateInfobar(SfxViewFrame& rViewFrame)
{
    OUString aBACName = GetBACName();
    bool bImpactLevel = HasImpactLevel();
    if (!aBACName.isEmpty() && bImpactLevel)
    {
        OUString aMessage = SfxResId(STR_CLASSIFIED_DOCUMENT);
        aMessage = aMessage.replaceFirst("%1", aBACName);
        basegfx::BColor aBackgroundColor = GetImpactLevelColor();
        basegfx::BColor aForegroundColor(1.0, 1.0, 1.0);

        rViewFrame.RemoveInfoBar("classification");
        rViewFrame.AppendInfoBar("classification", aMessage, &aBackgroundColor, &aForegroundColor, &aForegroundColor, WB_CENTER);
    }
}

const OUString& SfxClassificationHelper::PROP_DOCHEADER()
{
    static OUString sProp("urn:bails:IntellectualProperty:Marking:document-header");
    return sProp;
}

const OUString& SfxClassificationHelper::PROP_DOCFOOTER()
{
    static OUString sProp("urn:bails:IntellectualProperty:Marking:document-footer");
    return sProp;
}

const OUString& SfxClassificationHelper::PROP_DOCWATERMARK()
{
    static OUString sProp("urn:bails:IntellectualProperty:Marking:document-watermark");
    return sProp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
