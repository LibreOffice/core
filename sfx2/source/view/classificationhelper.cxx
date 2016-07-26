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
#include <svl/fstathelper.hxx>
#include <config_folders.h>

using namespace com::sun::star;

namespace
{

const OUString& PROP_BACNAME()
{
    static OUString sProp("BusinessAuthorizationCategory:Name");
    return sProp;
}

const OUString& PROP_STARTVALIDITY()
{
    static OUString sProp("Authorization:StartValidity");
    return sProp;
}

const OUString& PROP_NONE()
{
    static OUString sProp("None");
    return sProp;
}

const OUString& PROP_IMPACTSCALE()
{
    static OUString sProp("Impact:Scale");
    return sProp;
}

const OUString& PROP_IMPACTLEVEL()
{
    static OUString sProp("Impact:Level:Confidentiality");
    return sProp;
}

const OUString& PROP_PREFIX_EXPORTCONTROL()
{
    static OUString sProp("urn:bails:ExportControl:");
    return sProp;
}

const OUString& PROP_PREFIX_NATIONALSECURITY()
{
    static OUString sProp("urn:bails:NationalSecurity:");
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
            rCategory.m_aLabels["PolicyAuthority:Name"] = m_aPolicyAuthorityName;
            rCategory.m_aLabels["Policy:Name"] = m_aPolicyName;
            rCategory.m_aLabels["BusinessAuthorization:Identifier"] = m_aProgramID;
            rCategory.m_aLabels["BusinessAuthorizationCategory:Identifier"] = aIdentifier;

            // Also initialize defaults.
            rCategory.m_aLabels["PolicyAuthority:Identifier"] = PROP_NONE();
            rCategory.m_aLabels["PolicyAuthority:Country"] = PROP_NONE();
            rCategory.m_aLabels["Policy:Identifier"] = PROP_NONE();
            rCategory.m_aLabels["BusinessAuthorization:Name"] = PROP_NONE();
            rCategory.m_aLabels["BusinessAuthorization:Locator"] = PROP_NONE();
            rCategory.m_aLabels["BusinessAuthorizationCategory:Identifier:OID"] = PROP_NONE();
            rCategory.m_aLabels["BusinessAuthorizationCategory:Locator"] = PROP_NONE();
            rCategory.m_aLabels["BusinessAuthorization:Locator"] = PROP_NONE();
            rCategory.m_aLabels["MarkingPrecedence"] = PROP_NONE();
            rCategory.m_aLabels["Marking:general-summary"].clear();
            rCategory.m_aLabels["Marking:general-warning-statement"].clear();
            rCategory.m_aLabels["Marking:general-warning-statement:ext:2"].clear();
            rCategory.m_aLabels["Marking:general-warning-statement:ext:3"].clear();
            rCategory.m_aLabels["Marking:general-warning-statement:ext:4"].clear();
            rCategory.m_aLabels["Marking:general-distribution-statement"].clear();
            rCategory.m_aLabels["Marking:general-distribution-statement:ext:2"].clear();
            rCategory.m_aLabels["Marking:general-distribution-statement:ext:3"].clear();
            rCategory.m_aLabels["Marking:general-distribution-statement:ext:4"].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCHEADER()].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCFOOTER()].clear();
            rCategory.m_aLabels[SfxClassificationHelper::PROP_DOCWATERMARK()].clear();
            rCategory.m_aLabels["Marking:email-first-line-of-text"].clear();
            rCategory.m_aLabels["Marking:email-last-line-of-text"].clear();
            rCategory.m_aLabels["Marking:email-subject-prefix"].clear();
            rCategory.m_aLabels["Marking:email-subject-suffix"].clear();
            rCategory.m_aLabels[PROP_STARTVALIDITY()] = PROP_NONE();
            rCategory.m_aLabels["Authorization:StopValidity"] = PROP_NONE();
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
            if (rLabels.find("Impact:Level:Integrity") == rLabels.end())
                rLabels["Impact:Level:Integrity"] = m_aConfidentalityValue;
            if (rLabels.find("Impact:Level:Availability") == rLabels.end())
                rLabels["Impact:Level:Availability"] = m_aConfidentalityValue;
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
    /// Selected categories, one category for each policy type.
    std::map<SfxClassificationPolicyType, SfxClassificationCategory> m_aCategory;
    /// Possible categories of a policy to choose from.
    std::vector<SfxClassificationCategory> m_aCategories;
    const uno::Reference<document::XDocumentProperties>& m_xDocumentProperties;

    explicit Impl(const uno::Reference<document::XDocumentProperties>& xDocumentProperties);
    void parsePolicy();
    /// Synchronize m_aLabels back to the document properties.
    void pushToDocumentProperties();
    /// Set the classification start date to the system time.
    void setStartValidity(SfxClassificationPolicyType eType);
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

    // See if there is a localized variant next to the configured XML.
    OUString aExtension(".xml");
    if (aPath.endsWith(aExtension))
    {
        OUString aBase = aPath.copy(0, aPath.getLength() - aExtension.getLength());
        const LanguageTag& rLanguageTag = Application::GetSettings().GetLanguageTag();
        // Expected format is "<original path>_xx-XX.xml".
        OUString aLocalized = aBase + "_" + rLanguageTag.getBcp47() + aExtension;
        if (FStatHelper::IsDocument(aLocalized))
            aPath = aLocalized;
    }

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

void SfxClassificationHelper::Impl::setStartValidity(SfxClassificationPolicyType eType)
{
    auto itCategory = m_aCategory.find(eType);
    if (itCategory == m_aCategory.end())
        return;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(policyTypeToString(eType) + PROP_STARTVALIDITY());
    if (it != rCategory.m_aLabels.end())
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
    for (auto& rPair : m_aCategory)
    {
        SfxClassificationPolicyType eType = rPair.first;
        SfxClassificationCategory& rCategory = rPair.second;
        std::map<OUString, OUString> aLabels = rCategory.m_aLabels;
        aLabels[policyTypeToString(eType) + PROP_BACNAME()] = rCategory.m_aName;
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
            ScopedVclPtrInstance<MessageDialog>(nullptr, SfxResId(STR_TARGET_DOC_NOT_CLASSIFIED), VclMessageType::Info)->Execute();
        return false;
    }
    break;
    case SfxClassificationCheckPasteResult::DocClassificationTooLow:
    {
        if (!Application::IsHeadlessModeEnabled())
            ScopedVclPtrInstance<MessageDialog>(nullptr, SfxResId(STR_DOC_CLASSIFICATION_TOO_LOW), VclMessageType::Info)->Execute();
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
            SfxClassificationPolicyType eType = stringToPolicyType(rProperty.Name);
            OUString aPrefix = policyTypeToString(eType);
            if (!rProperty.Name.startsWith(aPrefix))
                // It's a prefix we did not recognize, ignore.
                continue;

            if (rProperty.Name == (aPrefix + PROP_BACNAME()))
                m_pImpl->m_aCategory[eType].m_aName = aValue;
            else
                m_pImpl->m_aCategory[eType].m_aLabels[rProperty.Name] = aValue;
        }
    }
}

SfxClassificationHelper::~SfxClassificationHelper()
{
}

const OUString& SfxClassificationHelper::GetBACName(SfxClassificationPolicyType eType)
{
    return m_pImpl->m_aCategory[eType].m_aName;
}

bool SfxClassificationHelper::HasImpactLevel()
{
    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return false;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTSCALE());
    if (it == rCategory.m_aLabels.end())
        return false;

    it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTLEVEL());
    if (it == rCategory.m_aLabels.end())
        return false;

    return true;
}

bool SfxClassificationHelper::HasDocumentHeader()
{
    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return false;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_DOCHEADER());
    if (it == rCategory.m_aLabels.end() || it->second.isEmpty())
        return false;

    return true;
}

bool SfxClassificationHelper::HasDocumentFooter()
{
    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return false;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_DOCFOOTER());
    if (it == rCategory.m_aLabels.end() || it->second.isEmpty())
        return false;

    return true;
}

basegfx::BColor SfxClassificationHelper::GetImpactLevelColor()
{
    basegfx::BColor aRet;

    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return aRet;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTSCALE());
    if (it == rCategory.m_aLabels.end())
        return aRet;
    OUString aScale = it->second;

    it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTLEVEL());
    if (it == rCategory.m_aLabels.end())
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
        auto itColor = aColors.find(aLevel);
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
        auto itColor = aColors.find(aLevel);
        if (itColor == aColors.end())
            return aRet;
        aRet = itColor->second;
    }

    return aRet;
}

sal_Int32 SfxClassificationHelper::GetImpactLevel()
{
    sal_Int32 nRet = -1;

    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return nRet;

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTSCALE());
    if (it == rCategory.m_aLabels.end())
        return nRet;
    OUString aScale = it->second;

    it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTLEVEL());
    if (it == rCategory.m_aLabels.end())
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
        auto itValues = aValues.find(aLevel);
        if (itValues == aValues.end())
            return nRet;
        nRet = itValues->second;
    }

    return nRet;
}

OUString SfxClassificationHelper::GetImpactScale()
{
    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return OUString();

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_IMPACTSCALE());
    if (it != rCategory.m_aLabels.end())
        return it->second;

    return OUString();
}

OUString SfxClassificationHelper::GetDocumentWatermark()
{
    auto itCategory = m_pImpl->m_aCategory.find(SfxClassificationPolicyType::IntellectualProperty);
    if (itCategory == m_pImpl->m_aCategory.end())
        return OUString();

    SfxClassificationCategory& rCategory = itCategory->second;
    auto it = rCategory.m_aLabels.find(PROP_PREFIX_INTELLECTUALPROPERTY() + PROP_DOCWATERMARK());
    if (it != rCategory.m_aLabels.end())
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

void SfxClassificationHelper::SetBACName(const OUString& rName, SfxClassificationPolicyType eType)
{
    if (m_pImpl->m_aCategories.empty())
        m_pImpl->parsePolicy();

    auto it = std::find_if(m_pImpl->m_aCategories.begin(), m_pImpl->m_aCategories.end(), [&](const SfxClassificationCategory& rCategory)
    {
        return rCategory.m_aName == rName;
    });
    if (it == m_pImpl->m_aCategories.end())
    {
        SAL_WARN("sfx.view", "'" << rName << "' is not a recognized category name");
        return;
    }

    m_pImpl->m_aCategory[eType].m_aName = it->m_aName;
    m_pImpl->m_aCategory[eType].m_aLabels.clear();
    const OUString& rPrefix = policyTypeToString(eType);
    for (const auto& rLabel : it->m_aLabels)
        m_pImpl->m_aCategory[eType].m_aLabels[rPrefix + rLabel.first] = rLabel.second;

    m_pImpl->setStartValidity(eType);
    m_pImpl->pushToDocumentProperties();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    UpdateInfobar(*pViewFrame);
}

void SfxClassificationHelper::UpdateInfobar(SfxViewFrame& rViewFrame)
{
    OUString aBACName = GetBACName(SfxClassificationPolicyType::IntellectualProperty);
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

SfxClassificationPolicyType SfxClassificationHelper::stringToPolicyType(const OUString& rType)
{
    if (rType.startsWith(PROP_PREFIX_EXPORTCONTROL()))
        return SfxClassificationPolicyType::ExportControl;
    else if (rType.startsWith(PROP_PREFIX_NATIONALSECURITY()))
        return SfxClassificationPolicyType::NationalSecurity;
    else
        return SfxClassificationPolicyType::IntellectualProperty;
}

const OUString& SfxClassificationHelper::policyTypeToString(SfxClassificationPolicyType eType)
{
    switch (eType)
    {
    case SfxClassificationPolicyType::ExportControl:
        return PROP_PREFIX_EXPORTCONTROL();
        break;
    case SfxClassificationPolicyType::NationalSecurity:
        return PROP_PREFIX_NATIONALSECURITY();
        break;
    case SfxClassificationPolicyType::IntellectualProperty:
        break;
    }

    return PROP_PREFIX_INTELLECTUALPROPERTY();
}

const OUString& SfxClassificationHelper::PROP_DOCHEADER()
{
    static OUString sProp("Marking:document-header");
    return sProp;
}

const OUString& SfxClassificationHelper::PROP_DOCFOOTER()
{
    static OUString sProp("Marking:document-footer");
    return sProp;
}

const OUString& SfxClassificationHelper::PROP_DOCWATERMARK()
{
    static OUString sProp("Marking:document-watermark");
    return sProp;
}

const OUString& SfxClassificationHelper::PROP_PREFIX_INTELLECTUALPROPERTY()
{
    static OUString sProp("urn:bails:IntellectualProperty:");
    return sProp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
