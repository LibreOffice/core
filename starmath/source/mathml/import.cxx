/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Our mathml
#include <mathml/import.hxx>

// LO tools to use
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>

// Extra LO tools
#include <comphelper/fileformat.h>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/servicehelper.hxx>
#include <rtl/character.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <sot/storage.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <unotools/streamwrap.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <o3tl/string_view.hxx>

// Our starmath tools
#include <cfgitem.hxx>
#include <document.hxx>
#include <xparsmlbase.hxx>
#include <smmod.hxx>
#include <starmathdatabase.hxx>
#include <unomodel.hxx>

// Old parser
#include <mathmlimport.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;

// SmMLImportContext
/*************************************************************************************************/

SmMlElement* SmMLImportWrapper::getElementTree()
{
    return m_pMlImport == nullptr ? nullptr : m_pMlImport->getElementTree();
}

ErrCode SmMLImportWrapper::Import(SfxMedium& rMedium)
{
    // Fetch context
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    if (!xContext.is())
    {
        SAL_WARN("starmath", "Failed to fetch model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Check model
    if (!m_xModel.is())
    {
        SAL_WARN("starmath", "Failed to fetch model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Try to get an XStatusIndicator from the Medium
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    // Get model via uno
    SmModel* pModel = m_xModel.get();
    if (pModel == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Get doc shell
    m_pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (m_pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch smdoc shell while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Check if it is an embed object
    bool bEmbedded = m_pDocShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED;

    if (!bEmbedded)
    {
        // Extra check to ensure everything is fine
        if (m_pDocShell->GetMedium() != &rMedium)
        {
            SAL_WARN("starmath", "Given medium and doc shell medium differ while file input");
            return ERRCODE_SFX_DOLOADFAILED;
        }

        // Fetch the item set
        const SfxUnoAnyItem* pItem = rMedium.GetItemSet().GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem != nullptr)
            pItem->GetValue() >>= xStatusIndicator;
    }

    // Create property list
    static const comphelper::PropertyMapEntry aInfoMap[]
        = { { u"PrivateData"_ustr, 0, cppu::UnoType<XInterface>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 } };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Set base URI
    // needed for relative URLs; but it's OK to import e.g. MathML from the clipboard without one
    SAL_INFO_IF(rMedium.GetBaseURL().isEmpty(), "starmath", "SmMLImportWrapper: no base URL");
    xInfoSet->setPropertyValue("BaseURI", Any(rMedium.GetBaseURL()));

    // Fetch progress range
    sal_Int32 nProgressRange(rMedium.IsStorage() ? 3 : 1);
    if (xStatusIndicator.is())
    {
        xStatusIndicator->start(SvxResId(RID_SVXSTR_DOC_LOAD), nProgressRange);
        xStatusIndicator->setValue(0);
    }

    // Get storage
    if (rMedium.IsStorage())
    {
        // TODO/LATER: handle the case of embedded links gracefully
        if (bEmbedded) // && !rMedium.GetStorage()->IsRoot() )
        {
            OUString aName(u"dummyObjName"_ustr);
            const SfxStringItem* pDocHierarchItem
                = rMedium.GetItemSet().GetItem(SID_DOC_HIERARCHICALNAME);
            if (pDocHierarchItem != nullptr)
                aName = pDocHierarchItem->GetValue();

            if (!aName.isEmpty())
                xInfoSet->setPropertyValue("StreamRelPath", Any(aName));
        }

        // Check if use OASIS ( new document format )
        bool bOASIS = SotStorage::GetVersion(rMedium.GetStorage()) > SOFFICE_FILEFORMAT_60;
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(1);

        // Error code in case of needed
        ErrCode nWarn = ERRCODE_NONE;

        // Read metadata
        // read a component from storage
        if (!bEmbedded)
        {
            if (bOASIS)
                nWarn = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"meta.xml", xContext,
                                              xInfoSet,
                                              u"com.sun.star.comp.Math.MLOasisMetaImporter", 6);
            else
                nWarn
                    = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"meta.xml", xContext,
                                            xInfoSet, u"com.sun.star.comp.Math.XMLMetaImporter", 5);
        }

        // Check if successful
        if (nWarn != ERRCODE_NONE)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->end();
            SAL_WARN("starmath", "Failed to read file");
            return nWarn;
        }

        // Increase success indicator
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(2);

        // Read settings
        // read a component from storage
        if (bOASIS)
            nWarn = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"settings.xml", xContext,
                                          xInfoSet,
                                          u"com.sun.star.comp.Math.MLOasisSettingsImporter", 6);
        else
            nWarn
                = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"settings.xml", xContext,
                                        xInfoSet, u"com.sun.star.comp.Math.XMLSettingsImporter", 5);

        // Check if successful
        if (nWarn != ERRCODE_NONE)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->end();
            SAL_WARN("starmath", "Failed to read file");
            return nWarn;
        }

        // Increase success indicator
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(3);

        // Read document
        // read a component from storage
        if (m_pDocShell->GetSmSyntaxVersion() == 5)
            nWarn = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"content.xml", xContext,
                                          xInfoSet, u"com.sun.star.comp.Math.XMLImporter", 5);
        else
            nWarn = ReadThroughComponentS(rMedium.GetStorage(), m_xModel, u"content.xml", xContext,
                                          xInfoSet, u"com.sun.star.comp.Math.MLImporter", 6);
        // Check if successful
        if (nWarn != ERRCODE_NONE)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->end();
            SAL_WARN("starmath", "Failed to read file");
            return nWarn;
        }

        // Finish
        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return ERRCODE_NONE;
    }
    else
    {
        // Create input stream
        Reference<io::XInputStream> xInputStream
            = new utl::OInputStreamWrapper(rMedium.GetInStream());

        // Increase success indicator
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(1);

        // Read data
        // read a component from input stream
        ErrCode nError = ERRCODE_NONE;
        if (m_pDocShell->GetSmSyntaxVersion() == 5)
            nError = ReadThroughComponentIS(xInputStream, m_xModel, xContext, xInfoSet,
                                            u"com.sun.star.comp.Math.XMLImporter", false, 5);
        else
            nError = ReadThroughComponentIS(xInputStream, m_xModel, xContext, xInfoSet,
                                            u"com.sun.star.comp.Math.MLImporter", false, 6);

        // Finish
        if (xStatusIndicator.is())
            xStatusIndicator->end();

        // Declare any error
        if (nError != ERRCODE_NONE)
            SAL_WARN("starmath", "Failed to read file");

        return nError;
    }
}

ErrCode SmMLImportWrapper::Import(std::u16string_view aSource)
{
    // Fetch context
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    if (!xContext.is())
    {
        SAL_WARN("starmath", "Failed to fetch model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Check model
    if (!m_xModel.is())
    {
        SAL_WARN("starmath", "Failed to fetch model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Make a model component from our SmModel
    uno::Reference<lang::XComponent> xModelComp = m_xModel;
    if (!xModelComp.is())
    {
        SAL_WARN("starmath", "Failed to make model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Get model via uno
    SmModel* pModel = m_xModel.get();
    if (pModel == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm model while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Get doc shell
    m_pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (m_pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch smdoc shell while file input");
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Create property list
    static const comphelper::PropertyMapEntry aInfoMap[]
        = { { u"PrivateData"_ustr, 0, cppu::UnoType<XInterface>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 } };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Read data
    // read a component from text
    ErrCode nError = ReadThroughComponentMS(aSource, xModelComp, xContext, xInfoSet);

    // Declare any error
    if (nError != ERRCODE_NONE)
    {
        SAL_WARN("starmath", "Failed to read file");
        return nError;
    }

    return ERRCODE_NONE;
}

// read a component from input stream
ErrCode SmMLImportWrapper::ReadThroughComponentIS(
    const Reference<io::XInputStream>& xInputStream, const Reference<XComponent>& xModelComponent,
    Reference<uno::XComponentContext> const& rxContext,
    Reference<beans::XPropertySet> const& rPropSet, const char16_t* pFilterName, bool bEncrypted,
    int_fast16_t nSyntaxVersion)
{
    // Needs an input stream but checked by caller
    // Needs a context but checked by caller
    // Needs property set but checked by caller
    // Needs a filter name but checked by caller

    // Prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // Prepare property list
    Sequence<Any> aArgs{ Any(rPropSet) };

    // Get filter
    Reference<XInterface> xFilter
        = rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString(pFilterName), aArgs, rxContext);
    if (!xFilter.is())
    {
        SAL_WARN("starmath", "Can't instantiate filter component " << OUString(pFilterName));
        return ERRCODE_SFX_DOLOADFAILED;
    }

    // Connect model and filter
    Reference<XImporter> xImporter(xFilter, UNO_QUERY);
    xImporter->setTargetDocument(xModelComponent);

    // Finally, parser the stream
    try
    {
        Reference<css::xml::sax::XFastParser> xFastParser(xFilter, UNO_QUERY);
        Reference<css::xml::sax::XFastDocumentHandler> xFastDocHandler(xFilter, UNO_QUERY);
        if (xFastParser)
        {
            xFastParser->setCustomEntityNames(starmathdatabase::getCustomMathmlHtmlEntities());
            xFastParser->parseStream(aParserInput);
        }
        else if (xFastDocHandler)
        {
            Reference<css::xml::sax::XFastParser> xParser
                = css::xml::sax::FastParser::create(rxContext);
            xParser->setCustomEntityNames(starmathdatabase::getCustomMathmlHtmlEntities());
            xParser->setFastDocumentHandler(xFastDocHandler);
            xParser->parseStream(aParserInput);
        }
        else
        {
            Reference<css::xml::sax::XDocumentHandler> xDocHandler(xFilter, UNO_QUERY);
            assert(xDocHandler);
            Reference<css::xml::sax::XParser> xParser = css::xml::sax::Parser::create(rxContext);
            xParser->setDocumentHandler(xDocHandler);
            xParser->parseStream(aParserInput);
        }

        if (nSyntaxVersion == 5)
        {
            SmXMLImport* pXMlImport = dynamic_cast<SmXMLImport*>(xFilter.get());
            if (pXMlImport != nullptr && pXMlImport->GetSuccess())
                return ERRCODE_NONE;
            else
            {
                SAL_WARN("starmath", "Filter failed on file input");
                // However this can not be included since it's not public
                if (pXMlImport == nullptr)
                    return ERRCODE_NONE;
                return ERRCODE_SFX_DOLOADFAILED;
            }
        }

        m_pMlImport = dynamic_cast<SmMLImport*>(xFilter.get());
        if (m_pMlImport != nullptr && m_pMlImport->getSuccess())
            return ERRCODE_NONE;
        else
        {
            SAL_WARN("starmath", "Filter failed on file input");
            return ERRCODE_SFX_DOLOADFAILED;
        }
    }
    catch (const xml::sax::SAXParseException& r)
    {
        // Sax parser sends wrapped exceptions, try to find the original one
        xml::sax::SAXException aTmp;
        xml::sax::SAXException aSaxEx = *static_cast<const xml::sax::SAXException*>(&r);
        while (aSaxEx.WrappedException >>= aTmp)
            aSaxEx = aTmp;

        packages::zip::ZipIOException aBrokenPackage;
        if (aSaxEx.WrappedException >>= aBrokenPackage)
        {
            SAL_WARN("starmath", "Failed to read file SAXParseException");
            return ERRCODE_IO_BROKENPACKAGE;
        }

        if (bEncrypted)
        {
            SAL_WARN("starmath", "Wrong file password SAXParseException");
            return ERRCODE_SFX_WRONGPASSWORD;
        }
    }
    catch (const xml::sax::SAXException& r)
    {
        packages::zip::ZipIOException aBrokenPackage;
        if (r.WrappedException >>= aBrokenPackage)
        {
            SAL_WARN("starmath", "Failed to read file SAXException");
            return ERRCODE_IO_BROKENPACKAGE;
        }

        if (bEncrypted)
        {
            SAL_WARN("starmath", "Wrong file password SAXException");
            return ERRCODE_SFX_WRONGPASSWORD;
        }
    }
    catch (const packages::zip::ZipIOException&)
    {
        SAL_WARN("starmath", "Failed to unzip file ZipIOException");
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const io::IOException&)
    {
        SAL_WARN("starmath", "Failed to read file ZipIOException");
        return ERRCODE_IO_UNKNOWN;
    }
    catch (const std::range_error&)
    {
        SAL_WARN("starmath", "Failed to read file");
        return ERRCODE_ABORT;
    }

    return ERRCODE_ABORT;
}

// read a component from storage
ErrCode SmMLImportWrapper::ReadThroughComponentS(const uno::Reference<embed::XStorage>& xStorage,
                                                 const Reference<XComponent>& xModelComponent,
                                                 const char16_t* pStreamName,
                                                 Reference<uno::XComponentContext> const& rxContext,
                                                 Reference<beans::XPropertySet> const& rPropSet,
                                                 const char16_t* pFilterName,
                                                 int_fast16_t nSyntaxVersion)
{
    // Needs a storage but checked by caller
    // Needs a model but checked by caller
    // Needs a stream name but checked by caller
    // Needs a context but checked by caller
    // Needs a property set but checked by caller
    // Needs a filter name but checked by caller

    // Get the input stream
    try
    {
        // Create the stream for the event read
        uno::Reference<io::XStream> xEventsStream
            = xStorage->openStreamElement(OUString(pStreamName), embed::ElementModes::READ);

        // Determine if stream is encrypted or not
        uno::Reference<beans::XPropertySet> xProps(xEventsStream, uno::UNO_QUERY);
        Any aAny = xProps->getPropertyValue("Encrypted");
        bool bEncrypted = false;
        aAny >>= bEncrypted;

        // Set base URL and open stream
        rPropSet->setPropertyValue("StreamName", Any(OUString(pStreamName)));
        Reference<io::XInputStream> xStream = xEventsStream->getInputStream();

        // Execute read
        return ReadThroughComponentIS(xStream, xModelComponent, rxContext, rPropSet, pFilterName,
                                      bEncrypted, nSyntaxVersion);
    }
    catch (packages::WrongPasswordException&)
    {
        SAL_WARN("starmath", "Wrong file password");
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (packages::zip::ZipIOException&)
    {
        SAL_WARN("starmath", "Failed to unzip file");
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (uno::Exception&)
    {
    }

    return ERRCODE_SFX_DOLOADFAILED;
}

// read a component from text
ErrCode SmMLImportWrapper::ReadThroughComponentMS(
    std::u16string_view aText, const css::uno::Reference<css::lang::XComponent>& xModelComponent,
    css::uno::Reference<css::uno::XComponentContext> const& rxContext,
    css::uno::Reference<css::beans::XPropertySet> const& rPropSet)
{
    // Needs a storage but checked by caller
    // Needs a model but checked by caller
    // Needs a stream name but checked by caller
    // Needs a context but checked by caller
    // Needs a property set but checked by caller
    // Needs a filter name but checked by caller

    // Get the input stream
    try
    {
        // Generate input memory stream
        SvMemoryStream aMemoryStream;
        aMemoryStream.WriteOString(OUStringToOString(aText, RTL_TEXTENCODING_UTF8));
        uno::Reference<io::XInputStream> xStream(new utl::OInputStreamWrapper(aMemoryStream));

        // Execute read
        return ReadThroughComponentIS(xStream, xModelComponent, rxContext, rPropSet,
                                      u"com.sun.star.comp.Math.MLImporter", false, 6);
    }
    catch (packages::WrongPasswordException&)
    {
        SAL_WARN("starmath", "Wrong file password");
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (packages::zip::ZipIOException&)
    {
        SAL_WARN("starmath", "Failed to unzip file");
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (uno::Exception&)
    {
    }

    return ERRCODE_SFX_DOLOADFAILED;
}

// SmMLImport technical
/*************************************************************************************************/

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_MLImporter_get_implementation(uno::XComponentContext* pCtx,
                                   uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SmMLImport(pCtx, "com.sun.star.comp.Math.XMLImporter", SvXMLImportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_MLOasisMetaImporter_get_implementation(uno::XComponentContext* pCtx,
                                            uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SmMLImport(pCtx, "com.sun.star.comp.Math.XMLOasisMetaImporter",
                                        SvXMLImportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_MLOasisSettingsImporter_get_implementation(uno::XComponentContext* pCtx,
                                                uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SmMLImport(pCtx, "com.sun.star.comp.Math.XMLOasisSettingsImporter",
                                        SvXMLImportFlags::SETTINGS));
}

// SmMLImportContext
/*************************************************************************************************/

namespace
{
class SmMLImportContext : public SvXMLImportContext
{
private:
    SmMlElement** m_pParent;
    SmMlElement* m_pElement;
    SmMlElement* m_pStyle;

public:
    SmMLImportContext(SmMLImport& rImport, SmMlElement** pParent)
        : SvXMLImportContext(rImport)
        , m_pParent(pParent)
        , m_pElement(nullptr)
        , m_pStyle(nullptr)
    {
    }

private:
    void declareMlError();

public:
    /** Handles characters (text)
     */
    virtual void SAL_CALL characters(const OUString& aChars) override;

    /** Starts the mathml element
     */
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement, const Reference<XFastAttributeList>& aAttributeList) override;

    /** Ends the mathml element
     */
    virtual void SAL_CALL endFastElement(sal_Int32 Element) override;

    /** Creates child element
     */
    virtual uno::Reference<XFastContextHandler>
        SAL_CALL createFastChildContext(sal_Int32 nElement,
                                        const uno::Reference<XFastAttributeList>& Attribs) override;

    /** Inherits the style from it's parents
    */
    void inheritStyle();

    /** Inherits the style from it's parents on end
    */
    void inheritStyleEnd();

    /** Handle mathml attributes
    */
    void handleAttributes(const Reference<XFastAttributeList>& aAttributeList);

    /** Handle mathml length attributes
    */
    SmLengthValue handleLengthAttribute(const OUString& aAttribute);
};

uno::Reference<XFastContextHandler> SAL_CALL
SmMLImportContext::createFastChildContext(sal_Int32, const uno::Reference<XFastAttributeList>&)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext;
    xContext = new SmMLImportContext(static_cast<SmMLImport&>(GetImport()), &m_pElement);
    return xContext;
}

void SmMLImportContext::declareMlError()
{
    SmMLImport& aSmMLImport = static_cast<SmMLImport&>(GetImport());
    aSmMLImport.declareMlError();
}

void SmMLImportContext::inheritStyle()
{
    while ((m_pStyle = m_pStyle->getParentElement()) != nullptr)
    {
        if (m_pStyle->getParentElement()->getMlElementType() == SmMlElementType::MlMstyle
            || m_pStyle->getParentElement()->getMlElementType() == SmMlElementType::MlMath)
            break;
    }

    // Parent inheritation
    // Mathcolor, mathsize, dir and displaystyle are inherited from parent
    SmMlElement* pParent = *m_pParent;
    m_pElement->setAttribute(pParent->getAttribute(SmMlAttributeValueType::MlMathcolor));
    m_pElement->setAttribute(pParent->getAttribute(SmMlAttributeValueType::MlMathsize));
    m_pElement->setAttribute(pParent->getAttribute(SmMlAttributeValueType::MlDir));
    m_pElement->setAttribute(pParent->getAttribute(SmMlAttributeValueType::MlDisplaystyle));

    // Inherit operator dictionary overwrites
    if (m_pStyle != nullptr
        && (m_pElement->getMlElementType() == SmMlElementType::MlMo
            || m_pElement->getMlElementType() == SmMlElementType::MlMstyle
            || m_pElement->getMlElementType() == SmMlElementType::MlMath))
    {
        // TODO fetch operator dictionary first and then overwrite
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlAccent))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlAccent));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlFence))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlFence));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlLspace))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlLspace));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMaxsize))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlMaxsize));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMinsize))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlMinsize));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMovablelimits))
            m_pElement->setAttribute(
                m_pStyle->getAttribute(SmMlAttributeValueType::MlMovablelimits));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlRspace))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlRspace));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlSeparator))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlSeparator));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlStretchy))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlStretchy));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlSymmetric))
            m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlSymmetric));

        if (m_pElement->getMlElementType() == SmMlElementType::MlMo)
        {
            // Set form based in position
            SmMlAttribute aAttribute(SmMlAttributeValueType::MlForm);
            SmMlForm aForm;
            if (m_pElement->getSubElementId() == 0)
                aForm = { SmMlAttributeValueForm::MlPrefix };
            else
                aForm = { SmMlAttributeValueForm::MlInfix };
            aAttribute.setMlForm(&aForm);
            m_pElement->setAttribute(aAttribute);
        }
    }

    // Inherit mathvariant
    if (m_pStyle && m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMathvariant))
        m_pElement->setAttribute(m_pStyle->getAttribute(SmMlAttributeValueType::MlMathvariant));
}

void SmMLImportContext::inheritStyleEnd()
{
    // Mo: check it is the end: postfix
    if (m_pElement->getMlElementType() == SmMlElementType::MlMo)
    {
        if ((*m_pParent)->getSubElementsCount() == m_pElement->getSubElementId())
        {
            // Set form based in position
            SmMlAttribute aAttribute(SmMlAttributeValueType::MlForm);
            SmMlForm aForm = { SmMlAttributeValueForm::MlPosfix };
            aAttribute.setMlForm(&aForm);
            m_pElement->setAttribute(aAttribute);
        }
    }

    // Mi: 1 char -> italic
    if (m_pElement->getMlElementType() != SmMlElementType::MlMi)
        return;

    // Inherit mathvariant
    if (!m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMathvariant))
    {
        sal_Int32 nIndexUtf16 = 0;
        // Check if there is only one code point
        m_pElement->getText().iterateCodePoints(&nIndexUtf16, 1);
        // Mathml says that 1 code point -> italic
        if (nIndexUtf16 == m_pElement->getText().getLength())
        {
            SmMlAttribute aAttribute(SmMlAttributeValueType::MlMathvariant);
            SmMlMathvariant aMathvariant = { SmMlAttributeValueMathvariant::italic };
            aAttribute.setMlMathvariant(&aMathvariant);
            aAttribute.setSet(false);
            m_pElement->setAttribute(aAttribute);
        }
    }
}

SmLengthValue SmMLImportContext::handleLengthAttribute(const OUString& aAttribute)
{
    // Locate unit indication
    int32_t nUnitPos;
    for (nUnitPos = 0;
         nUnitPos < aAttribute.getLength()
         && (rtl::isAsciiHexDigit(aAttribute[nUnitPos]) || aAttribute[nUnitPos] == '.');
         ++nUnitPos)
        ;

    // Find unit
    SmLengthUnit nUnit = SmLengthUnit::MlM;
    if (nUnitPos != aAttribute.getLength())
    {
        OUString aUnit = aAttribute.copy(nUnitPos);
        if (aUnit.compareToIgnoreAsciiCaseAscii("ex"))
            nUnit = SmLengthUnit::MlEx;
        if (aUnit.compareToIgnoreAsciiCaseAscii("px"))
            nUnit = SmLengthUnit::MlPx;
        if (aUnit.compareToIgnoreAsciiCaseAscii("in"))
            nUnit = SmLengthUnit::MlIn;
        if (aUnit.compareToIgnoreAsciiCaseAscii("cm"))
            nUnit = SmLengthUnit::MlCm;
        if (aUnit.compareToIgnoreAsciiCaseAscii("mm"))
            nUnit = SmLengthUnit::MlMm;
        if (aUnit.compareToIgnoreAsciiCaseAscii("pt"))
            nUnit = SmLengthUnit::MlPt;
        if (aUnit.compareToIgnoreAsciiCaseAscii("pc"))
            nUnit = SmLengthUnit::MlPc;
        if (aUnit.compareToIgnoreAsciiCaseAscii("%"))
            nUnit = SmLengthUnit::MlP;
        else
            declareMlError();
    }

    // Get value
    std::u16string_view aValue = aAttribute.subView(0, nUnitPos);
    double nValue = o3tl::toDouble(aValue);
    if (nValue == 0)
    {
        nUnit = SmLengthUnit::MlM;
        nValue = 1.0;
        declareMlError();
    }

    // Return
    SmLengthValue aLengthValue = { nUnit, nValue, new OUString(aAttribute) };
    return aLengthValue;
}

void SmMLImportContext::handleAttributes(const Reference<XFastAttributeList>& aAttributeList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(aAttributeList))
    {
        SmMlAttribute aAttribute(SmMlAttributeValueType::NMlEmpty);
        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_ACCENT:
            {
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlAccent);
                    SmMlAccent aAccent = { SmMlAttributeValueAccent::MlTrue };
                    aAttribute.setMlAccent(&aAccent);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlAccent);
                    SmMlAccent aAccent = { SmMlAttributeValueAccent::MlFalse };
                    aAttribute.setMlAccent(&aAccent);
                }
                else
                {
                    declareMlError();
                }
                break;
            }
            case XML_DIR:
            {
                if (IsXMLToken(aIter, XML_RTL))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDir);
                    SmMlDir aDir = { SmMlAttributeValueDir::MlRtl };
                    aAttribute.setMlDir(&aDir);
                }
                else if (IsXMLToken(aIter, XML_LTR))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDir);
                    SmMlDir aDir = { SmMlAttributeValueDir::MlLtr };
                    aAttribute.setMlDir(&aDir);
                }
                else
                {
                    declareMlError();
                }
                break;
            }
            case XML_DISPLAYSTYLE:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDisplaystyle);
                    SmMlDisplaystyle aDisplaystyle = { SmMlAttributeValueDisplaystyle::MlTrue };
                    aAttribute.setMlDisplaystyle(&aDisplaystyle);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDisplaystyle);
                    SmMlDisplaystyle aDisplaystyle = { SmMlAttributeValueDisplaystyle::MlFalse };
                    aAttribute.setMlDisplaystyle(&aDisplaystyle);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_FENCE:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlFence);
                    SmMlFence aFence = { SmMlAttributeValueFence::MlTrue };
                    aAttribute.setMlFence(&aFence);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlFence);
                    SmMlFence aFence = { SmMlAttributeValueFence::MlFalse };
                    aAttribute.setMlFence(&aFence);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_HREF:
            {
                aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlHref);
                OUString* aRef = new OUString(aIter.toString());
                SmMlHref aHref = { SmMlAttributeValueHref::NMlValid, aRef };
                aAttribute.setMlHref(&aHref);
                break;
            }
            case XML_LSPACE:
            {
                SmMlLspace aLspace;
                aLspace.m_aLengthValue = handleLengthAttribute(aIter.toString());
                aAttribute.setMlLspace(&aLspace);
                break;
            }
            case XML_MATHBACKGROUND:
            {
                if (IsXMLToken(aIter, XML_TRANSPARENT))
                {
                    SmMlMathbackground aMathbackground
                        = { SmMlAttributeValueMathbackground::MlTransparent, COL_TRANSPARENT };
                    aAttribute.setMlMathbackground(&aMathbackground);
                }
                else
                {
                    Color aColor
                        = starmathdatabase::Identify_ColorName_HTML(aIter.toString()).cColor;
                    SmMlMathbackground aMathbackground
                        = { SmMlAttributeValueMathbackground::MlRgb, aColor };
                    aAttribute.setMlMathbackground(&aMathbackground);
                }
                break;
            }
            case XML_MATHCOLOR:
            {
                if (IsXMLToken(aIter, XML_DEFAULT))
                {
                    SmMlMathcolor aMathcolor
                        = { SmMlAttributeValueMathcolor::MlDefault, COL_BLACK };
                    aAttribute.setMlMathcolor(&aMathcolor);
                }
                else
                {
                    Color aColor
                        = starmathdatabase::Identify_ColorName_HTML(aIter.toString()).cColor;
                    SmMlMathcolor aMathcolor = { SmMlAttributeValueMathcolor::MlRgb, aColor };
                    aAttribute.setMlMathcolor(&aMathcolor);
                }
                break;
            }
            case XML_MATHSIZE:
            {
                SmMlMathsize aMathsize;
                aMathsize.m_aLengthValue = handleLengthAttribute(aIter.toString());
                aAttribute.setMlMathsize(&aMathsize);
                break;
            }
            case XML_MATHVARIANT:
            {
                OUString aVariant = aIter.toString();
                SmMlAttributeValueMathvariant nVariant = SmMlAttributeValueMathvariant::normal;
                if (aVariant.compareTo(u"normal"))
                    nVariant = SmMlAttributeValueMathvariant::normal;
                else if (aVariant.compareTo(u"bold"))
                    nVariant = SmMlAttributeValueMathvariant::bold;
                else if (aVariant.compareTo(u"italic"))
                    nVariant = SmMlAttributeValueMathvariant::italic;
                else if (aVariant.compareTo(u"double-struck"))
                    nVariant = SmMlAttributeValueMathvariant::double_struck;
                else if (aVariant.compareTo(u"script"))
                    nVariant = SmMlAttributeValueMathvariant::script;
                else if (aVariant.compareTo(u"fraktur"))
                    nVariant = SmMlAttributeValueMathvariant::fraktur;
                else if (aVariant.compareTo(u"sans-serif"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif;
                else if (aVariant.compareTo(u"monospace"))
                    nVariant = SmMlAttributeValueMathvariant::monospace;
                else if (aVariant.compareTo(u"bold-italic"))
                    nVariant = SmMlAttributeValueMathvariant::bold_italic;
                else if (aVariant.compareTo(u"bold-fracktur"))
                    nVariant = SmMlAttributeValueMathvariant::bold_fraktur;
                else if (aVariant.compareTo(u"bold-script"))
                    nVariant = SmMlAttributeValueMathvariant::bold_script;
                else if (aVariant.compareTo(u"bold-sans-serif"))
                    nVariant = SmMlAttributeValueMathvariant::bold_sans_serif;
                else if (aVariant.compareTo(u"sans-serif-italic"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif_italic;
                else if (aVariant.compareTo(u"sans-serif-bold-italic"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif_bold_italic;
                else if (aVariant.compareTo(u"initial"))
                    nVariant = SmMlAttributeValueMathvariant::initial;
                else if (aVariant.compareTo(u"tailed"))
                    nVariant = SmMlAttributeValueMathvariant::tailed;
                else if (aVariant.compareTo(u"looped"))
                    nVariant = SmMlAttributeValueMathvariant::looped;
                else if (aVariant.compareTo(u"stretched"))
                    nVariant = SmMlAttributeValueMathvariant::stretched;
                else
                    declareMlError();
                SmMlMathvariant aMathvariant = { nVariant };
                aAttribute.setMlMathvariant(&aMathvariant);
                break;
            }
            case XML_MAXSIZE:
            {
                SmMlMaxsize aMaxsize;
                if (IsXMLToken(aIter, XML_INFINITY))
                {
                    aMaxsize.m_aMaxsize = SmMlAttributeValueMaxsize::MlInfinity;
                    aMaxsize.m_aLengthValue
                        = { SmLengthUnit::MlP, 10000, new OUString(u"10000%"_ustr) };
                }
                else
                {
                    aMaxsize.m_aMaxsize = SmMlAttributeValueMaxsize::MlFinite;
                    aMaxsize.m_aLengthValue = handleLengthAttribute(aIter.toString());
                }
                aAttribute.setMlMaxsize(&aMaxsize);
                break;
            }
            case XML_MINSIZE:
            {
                SmMlMinsize aMinsize;
                aMinsize.m_aLengthValue = handleLengthAttribute(aIter.toString());
                aAttribute.setMlMinsize(&aMinsize);
                break;
            }
            case XML_MOVABLELIMITS:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlMovablelimits);
                    SmMlMovablelimits aMovablelimits = { SmMlAttributeValueMovablelimits::MlTrue };
                    aAttribute.setMlMovablelimits(&aMovablelimits);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlMovablelimits);
                    SmMlMovablelimits aMovablelimits = { SmMlAttributeValueMovablelimits::MlFalse };
                    aAttribute.setMlMovablelimits(&aMovablelimits);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_RSPACE:
            {
                SmMlRspace aRspace;
                aRspace.m_aLengthValue = handleLengthAttribute(aIter.toString());
                aAttribute.setMlRspace(&aRspace);
                break;
            }
            case XML_SEPARATOR:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSeparator);
                    SmMlSeparator aSeparator = { SmMlAttributeValueSeparator::MlTrue };
                    aAttribute.setMlSeparator(&aSeparator);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSeparator);
                    SmMlSeparator aSeparator = { SmMlAttributeValueSeparator::MlFalse };
                    aAttribute.setMlSeparator(&aSeparator);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_STRETCHY:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlStretchy);
                    SmMlStretchy aStretchy = { SmMlAttributeValueStretchy::MlTrue };
                    aAttribute.setMlStretchy(&aStretchy);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlStretchy);
                    SmMlStretchy aStretchy = { SmMlAttributeValueStretchy::MlFalse };
                    aAttribute.setMlStretchy(&aStretchy);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_SYMMETRIC:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSymmetric);
                    SmMlSymmetric aSymmetric = { SmMlAttributeValueSymmetric::MlTrue };
                    aAttribute.setMlSymmetric(&aSymmetric);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSymmetric);
                    SmMlSymmetric aSymmetric = { SmMlAttributeValueSymmetric::MlFalse };
                    aAttribute.setMlSymmetric(&aSymmetric);
                }
                else
                {
                    declareMlError();
                }
                break;
            default:
                declareMlError();
                break;
        }
        if (aAttribute.isNullAttribute())
            declareMlError();
        else
            m_pElement->setAttribute(aAttribute);
    }
}

void SmMLImportContext::characters(const OUString& aChars) { m_pElement->setText(aChars); }

void SmMLImportContext::startFastElement(sal_Int32 nElement,
                                         const Reference<XFastAttributeList>& aAttributeList)
{
    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MATH):
            m_pElement = new SmMlElement(SmMlElementType::MlMath);
            break;
        case XML_ELEMENT(MATH, XML_MI):
            m_pElement = new SmMlElement(SmMlElementType::MlMi);
            break;
        case XML_ELEMENT(MATH, XML_MERROR):
            m_pElement = new SmMlElement(SmMlElementType::MlMerror);
            break;
        case XML_ELEMENT(MATH, XML_MN):
            m_pElement = new SmMlElement(SmMlElementType::MlMn);
            break;
        case XML_ELEMENT(MATH, XML_MO):
            m_pElement = new SmMlElement(SmMlElementType::MlMo);
            break;
        case XML_ELEMENT(MATH, XML_MROW):
            m_pElement = new SmMlElement(SmMlElementType::MlMrow);
            break;
        case XML_ELEMENT(MATH, XML_MTEXT):
            m_pElement = new SmMlElement(SmMlElementType::MlMtext);
            break;
        case XML_ELEMENT(MATH, XML_MSTYLE):
            m_pElement = new SmMlElement(SmMlElementType::MlMstyle);
            break;
        default:
            m_pElement = new SmMlElement(SmMlElementType::NMlEmpty);
            declareMlError();
            break;
    }
    SmMlElement* pParent = *m_pParent;
    pParent->setSubElement(pParent->getSubElementsCount(), m_pElement);
    inheritStyle();
    handleAttributes(aAttributeList);
}

void SmMLImportContext::endFastElement(sal_Int32) { inheritStyleEnd(); }
}

// SmMLImport
/*************************************************************************************************/

SvXMLImportContext*
SmMLImport::CreateFastContext(sal_Int32 nElement,
                              const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    SvXMLImportContext* pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_DOCUMENT):
        {
            if (m_pElementTree == nullptr)
                m_pElementTree = new SmMlElement(SmMlElementType::NMlEmpty);
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(),
                                                                       uno::UNO_QUERY_THROW);
            pContext = new SmMLImportContext(*this, &m_pElementTree);
            break;
        }
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_META):
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(),
                                                                       uno::UNO_QUERY_THROW);
            pContext = new SvXMLMetaDocumentContext(*this, xDPS->getDocumentProperties());
            break;
        }
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_SETTINGS):
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(),
                                                                       uno::UNO_QUERY_THROW);
            pContext = new XMLDocumentSettingsContext(*this);
            break;
        }
        default:
            declareMlError();
            break;
    }
    return pContext;
}

void SmMLImport::endDocument()
{
    uno::Reference<frame::XModel> xModel = GetModel();
    if (!xModel.is())
    {
        SAL_WARN("starmath", "Failed to set view settings because missing model");
        SvXMLImport::endDocument();
        return;
    }

    SmModel* pModel = comphelper::getFromUnoTunnel<SmModel>(xModel);
    if (!pModel)
    {
        SAL_WARN("starmath", "Failed to set view settings because missing sm model");
        SvXMLImport::endDocument();
        return;
    }

    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (!pDocShell)
    {
        SAL_WARN("starmath", "Failed to set view settings because missing sm doc shell");
        SvXMLImport::endDocument();
        return;
    }

    // Check if there is element tree
    if (m_pElementTree == nullptr)
    {
        m_bSuccess = true;
        SvXMLImport::endDocument();
        return;
    }

    // Get element tree and setup

    if (m_pElementTree->getSubElementsCount() == 0)
    {
        delete m_pElementTree;
        m_pElementTree = nullptr;
    }
    else
    {
        SmMlElement* pTmpElememt = m_pElementTree->getSubElement(0);
        delete m_pElementTree;
        m_pElementTree = pTmpElememt;
    }
    pDocShell->SetMlElementTree(m_pElementTree);

    m_bSuccess = true;
    SvXMLImport::endDocument();
}

void SmMLImport::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
{
    uno::Reference<frame::XModel> xModel = GetModel();
    if (!xModel.is())
    {
        SAL_WARN("starmath", "Failed to set view settings because missing model");
        return;
    }

    SmModel* pModel = comphelper::getFromUnoTunnel<SmModel>(xModel);
    if (!pModel)
    {
        SAL_WARN("starmath", "Failed to set view settings because missing sm model");
        return;
    }

    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (!pDocShell)
    {
        SAL_WARN("starmath", "Failed to set view settings because missing sm doc shell");
        return;
    }

    tools::Rectangle aRect(pDocShell->GetVisArea());

    tools::Long nTmp = 0;

    for (const PropertyValue& rValue : aViewProps)
    {
        if (rValue.Name == "ViewAreaTop")
        {
            rValue.Value >>= nTmp;
            aRect.SaturatingSetPosY(nTmp);
        }
        else if (rValue.Name == "ViewAreaLeft")
        {
            rValue.Value >>= nTmp;
            aRect.SaturatingSetPosX(nTmp);
        }
        else if (rValue.Name == "ViewAreaWidth")
        {
            rValue.Value >>= nTmp;
            Size aSize(aRect.GetSize());
            aSize.setWidth(nTmp);
            aRect.SaturatingSetSize(aSize);
        }
        else if (rValue.Name == "ViewAreaHeight")
        {
            rValue.Value >>= nTmp;
            Size aSize(aRect.GetSize());
            aSize.setHeight(nTmp);
            aRect.SaturatingSetSize(aSize);
        }
    }

    pDocShell->SetVisArea(aRect);
}

void SmMLImport::SetConfigurationSettings(const Sequence<PropertyValue>& aConfProps)
{
    uno::Reference<frame::XModel> xModel = GetModel();
    if (!xModel.is())
    {
        SAL_WARN("starmath", "Failed to set view settings because missing model");
        return;
    }

    uno::Reference<XPropertySet> xProps(xModel, UNO_QUERY);
    if (!xProps.is())
    {
        SAL_WARN("starmath", "Failed to set view settings because missing model properties");
        return;
    }

    Reference<XPropertySetInfo> xInfo(xProps->getPropertySetInfo());
    if (!xInfo.is())
    {
        SAL_WARN("starmath",
                 "Failed to set view settings because missing model properties information");
        return;
    }

    static constexpr OUStringLiteral sFormula(u"Formula");
    static constexpr OUStringLiteral sBasicLibraries(u"BasicLibraries");
    static constexpr OUStringLiteral sDialogLibraries(u"DialogLibraries");
    for (const PropertyValue& rValue : aConfProps)
    {
        if (rValue.Name != sFormula && rValue.Name != sBasicLibraries
            && rValue.Name != sDialogLibraries)
        {
            try
            {
                if (xInfo->hasPropertyByName(rValue.Name))
                    xProps->setPropertyValue(rValue.Name, rValue.Value);
            }
            catch (const beans::PropertyVetoException&)
            {
                // dealing with read-only properties here. Nothing to do...
            }
            catch (const Exception&)
            {
                SAL_WARN("starmath", "Unexpected issue while loading document properties");
            }
        }
    }
}

SmMLImport::SmMLImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLImportFlags nImportFlags)
    : SvXMLImport(rContext, implementationName, nImportFlags)
    , m_pElementTree(nullptr)
    , m_bSuccess(false)
    , m_nSmSyntaxVersion(SM_MOD()->GetConfig()->GetDefaultSmSyntaxVersion())
{
}

/** Handles an error on the mathml structure
 */
void SmMLImport::declareMlError()
{
    m_bSuccess = false;
    SAL_WARN("starmath", "MathML error");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
