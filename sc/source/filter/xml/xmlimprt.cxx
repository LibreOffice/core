/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svl/numformat.hxx>

#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmetai.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/XMLFontStylesContext.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <svx/svdpage.hxx>

#include <svl/languageoptions.hxx>
#include <editeng/editstat.hxx>
#include <formula/errorcodes.hxx>
#include <vcl/svapp.hxx>

#include <appluno.hxx>
#include "xmlimprt.hxx"
#include "importcontext.hxx"
#include <document.hxx>
#include <docsh.hxx>
#include <docuno.hxx>
#include "xmlbodyi.hxx"
#include "xmlstyli.hxx"
#include <ViewSettingsSequenceDefines.hxx>
#include <userdat.hxx>

#include <compiler.hxx>

#include "XMLConverter.hxx"
#include "XMLDetectiveContext.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include <chgviset.hxx>
#include "XMLStylesImportHelper.hxx"
#include <sheetdata.hxx>
#include <rangeutl.hxx>
#include <formulaparserpool.hxx>
#include <externalrefmgr.hxx>
#include <editutil.hxx>
#include "editattributemap.hxx"
#include <documentimport.hxx>
#include "pivotsource.hxx"
#include <unonames.hxx>
#include <numformat.hxx>
#include <sizedev.hxx>
#include <scdll.hxx>
#include "xmlstyle.hxx"

#include <comphelper/base64.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XLabelRanges.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <cellsuno.hxx>

#include <memory>
#include <utility>

constexpr OUString SC_LOCALE = u"Locale"_ustr;
constexpr OUStringLiteral SC_CURRENCYSYMBOL = u"CurrencySymbol";
constexpr OUStringLiteral SC_REPEAT_ROW = u"repeat-row";
constexpr OUStringLiteral SC_FILTER = u"filter";
constexpr OUStringLiteral SC_PRINT_RANGE = u"print-range";
constexpr OUStringLiteral SC_HIDDEN = u"hidden";

using namespace com::sun::star;
using namespace ::xmloff::token;
using namespace ::formula;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(
        new ScXMLImport(
            context,
            u"com.sun.star.comp.Calc.XMLOasisImporter"_ustr,
            SvXMLImportFlags::ALL,
            { u"com.sun.star.comp.Calc.XMLOasisImporter"_ustr } ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisMetaImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(
        new ScXMLImport(
            context,
            u"com.sun.star.comp.Calc.XMLOasisMetaImporter"_ustr,
            SvXMLImportFlags::META,
            { u"com.sun.star.comp.Calc.XMLOasisMetaImporter"_ustr }  ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisStylesImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(
        new ScXMLImport(
            context,
            u"com.sun.star.comp.Calc.XMLOasisStylesImporter"_ustr,
            SvXMLImportFlags::STYLES|SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::MASTERSTYLES|SvXMLImportFlags::FONTDECLS,
            { u"com.sun.star.comp.Calc.XMLOasisStylesImporter"_ustr } ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisContentImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ScXMLImport(
        context,
        u"com.sun.star.comp.Calc.XMLOasisContentImporter"_ustr,
        SvXMLImportFlags::AUTOSTYLES|SvXMLImportFlags::CONTENT|SvXMLImportFlags::SCRIPTS|SvXMLImportFlags::FONTDECLS,
        uno::Sequence< OUString > { u"com.sun.star.comp.Calc.XMLOasisContentImporter"_ustr }));
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_XMLOasisSettingsImporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(
        new ScXMLImport(
            context,
            u"com.sun.star.comp.Calc.XMLOasisSettingsImporter"_ustr,
            SvXMLImportFlags::SETTINGS,
            { u"com.sun.star.comp.Calc.XMLOasisSettingsImporter"_ustr } ));
}

namespace {

// NB: virtually inherit so we can multiply inherit properly
//     in ScXMLFlatDocContext_Impl
class ScXMLDocContext_Impl : public virtual SvXMLImportContext
{
protected:
    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDocContext_Impl( ScXMLImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

}

ScXMLDocContext_Impl::ScXMLDocContext_Impl( ScXMLImport& rImport ) :
SvXMLImportContext( rImport )
{
}

namespace {

// context for flat file xml format
class ScXMLFlatDocContext_Impl
    : public ScXMLDocContext_Impl, public SvXMLMetaDocumentContext
{
public:

    ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
        const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

}

ScXMLFlatDocContext_Impl::ScXMLFlatDocContext_Impl( ScXMLImport& i_rImport,
                                                   const uno::Reference<document::XDocumentProperties>& i_xDocProps) :
SvXMLImportContext(i_rImport),
ScXMLDocContext_Impl(i_rImport),
SvXMLMetaDocumentContext(i_rImport, i_xDocProps)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLFlatDocContext_Impl::createFastChildContext( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    if ( nElement == XML_ELEMENT( OFFICE, XML_META ) )
        return SvXMLMetaDocumentContext::createFastChildContext( nElement, xAttrList );
    else
        return ScXMLDocContext_Impl::createFastChildContext( nElement, xAttrList );
}

namespace {

class ScXMLBodyContext_Impl : public ScXMLImportContext
{
public:
    ScXMLBodyContext_Impl( ScXMLImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
        createFastChildContext( sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList ) override;
};

}

ScXMLBodyContext_Impl::ScXMLBodyContext_Impl( ScXMLImport& rImport ) :
ScXMLImportContext( rImport )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLBodyContext_Impl::createFastChildContext( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );
    return GetScImport().CreateBodyContext( pAttribList );
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
    ScXMLDocContext_Impl::createFastChildContext( sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext *pContext(nullptr);

    switch( nElement )
    {
        case XML_ELEMENT( OFFICE, XML_BODY ):
        if (GetScImport().getImportFlags() & SvXMLImportFlags::CONTENT)
            pContext = new ScXMLBodyContext_Impl( GetScImport() );
        break;
        case XML_ELEMENT( OFFICE, XML_SCRIPTS ):
        if (GetScImport().getImportFlags() & SvXMLImportFlags::SCRIPTS)
            pContext = GetScImport().CreateScriptContext();
        break;
        case XML_ELEMENT( OFFICE, XML_SETTINGS ):
        if (GetScImport().getImportFlags() & SvXMLImportFlags::SETTINGS)
            pContext = new XMLDocumentSettingsContext(GetScImport());
        break;
        case XML_ELEMENT(OFFICE, XML_STYLES):
            if (GetScImport().getImportFlags() & SvXMLImportFlags::STYLES)
                pContext = GetScImport().CreateStylesContext( false);
            break;
        case XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES):
            if (GetScImport().getImportFlags() & SvXMLImportFlags::AUTOSTYLES)
                pContext = GetScImport().CreateStylesContext( true);
            break;
        case XML_ELEMENT(OFFICE, XML_FONT_FACE_DECLS):
            if (GetScImport().getImportFlags() & SvXMLImportFlags::FONTDECLS)
                pContext = GetScImport().CreateFontDeclsContext();
            break;
        case XML_ELEMENT(OFFICE, XML_MASTER_STYLES):
            if (GetScImport().getImportFlags() & SvXMLImportFlags::MASTERSTYLES)
                pContext = new ScXMLMasterStylesContext( GetImport() );
            break;
        case XML_ELEMENT(OFFICE, XML_META):
            SAL_INFO("sc", "XML_ELEMENT(OFFICE, XML_META): should not have come here, maybe document is invalid?");
            break;
    }

    return pContext;
}


void ScXMLImport::SetPostProcessData( sc::ImportPostProcessData* p )
{
    mpPostProcessData = p;
}

sc::PivotTableSources& ScXMLImport::GetPivotTableSources()
{
    if (!mpPivotSources)
        mpPivotSources.reset(new sc::PivotTableSources);

    return *mpPivotSources;
}

SvXMLImportContext *ScXMLImport::CreateFastContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_STYLES ):
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_CONTENT ):
    case XML_ELEMENT( OFFICE, XML_DOCUMENT_SETTINGS ):
        pContext = new ScXMLDocContext_Impl( *this );
        break;

    case XML_ELEMENT( OFFICE, XML_DOCUMENT_META ):
        pContext = CreateMetaContext(nElement);
        break;

    case XML_ELEMENT( OFFICE, XML_DOCUMENT ):
    {
        // flat OpenDocument file format
        pContext = new ScXMLFlatDocContext_Impl( *this, GetScModel()->getDocumentProperties());
        break;
    }

    }

    return pContext;
}

ScXMLImport::ScXMLImport(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    OUString const & implementationName, SvXMLImportFlags nImportFlag,
    const css::uno::Sequence< OUString > & sSupportedServiceNames)
:   SvXMLImport( rContext, implementationName, nImportFlag, sSupportedServiceNames ),
    mpDoc( nullptr ),
    mpPostProcessData(nullptr),
    aTables(*this),
    nSolarMutexLocked(0),
    nProgressCount(0),
    nPrevCellType(0),
    bLoadDoc( true ),
    bNullDateSetted(false),
    bSelfImportingXMLSet(false),
    mbLockSolarMutex(true),
    mbImportStyles(true),
    mbHasNewCondFormatData(false)
{
    pStylesImportHelper.reset(new ScMyStylesImportHelper(*this));

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScCellStylesProperties, xScPropHdlFactory, false);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScColumnStylesProperties, xScPropHdlFactory, false);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScRowStylesImportProperties, xScPropHdlFactory, false);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper(aXMLScTableStylesImportProperties, xScPropHdlFactory, false);

    // #i66550# needed for 'presentation:event-listener' element for URLs in shapes
    GetNamespaceMap().Add(
        GetXMLToken( XML_NP_PRESENTATION ),
        GetXMLToken( XML_N_PRESENTATION ),
        XML_NAMESPACE_PRESENTATION );
}

ScXMLImport::~ScXMLImport() noexcept
{
    pChangeTrackingImportHelper.reset();
    pNumberFormatAttributesExportHelper.reset();
    pStyleNumberFormats.reset();
    pStylesImportHelper.reset();

    m_aMyNamedExpressions.clear();
    maMyLabelRanges.clear();
    maValidations.clear();
    pDetectiveOpArray.reset();

    //call SvXMLImport dtor contents before deleting pSolarMutexGuard
    cleanup();

    moSolarMutexGuard.reset();
}

void ScXMLImport::initialize( const css::uno::Sequence<css::uno::Any>& aArguments )
{
    SvXMLImport::initialize(aArguments);

    uno::Reference<beans::XPropertySet> xInfoSet = getImportInfo();
    if (!xInfoSet.is())
        return;

    uno::Reference<beans::XPropertySetInfo> xInfoSetInfo = xInfoSet->getPropertySetInfo();
    if (!xInfoSetInfo.is())
        return;

    if (xInfoSetInfo->hasPropertyByName(SC_UNO_ODS_LOCK_SOLAR_MUTEX))
        xInfoSet->getPropertyValue(SC_UNO_ODS_LOCK_SOLAR_MUTEX) >>= mbLockSolarMutex;

    if (xInfoSetInfo->hasPropertyByName(SC_UNO_ODS_IMPORT_STYLES))
        xInfoSet->getPropertyValue(SC_UNO_ODS_IMPORT_STYLES) >>= mbImportStyles;
}

SvXMLImportContext *ScXMLImport::CreateFontDeclsContext()
{
    XMLFontStylesContext *pFSContext = new XMLFontStylesContext(
        *this, osl_getThreadTextEncoding());
    SetFontDecls(pFSContext);
    SvXMLImportContext* pContext = pFSContext;
    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateStylesContext( bool bIsAutoStyle )
{
    SvXMLImportContext* pContext = new XMLTableStylesContext(
        *this, bIsAutoStyle);

    if (bIsAutoStyle)
        SetAutoStyles(static_cast<SvXMLStylesContext*>(pContext));
    else
        SetStyles(static_cast<SvXMLStylesContext*>(pContext));

    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateBodyContext(const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
{
    return new ScXMLBodyContext(*this, rAttrList);
}

SvXMLImportContext *ScXMLImport::CreateMetaContext(
    const sal_Int32 /*nElement*/ )
{
    SvXMLImportContext* pContext = nullptr;

    if (getImportFlags() & SvXMLImportFlags::META)
    {
        uno::Reference<document::XDocumentProperties> const xDocProps(
            (IsStylesOnlyMode()) ? nullptr : GetScModel()->getDocumentProperties());
        pContext = new SvXMLMetaDocumentContext(*this, xDocProps);
    }

    return pContext;
}

SvXMLImportContext *ScXMLImport::CreateScriptContext()
{
    SvXMLImportContext* pContext = nullptr;

    if( !(IsStylesOnlyMode()) )
    {
        pContext = new XMLScriptContext( *this, GetModel() );
    }

    return pContext;
}

void ScXMLImport::SetStatistics(const uno::Sequence<beans::NamedValue> & i_rStats)
{
    static const char* const s_stats[] =
    { "TableCount", "CellCount", "ObjectCount", nullptr };

    SvXMLImport::SetStatistics(i_rStats);

    sal_uInt64 nCount(0);
    for (const auto& rStat : i_rStats) {
        for (const char*const* pStat = s_stats; *pStat != nullptr; ++pStat) {
            if (rStat.Name.equalsAscii(*pStat)) {
                sal_Int32 val = 0;
                if (rStat.Value >>= val) {
                    nCount += val;
                } else {
                    OSL_FAIL("ScXMLImport::SetStatistics: invalid entry");
                }
            }
        }
    }

    if (nCount)
    {
        GetProgressBarHelper()->SetReference(nCount);
        GetProgressBarHelper()->SetValue(0);
    }
}

ScDocumentImport& ScXMLImport::GetDoc()
{
    return *mpDocImport;
}

sal_Int16 ScXMLImport::GetCellType(const char* rStrValue, const sal_Int32 nStrLength)
{
    sal_Int16 nCellType = util::NumberFormat::UNDEFINED;
    if (rStrValue != nullptr)
    {
        switch (rStrValue[0])
        {
            case 'b':
                if (nStrLength == 7 && !strcmp(rStrValue, "boolean"))
                    nCellType = util::NumberFormat::LOGICAL;
            break;
            case 'c':
                if (nStrLength == 8 && !strcmp(rStrValue, "currency"))
                    nCellType = util::NumberFormat::CURRENCY;
            break;
            case 'd':
                if (nStrLength == 4 && !strcmp(rStrValue, "date"))
                    nCellType = util::NumberFormat::DATETIME;
            break;
            case 'f':
                if (nStrLength == 5 && !strcmp(rStrValue, "float"))
                    nCellType = util::NumberFormat::NUMBER;
            break;
            case 'p':
                if (nStrLength == 10 && !strcmp(rStrValue, "percentage"))
                    nCellType = util::NumberFormat::PERCENT;
            break;
            case 's':
                if (nStrLength == 6 && !strcmp(rStrValue, "string"))
                    nCellType = util::NumberFormat::TEXT;
            break;
            case 't':
                if (nStrLength == 4 && !strcmp(rStrValue, "time"))
                    nCellType = util::NumberFormat::TIME;
            break;
        }
    }

    return nCellType;
}

XMLShapeImportHelper* ScXMLImport::CreateShapeImport()
{
    return new XMLTableShapeImportHelper(*this);
}

bool ScXMLImport::GetValidation(const OUString& sName, ScMyImportValidation& aValidation)
{
    auto aItr = std::find_if(maValidations.begin(), maValidations.end(),
        [&sName](const ScMyImportValidation& rValidation) { return rValidation.sName == sName; });
    if (aItr != maValidations.end())
    {
        // source position must be set as string,
        // so sBaseCellAddress no longer has to be converted here
        aValidation = *aItr;
        return true;
    }
    return false;
}

void ScXMLImport::AddNamedExpression(SCTAB nTab, ScMyNamedExpression aNamedExp)
{
    SheetNamedExpMap::iterator itr = m_SheetNamedExpressions.find(nTab);
    if (itr == m_SheetNamedExpressions.end())
    {
        // No chain exists for this sheet.  Create one.
        ::std::pair<SheetNamedExpMap::iterator, bool> r =
            m_SheetNamedExpressions.insert(std::make_pair(nTab, ScMyNamedExpressions()));
        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }
    ScMyNamedExpressions& r = itr->second;
    r.push_back(std::move(aNamedExp));
}

ScXMLChangeTrackingImportHelper* ScXMLImport::GetChangeTrackingImportHelper()
{
    if (!pChangeTrackingImportHelper)
        pChangeTrackingImportHelper.reset(new ScXMLChangeTrackingImportHelper());
    return pChangeTrackingImportHelper.get();
}

void ScXMLImport::InsertStyles()
{
    GetStyles()->CopyStylesToDoc(true);

    // if content is going to be loaded with the same import, set bLatinDefaultStyle flag now
    if ( getImportFlags() & SvXMLImportFlags::CONTENT )
        ExamineDefaultStyle();
}

void ScXMLImport::ExamineDefaultStyle()
{
    if (mpDoc)
    {
        // #i62435# after inserting the styles, check if the default style has a latin-script-only
        // number format (then, value cells can be pre-initialized with western script type)

        const ScPatternAttr& rDefPattern(mpDoc->getCellAttributeHelper().getDefaultCellAttribute());
        if (sc::NumFmtUtil::isLatinScript(rDefPattern, *mpDoc))
            mpDocImport->setDefaultNumericScript(SvtScriptType::LATIN);
    }
}

void ScXMLImport::SetChangeTrackingViewSettings(const css::uno::Sequence<css::beans::PropertyValue>& rChangeProps)
{
    if (!mpDoc)
        return;

    if (!rChangeProps.hasElements())
        return;

    ScXMLImport::MutexGuard aGuard(*this);
    sal_Int16 nTemp16(0);
    ScChangeViewSettings aViewSettings;
    for (const auto& rChangeProp : rChangeProps)
    {
        OUString sName(rChangeProp.Name);
        if (sName == "ShowChanges")
            aViewSettings.SetShowChanges(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowAcceptedChanges")
            aViewSettings.SetShowAccepted(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowRejectedChanges")
            aViewSettings.SetShowRejected(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowChangesByDatetime")
            aViewSettings.SetHasDate(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowChangesByDatetimeMode")
        {
            if (rChangeProp.Value >>= nTemp16)
                aViewSettings.SetTheDateMode(static_cast<SvxRedlinDateMode>(nTemp16));
        }
        else if (sName == "ShowChangesByDatetimeFirstDatetime")
        {
            util::DateTime aDateTime;
            if (rChangeProp.Value >>= aDateTime)
            {
                aViewSettings.SetTheFirstDateTime(::DateTime(aDateTime));
            }
        }
        else if (sName == "ShowChangesByDatetimeSecondDatetime")
        {
            util::DateTime aDateTime;
            if (rChangeProp.Value >>= aDateTime)
            {
                aViewSettings.SetTheLastDateTime(::DateTime(aDateTime));
            }
        }
        else if (sName == "ShowChangesByAuthor")
            aViewSettings.SetHasAuthor(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowChangesByAuthorName")
        {
            OUString sOUName;
            if (rChangeProp.Value >>= sOUName)
            {
                aViewSettings.SetTheAuthorToShow(sOUName);
            }
        }
        else if (sName == "ShowChangesByComment")
            aViewSettings.SetHasComment(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowChangesByCommentText")
        {
            OUString sOUComment;
            if (rChangeProp.Value >>= sOUComment)
            {
                aViewSettings.SetTheComment(sOUComment);
            }
        }
        else if (sName == "ShowChangesByRanges")
            aViewSettings.SetHasRange(::cppu::any2bool(rChangeProp.Value));
        else if (sName == "ShowChangesByRangesList")
        {
            OUString sRanges;
            if ((rChangeProp.Value >>= sRanges) && !sRanges.isEmpty())
            {
                ScRangeList aRangeList;
                ScRangeStringConverter::GetRangeListFromString(
                    aRangeList, sRanges, *mpDoc, FormulaGrammar::CONV_OOO);
                aViewSettings.SetTheRangeList(aRangeList);
            }
        }
    }
    mpDoc->SetChangeViewSettings(aViewSettings);
}

void ScXMLImport::SetViewSettings(const uno::Sequence<beans::PropertyValue>& aViewProps)
{
    sal_Int32 nHeight(0);
    sal_Int32 nLeft(0);
    sal_Int32 nTop(0);
    sal_Int32 nWidth(0);
    for (const auto& rViewProp : aViewProps)
    {
        OUString sName(rViewProp.Name);
        if (sName == "VisibleAreaHeight")
            rViewProp.Value >>= nHeight;
        else if (sName == "VisibleAreaLeft")
            rViewProp.Value >>= nLeft;
        else if (sName == "VisibleAreaTop")
            rViewProp.Value >>= nTop;
        else if (sName == "VisibleAreaWidth")
            rViewProp.Value >>= nWidth;
        else if (sName == "TrackedChangesViewSettings")
        {
            uno::Sequence<beans::PropertyValue> aChangeProps;
            if(rViewProp.Value >>= aChangeProps)
                SetChangeTrackingViewSettings(aChangeProps);
        }
    }
    if (!(nHeight && nWidth && GetModel().is()))
        return;

    ScModelObj* pDocObj( GetScModel() );
    if (!pDocObj)
        return;

    SfxObjectShell* pEmbeddedObj = pDocObj->GetEmbeddedObject();
    if (pEmbeddedObj)
    {
        tools::Rectangle aRect{ nLeft, nTop };
        aRect.setWidth( nWidth );
        aRect.setHeight( nHeight );
        pEmbeddedObj->SetVisArea(aRect);
    }
}

void ScXMLImport::SetConfigurationSettings(const uno::Sequence<beans::PropertyValue>& aConfigProps)
{
    rtl::Reference<ScModelObj> xMultiServiceFactory(GetScModel());
    if (!xMultiServiceFactory.is())
        return;

    sal_Int32 nCount(aConfigProps.getLength());
    css::uno::Sequence<css::beans::PropertyValue> aFilteredProps(nCount);
    auto pFilteredProps = aFilteredProps.getArray();
    sal_Int32 nFilteredPropsLen = 0;
    for (sal_Int32 i = nCount - 1; i >= 0; --i)
    {
        if (aConfigProps[i].Name == "TrackedChangesProtectionKey")
        {
            OUString sKey;
            if (aConfigProps[i].Value >>= sKey)
            {
                uno::Sequence<sal_Int8> aPass;
                ::comphelper::Base64::decode(aPass, sKey);
                if (mpDoc && aPass.hasElements())
                {
                    if (mpDoc->GetChangeTrack())
                        mpDoc->GetChangeTrack()->SetProtection(aPass);
                    else
                    {
                        std::set<OUString> aUsers;
                        std::unique_ptr<ScChangeTrack> pTrack( new ScChangeTrack(*mpDoc, std::move(aUsers)) );
                        pTrack->SetProtection(aPass);
                        mpDoc->SetChangeTrack(std::move(pTrack));
                    }
                }
            }
        }
        // store the following items for later use (after document is loaded)
        else if ((aConfigProps[i].Name == "VBACompatibilityMode") || (aConfigProps[i].Name == "ScriptConfiguration"))
        {
            uno::Reference< beans::XPropertySet > xImportInfo = getImportInfo();
            if (xImportInfo.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropertySetInfo = xImportInfo->getPropertySetInfo();
                if (xPropertySetInfo.is() && xPropertySetInfo->hasPropertyByName(aConfigProps[i].Name))
                    xImportInfo->setPropertyValue( aConfigProps[i].Name, aConfigProps[i].Value );
            }
        }
        if (aConfigProps[i].Name != "LinkUpdateMode")
        {
            pFilteredProps[nFilteredPropsLen++] = aConfigProps[i];
        }
    }
    aFilteredProps.realloc(nFilteredPropsLen);
    uno::Reference <uno::XInterface> xInterface = xMultiServiceFactory->createInstance(u"com.sun.star.comp.SpreadsheetSettings"_ustr);
    uno::Reference <beans::XPropertySet> xProperties(xInterface, uno::UNO_QUERY);
    if (xProperties.is())
        SvXMLUnitConverter::convertPropertySet(xProperties, aFilteredProps);
}

sal_Int32 ScXMLImport::SetCurrencySymbol(const sal_Int32 nKey, std::u16string_view rCurrency)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            OUString sFormatString;
            try
            {
                uno::Reference <beans::XPropertySet> xProperties(xLocalNumberFormats->getByKey(nKey));
                if (xProperties.is())
                {
                    lang::Locale aLocale;
                    if (xProperties->getPropertyValue(SC_LOCALE) >>= aLocale)
                    {
                        {
                            ScXMLImport::MutexGuard aGuard(*this);
                            const LocaleDataWrapper* pLocaleData = LocaleDataWrapper::get( LanguageTag( aLocale) );
                            sFormatString = "#" +
                                    pLocaleData->getNumThousandSep() +
                                    "##0" +
                                    pLocaleData->getNumDecimalSep() +
                                    "00 [$" +
                                    rCurrency +
                                    "]";
                        }
                        sal_Int32 nNewKey = xLocalNumberFormats->queryKey(sFormatString, aLocale, true);
                        if (nNewKey == -1)
                            nNewKey = xLocalNumberFormats->addNew(sFormatString, aLocale);
                        return nNewKey;
                    }
                }
            }
            catch ( const util::MalformedNumberFormatException& rException )
            {
                OUString sErrorMessage ="Error in Formatstring " +
                    sFormatString + " at position " +
                    OUString::number(rException.CheckPos);
                uno::Sequence<OUString> aSeq { sErrorMessage };
                uno::Reference<xml::sax::XLocator> xLocator;
                SetError(XMLERROR_API | XMLERROR_FLAG_ERROR, aSeq, rException.Message, xLocator);
            }
        }
    }
    return nKey;
}

bool ScXMLImport::IsCurrencySymbol(const sal_Int32 nNumberFormat, std::u16string_view sCurrentCurrency, std::u16string_view sBankSymbol)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xLocalNumberFormats(xNumberFormatsSupplier->getNumberFormats());
        if (xLocalNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet(xLocalNumberFormats->getByKey(nNumberFormat));
                if (xNumberPropertySet.is())
                {
                    OUString sTemp;
                    if ( xNumberPropertySet->getPropertyValue(SC_CURRENCYSYMBOL) >>= sTemp)
                    {
                        if (sCurrentCurrency == sTemp)
                            return true;
                        // A release that saved an unknown currency may have
                        // saved the currency symbol of the number format
                        // instead of an ISO code bank symbol. In another
                        // release we may have a match for that. In this case
                        // sCurrentCurrency is the ISO code obtained through
                        // XMLNumberFormatAttributesExportHelper::GetCellType()
                        // and sBankSymbol is the currency symbol.
                        if (sCurrentCurrency.size() == 3 && sBankSymbol == sTemp)
                            return true;
                        // #i61657# This may be a legacy currency symbol that changed in the meantime.
                        if (SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sCurrentCurrency, sBankSymbol) != nullptr)
                            return true;
                        // In the rare case that sCurrentCurrency is not the
                        // currency symbol, but a matching ISO code
                        // abbreviation instead that was obtained through
                        // XMLNumberFormatAttributesExportHelper::GetCellType(),
                        // check with the number format's symbol. This happens,
                        // for example, in the es_BO locale, where a legacy
                        // B$,BOB matched B$->BOP, which leads to
                        // sCurrentCurrency being BOP, and the previous call
                        // with BOP,BOB didn't find an entry, but B$,BOB will.
                        return SvNumberFormatter::GetLegacyOnlyCurrencyEntry( sTemp, sBankSymbol) != nullptr;
                    }
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    return false;
}

void ScXMLImport::SetType(const uno::Reference <beans::XPropertySet>& rProperties,
                          sal_Int32& rNumberFormat,
                          const sal_Int16 nCellType,
                          std::u16string_view rCurrency)
{
    if (!mbImportStyles)
        return;

    if ((nCellType == util::NumberFormat::TEXT) || (nCellType == util::NumberFormat::UNDEFINED))
        return;

    if (rNumberFormat == -1)
        rProperties->getPropertyValue( SC_UNONAME_NUMFMT ) >>= rNumberFormat;
    OSL_ENSURE(rNumberFormat != -1, "no NumberFormat");
    bool bIsStandard;
    // sCurrentCurrency may be the ISO code abbreviation if the currency
    // symbol matches such, or if no match found the symbol itself!
    OUString sCurrentCurrency;
    sal_Int32 nCurrentCellType(
        GetNumberFormatAttributesExportHelper()->GetCellType(
            rNumberFormat, sCurrentCurrency, bIsStandard) & ~util::NumberFormat::DEFINED);
    // If the (numeric) cell type (number, currency, date, time, boolean)
    // is different from the format type then for some combinations we may
    // have to apply a format, e.g. in case the generator deduced format
    // from type and did not apply a format but we don't keep a dedicated
    // type internally. Specifically this is necessary if the cell type is
    // not number but the format type is (i.e. General). Currency cells
    // need extra attention, see calls of ScXMLImport::IsCurrencySymbol()
    // and description within there and ScXMLImport::SetCurrencySymbol().
    if ((nCellType != nCurrentCellType) &&
            (nCellType != util::NumberFormat::NUMBER) &&
            (bIsStandard || (nCellType == util::NumberFormat::CURRENCY)))
    {
        if (!xNumberFormats.is())
        {
            uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier(GetNumberFormatsSupplier());
            if (xNumberFormatsSupplier.is())
                xNumberFormats.set(xNumberFormatsSupplier->getNumberFormats());
        }
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference < beans::XPropertySet> xNumberFormatProperties(xNumberFormats->getByKey(rNumberFormat));
                if (xNumberFormatProperties.is())
                {
                    if (nCellType != util::NumberFormat::CURRENCY)
                    {
                        lang::Locale aLocale;
                        if ( xNumberFormatProperties->getPropertyValue(SC_LOCALE) >>= aLocale )
                        {
                            if (!xNumberFormatTypes.is())
                                xNumberFormatTypes.set(uno::Reference <util::XNumberFormatTypes>(xNumberFormats, uno::UNO_QUERY));
                            rProperties->setPropertyValue( SC_UNONAME_NUMFMT, uno::Any(xNumberFormatTypes->getStandardFormat(nCellType, aLocale)) );
                        }
                    }
                    else if (!rCurrency.empty() && !sCurrentCurrency.isEmpty())
                    {
                        if (sCurrentCurrency != rCurrency)
                            if (!IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
                                rProperties->setPropertyValue( SC_UNONAME_NUMFMT, uno::Any(SetCurrencySymbol(rNumberFormat, rCurrency)));
                    }
                }
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL("Numberformat not found");
            }
        }
    }
    else
    {
        if ((nCellType == util::NumberFormat::CURRENCY) && !rCurrency.empty() && !sCurrentCurrency.isEmpty() &&
            sCurrentCurrency != rCurrency && !IsCurrencySymbol(rNumberFormat, sCurrentCurrency, rCurrency))
            rProperties->setPropertyValue( SC_UNONAME_NUMFMT, uno::Any(SetCurrencySymbol(rNumberFormat, rCurrency)));
    }
}

void ScXMLImport::SetStyleToRanges()
{
    if (!mbImportStyles)
        return;

    if (!sPrevStyleName.isEmpty())
    {
        if (mxSheetCellRanges.is())
        {
            XMLTableStylesContext *pStyles(static_cast<XMLTableStylesContext *>(GetAutoStyles()));
            XMLTableStyleContext* pStyle = nullptr;
            if ( pStyles )
                pStyle = const_cast<XMLTableStyleContext*>(static_cast<const XMLTableStyleContext *>(pStyles->FindStyleChildContext(
                        XmlStyleFamily::TABLE_CELL, sPrevStyleName, true)));
            if (pStyle)
            {
                pStyle->FillPropertySet(mxSheetCellRanges);
                // here needs to be the cond format import method
                sal_Int32 nNumberFormat(pStyle->GetNumberFormat());
                SetType(mxSheetCellRanges, nNumberFormat, nPrevCellType, sPrevCurrency);

                css::uno::Any aAny = mxSheetCellRanges->getPropertyValue(u"FormatID"_ustr);
                sal_uInt64 nKey = 0;
                if ((aAny >>= nKey) && nKey)
                {
                    ScFormatSaveData* pFormatSaveData = GetScModel()->GetFormatSaveData();
                    pFormatSaveData->maIDToName.insert(std::pair<sal_uInt64, OUString>(nKey, sPrevStyleName));
                }

                // store first cell of first range for each style, once per sheet
                uno::Sequence<table::CellRangeAddress> aAddresses(mxSheetCellRanges->getRangeAddresses());
                pStyle->ApplyCondFormat(aAddresses);
                if ( aAddresses.hasElements() )
                {
                    const table::CellRangeAddress& rRange = aAddresses[0];
                    if ( rRange.Sheet != pStyle->GetLastSheet() )
                    {
                        ScSheetSaveData* pSheetData = GetScModel()->GetSheetSaveData();
                        pSheetData->AddCellStyle( sPrevStyleName,
                            ScAddress( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), static_cast<SCTAB>(rRange.Sheet) ) );
                        pStyle->SetLastSheet(rRange.Sheet);
                    }
                }
            }
            else
            {
                mxSheetCellRanges->setPropertyValue(SC_UNONAME_CELLSTYL, uno::Any(GetStyleDisplayName( XmlStyleFamily::TABLE_CELL, sPrevStyleName )));
                sal_Int32 nNumberFormat(GetStyleNumberFormats()->GetStyleNumberFormat(sPrevStyleName));
                bool bInsert(nNumberFormat == -1);
                SetType(mxSheetCellRanges, nNumberFormat, nPrevCellType, sPrevCurrency);
                if (bInsert)
                    GetStyleNumberFormats()->AddStyleNumberFormat(sPrevStyleName, nNumberFormat);
            }
        }
    }
    if (GetModel().is())
    {
        rtl::Reference<ScModelObj> xMultiServiceFactory(GetScModel());
        mxSheetCellRanges = &dynamic_cast<ScCellRangesObj&>(
            *xMultiServiceFactory->createInstance(u"com.sun.star.sheet.SheetCellRanges"_ustr));
    }
    OSL_ENSURE(mxSheetCellRanges.is(), "didn't get SheetCellRanges");
}

void ScXMLImport::SetStyleToRanges(const ScRangeList& rRanges, const OUString* pStyleName,
                                  const sal_Int16 nCellType, const OUString* pCurrency)
{
    if (!mbImportStyles)
        return;

    if (sPrevStyleName.isEmpty())
    {
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if (!sPrevCurrency.isEmpty())
            sPrevCurrency.clear();
    }
    else if ((nCellType != nPrevCellType) ||
        ((pStyleName && *pStyleName != sPrevStyleName) ||
        (!pStyleName && !sPrevStyleName.isEmpty())) ||
        ((pCurrency && *pCurrency != sPrevCurrency) ||
        (!pCurrency && !sPrevCurrency.isEmpty())))
    {
        SetStyleToRanges();
        nPrevCellType = nCellType;
        if (pStyleName)
            sPrevStyleName = *pStyleName;
        else if(!sPrevStyleName.isEmpty())
            sPrevStyleName.clear();
        if (pCurrency)
            sPrevCurrency = *pCurrency;
        else if(!sPrevCurrency.isEmpty())
            sPrevCurrency.clear();
    }

    if (!mxSheetCellRanges.is() && GetModel().is())
    {
        rtl::Reference<ScModelObj> xMultiServiceFactory(GetScModel());
        mxSheetCellRanges = &dynamic_cast<ScCellRangesObj&>(*xMultiServiceFactory->createInstance(u"com.sun.star.sheet.SheetCellRanges"_ustr));
        OSL_ENSURE(mxSheetCellRanges.is(), "didn't get SheetCellRanges");
    }
    mxSheetCellRanges->SetNewRanges(rRanges);
}

bool ScXMLImport::SetNullDateOnUnitConverter()
{
    if (!bNullDateSetted)
        bNullDateSetted = GetMM100UnitConverter().setNullDate(GetModel());
    OSL_ENSURE(bNullDateSetted, "could not set the null date");
    return bNullDateSetted;
}

XMLNumberFormatAttributesExportHelper* ScXMLImport::GetNumberFormatAttributesExportHelper()
{
    if (!pNumberFormatAttributesExportHelper)
        pNumberFormatAttributesExportHelper.reset(new XMLNumberFormatAttributesExportHelper(GetNumberFormatsSupplier()));
    return pNumberFormatAttributesExportHelper.get();
}

ScMyStyleNumberFormats* ScXMLImport::GetStyleNumberFormats()
{
    if (!pStyleNumberFormats)
        pStyleNumberFormats.reset(new ScMyStyleNumberFormats);
    return pStyleNumberFormats.get();
}

void ScXMLImport::SetStylesToRangesFinished()
{
    SetStyleToRanges();
    sPrevStyleName.clear();
}

// XImporter
void SAL_CALL ScXMLImport::setTargetDocument( const css::uno::Reference< css::lang::XComponent >& xDoc )
{
    ScXMLImport::MutexGuard aGuard(*this);
    SvXMLImport::setTargetDocument( xDoc );

    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
    mpDoc = ScXMLConverter::GetScDocument( xModel );
    OSL_ENSURE( mpDoc, "ScXMLImport::setTargetDocument - no ScDocument!" );
    if (!mpDoc)
        throw lang::IllegalArgumentException();

    if (ScDocShell* mpDocSh = mpDoc->GetDocumentShell())
        mpDocSh->SetInitialLinkUpdate( mpDocSh->GetMedium());

    mpDocImport.reset(new ScDocumentImport(*mpDoc));
    mpComp.reset(new ScCompiler(*mpDoc, ScAddress(), formula::FormulaGrammar::GRAM_ODFF));

    uno::Reference<document::XActionLockable> xActionLockable(xDoc, uno::UNO_QUERY);
    if (xActionLockable.is())
        xActionLockable->addActionLock();
}

// css::xml::sax::XDocumentHandler
void SAL_CALL ScXMLImport::startDocument()
{
    ScXMLImport::MutexGuard aGuard(*this);
    SvXMLImport::startDocument();
    if (mpDoc && !mpDoc->IsImportingXML())
    {
        GetScModel()->BeforeXMLLoading();
        bSelfImportingXMLSet = true;
    }

    // if content and styles are loaded with separate imports,
    // set bLatinDefaultStyle flag at the start of the content import
    SvXMLImportFlags nFlags = getImportFlags();
    if ( ( nFlags & SvXMLImportFlags::CONTENT ) && !( nFlags & SvXMLImportFlags::STYLES ) )
        ExamineDefaultStyle();

    if (getImportFlags() & SvXMLImportFlags::CONTENT)
    {
        if (GetModel().is())
        {
            // store initial namespaces, to find the ones that were added from the file later
            ScSheetSaveData* pSheetData = GetScModel()->GetSheetSaveData();
            const SvXMLNamespaceMap& rNamespaces = GetNamespaceMap();
            pSheetData->StoreInitialNamespaces(rNamespaces);
        }
    }

    uno::Reference< beans::XPropertySet > const xImportInfo( getImportInfo() );
    uno::Reference< beans::XPropertySetInfo > const xPropertySetInfo(
            xImportInfo.is() ? xImportInfo->getPropertySetInfo() : nullptr);
    if (xPropertySetInfo.is())
    {
        static constexpr OUString sOrganizerMode(u"OrganizerMode"_ustr);
        if (xPropertySetInfo->hasPropertyByName(sOrganizerMode))
        {
            bool bStyleOnly(false);
            if (xImportInfo->getPropertyValue(sOrganizerMode) >>= bStyleOnly)
            {
                bLoadDoc = !bStyleOnly;
            }
        }
    }

    UnlockSolarMutex();
}

sal_Int32 ScXMLImport::GetRangeType(std::u16string_view sRangeType)
{
    sal_Int32 nRangeType(0);
    OUStringBuffer sBuffer;
    size_t i = 0;
    while (i <= sRangeType.size())
    {
        if ((i == sRangeType.size()) || (sRangeType[i] == ' '))
        {
            OUString sTemp = sBuffer.makeStringAndClear();
            if (sTemp == "repeat-column")
                nRangeType |= sheet::NamedRangeFlag::COLUMN_HEADER;
            else if (sTemp == SC_REPEAT_ROW)
                nRangeType |= sheet::NamedRangeFlag::ROW_HEADER;
            else if (sTemp == SC_FILTER)
                nRangeType |= sheet::NamedRangeFlag::FILTER_CRITERIA;
            else if (sTemp == SC_PRINT_RANGE)
                nRangeType |= sheet::NamedRangeFlag::PRINT_AREA;
            else if (sTemp == SC_HIDDEN)
                nRangeType |= sheet::NamedRangeFlag::HIDDEN;
        }
        else if (i < sRangeType.size())
            sBuffer.append(sRangeType[i]);
        ++i;
    }
    return nRangeType;
}

void ScXMLImport::SetLabelRanges()
{
    if (maMyLabelRanges.empty())
        return;

    if (!mpDoc)
        return;

    rtl::Reference<ScModelObj> xPropertySet (GetScModel());
    if (!xPropertySet.is())
        return;

    uno::Any aColAny = xPropertySet->getPropertyValue(SC_UNO_COLLABELRNG);
    uno::Any aRowAny = xPropertySet->getPropertyValue(SC_UNO_ROWLABELRNG);

    uno::Reference< sheet::XLabelRanges > xColRanges;
    uno::Reference< sheet::XLabelRanges > xRowRanges;

    if ( !(( aColAny >>= xColRanges ) && ( aRowAny >>= xRowRanges )) )
        return;

    table::CellRangeAddress aLabelRange;
    table::CellRangeAddress aDataRange;

    for (const auto& rLabelRange : maMyLabelRanges)
    {
        sal_Int32 nOffset1(0);
        sal_Int32 nOffset2(0);
        FormulaGrammar::AddressConvention eConv = FormulaGrammar::CONV_OOO;

        if (ScRangeStringConverter::GetRangeFromString( aLabelRange, rLabelRange.sLabelRangeStr, *mpDoc, eConv, nOffset1 ) &&
            ScRangeStringConverter::GetRangeFromString( aDataRange, rLabelRange.sDataRangeStr, *mpDoc, eConv, nOffset2 ))
        {
            if ( rLabelRange.bColumnOrientation )
                xColRanges->addNew( aLabelRange, aDataRange );
            else
                xRowRanges->addNew( aLabelRange, aDataRange );
        }
    }

    maMyLabelRanges.clear();
}

namespace {

class RangeNameInserter
{
    ScDocument&  mrDoc;
    ScRangeName& mrRangeName;
    SCTAB        mnTab;

public:
    RangeNameInserter(ScDocument& rDoc, ScRangeName& rRangeName, SCTAB nTab) :
        mrDoc(rDoc), mrRangeName(rRangeName), mnTab(nTab) {}

    void operator() (const ScMyNamedExpression& p) const
    {
        using namespace formula;

        const OUString& aType = p.sRangeType;
        sal_uInt32 nUnoType = ScXMLImport::GetRangeType(aType);

        ScRangeData::Type nNewType = ScRangeData::Type::Name;
        if ( nUnoType & sheet::NamedRangeFlag::FILTER_CRITERIA )    nNewType |= ScRangeData::Type::Criteria;
        if ( nUnoType & sheet::NamedRangeFlag::PRINT_AREA )         nNewType |= ScRangeData::Type::PrintArea;
        if ( nUnoType & sheet::NamedRangeFlag::COLUMN_HEADER )      nNewType |= ScRangeData::Type::ColHeader;
        if ( nUnoType & sheet::NamedRangeFlag::ROW_HEADER )         nNewType |= ScRangeData::Type::RowHeader;
        if ( nUnoType & sheet::NamedRangeFlag::HIDDEN )             nNewType |= ScRangeData::Type::Hidden;

        // Insert a new name.
        ScAddress aPos;
        sal_Int32 nOffset = 0;
        bool bSuccess = ScRangeStringConverter::GetAddressFromString(
            aPos, p.sBaseCellAddress, mrDoc, FormulaGrammar::CONV_OOO, nOffset);

        if (!bSuccess)
        {
            SAL_WARN("sc.filter", "No conversion from table:base-cell-address '" << p.sBaseCellAddress
                    << "' for name '" << p.sName << "' on sheet " << mnTab);
            // Do not lose the defined name. Relative addressing in
            // content/expression, if any, will be broken though.
            // May had happened due to tdf#150312.
            aPos.SetTab(mnTab < 0 ? 0 : mnTab);
            bSuccess = true;
        }

        if (bSuccess)
        {
            OUString aContent = p.sContent;
            if (!p.bIsExpression)
                ScXMLConverter::ConvertCellRangeAddress(aContent);

            ScRangeData* pData = new ScRangeData(
                mrDoc, p.sName, aContent, aPos, nNewType, p.eGrammar);
            mrRangeName.insert(pData);
        }
    }
};

}

void ScXMLImport::SetNamedRanges()
{
    if (m_aMyNamedExpressions.empty())
        return;

    if (!mpDoc)
        return;

    // Insert the namedRanges
    ScRangeName* pRangeNames = mpDoc->GetRangeName();
    ::std::for_each(m_aMyNamedExpressions.begin(), m_aMyNamedExpressions.end(),
            RangeNameInserter(*mpDoc, *pRangeNames, -1));
}

void ScXMLImport::SetSheetNamedRanges()
{
    if (!mpDoc)
        return;

    for (auto const& itr : m_SheetNamedExpressions)
    {
        const SCTAB nTab = itr.first;
        ScRangeName* pRangeNames = mpDoc->GetRangeName(nTab);
        if (!pRangeNames)
            continue;

        const ScMyNamedExpressions& rNames = itr.second;
        ::std::for_each(rNames.begin(), rNames.end(), RangeNameInserter(*mpDoc, *pRangeNames, nTab));
    }
}

void ScXMLImport::SetStringRefSyntaxIfMissing()
{
    if (!mpDoc)
        return;

    ScCalcConfig aCalcConfig = mpDoc->GetCalcConfig();

    // Has any string ref syntax been imported?
    // If not, we need to take action
    if ( !aCalcConfig.mbHasStringRefSyntax )
    {
        aCalcConfig.meStringRefAddressSyntax = formula::FormulaGrammar::CONV_A1_XL_A1;
        mpDoc->SetCalcConfig(aCalcConfig);
    }
}

void SAL_CALL ScXMLImport::endDocument()
{
    ScXMLImport::MutexGuard aGuard(*this);
    if (getImportFlags() & SvXMLImportFlags::CONTENT)
    {
        if (GetModel().is())
        {
            mpDocImport->finalize();

            rtl::Reference<ScModelObj> xViewDataSupplier(GetScModel());
            uno::Reference<container::XIndexAccess> xIndexAccess(xViewDataSupplier->getViewData());
            if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
            {
                uno::Sequence< beans::PropertyValue > aSeq;
                if (xIndexAccess->getByIndex(0) >>= aSeq)
                {
                    for (const auto& rProp : aSeq)
                    {
                        OUString sName(rProp.Name);
                        if (sName == SC_ACTIVETABLE)
                        {
                            OUString sTabName;
                            if(rProp.Value >>= sTabName)
                            {
                                SCTAB nTab(0);
                                if (mpDoc && mpDoc->GetTable(sTabName, nTab))
                                {
                                    mpDoc->SetVisibleTab(nTab);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            SetLabelRanges();
            SetNamedRanges();
            SetSheetNamedRanges();
            SetStringRefSyntaxIfMissing();
            if (mpPivotSources)
                // Process pivot table sources after the named ranges have been set.
                mpPivotSources->process();
        }
        GetProgressBarHelper()->End();  // make room for subsequent SfxProgressBars
        if (mpDoc)
        {
            mpDoc->CompileXML();

            // After CompileXML, links must be completely changed to the new URLs.
            // Otherwise, hasExternalFile for API wouldn't work (#i116940#),
            // and typing a new formula would create a second link with the same "real" file name.
            if (mpDoc->HasExternalRefManager())
                mpDoc->GetExternalRefManager()->updateAbsAfterLoad();
        }

        // If the stream contains cells outside of the current limits, the styles can't be re-created,
        // so stream copying is disabled then.
        if (mpDoc && GetModel().is() && !mpDoc->HasRangeOverflow())
        {
            // set "valid stream" flags after loading (before UpdateRowHeights, so changed formula results
            // in UpdateRowHeights can already clear the flags again)
            ScSheetSaveData* pSheetData = GetScModel()->GetSheetSaveData();

            SCTAB nTabCount = mpDoc->GetTableCount();
            for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
            {
                mpDoc->SetDrawPageSize(nTab);
                if (!pSheetData->IsSheetBlocked( nTab ))
                    mpDoc->SetStreamValid( nTab, true );
            }
        }

        // There are rows with optimal height which need to be updated
        if (mpDoc && !maRecalcRowRanges.empty() && mpDoc->GetDocumentShell()
            && mpDoc->GetDocumentShell()->GetRecalcRowHeightsMode())
        {
            bool bLockHeight = mpDoc->IsAdjustHeightLocked();
            if (bLockHeight)
            {
                mpDoc->UnlockAdjustHeight();
            }

            ScSizeDeviceProvider aProv(*mpDoc->GetDocumentShell());
            ScDocRowHeightUpdater aUpdater(*mpDoc, aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), &maRecalcRowRanges);
            aUpdater.update();

            if (bLockHeight)
            {
                mpDoc->LockAdjustHeight();
            }
        }

        // Initialize and set position and size of objects
        if (mpDoc && mpDoc->GetDrawLayer())
        {
            ScDrawLayer* pDrawLayer = mpDoc->GetDrawLayer();
            SCTAB nTabCount = mpDoc->GetTableCount();
            for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
            {
                const SdrPage* pPage = pDrawLayer->GetPage(nTab);
                if (!pPage)
                    continue;
                bool bNegativePage = mpDoc->IsNegativePage(nTab);
                for (const rtl::Reference<SdrObject>& pObj : *pPage)
                {
                    ScDrawObjData* pData
                        = ScDrawLayer::GetObjDataTab(pObj.get(), nTab);
                    // Existence of pData means, that it is a cell anchored object
                    if (pData)
                    {
                        // Finish and correct import based on full size (no hidden row/col) and LTR
                        pDrawLayer->InitializeCellAnchoredObj(pObj.get(), *pData);
                        // Adapt object to hidden row/col and RTL
                        pDrawLayer->RecalcPos(pObj.get(), *pData, bNegativePage,
                                              true /*bUpdateNoteCaptionPos*/);
                    }
                }
            }
        }

        aTables.FixupOLEs();
    }
    if (GetScModel())
    {
        GetScModel()->removeActionLock();
    }
    SvXMLImport::endDocument();

    if (mpDoc)
    {
        mpDoc->BroadcastUno(SfxHint(SfxHintId::ScClearCache));
    }

    if(mpDoc && bSelfImportingXMLSet)
        GetScModel()->AfterXMLLoading();
}

// XEventListener
void ScXMLImport::DisposingModel()
{
    SvXMLImport::DisposingModel();
    mpDoc = nullptr;
}

ScXMLImport::MutexGuard::MutexGuard(ScXMLImport& rImport) :
    mrImport(rImport)
{
    mrImport.LockSolarMutex();
}

ScXMLImport::MutexGuard::~MutexGuard()
{
    mrImport.UnlockSolarMutex();
}

void ScXMLImport::LockSolarMutex()
{
    // #i62677# When called from DocShell/Wrapper, the SolarMutex is already locked,
    // so there's no need to allocate (and later delete) the SolarMutexGuard.
    if (!mbLockSolarMutex)
    {
        DBG_TESTSOLARMUTEX();
        return;
    }

    if (nSolarMutexLocked == 0)
    {
        OSL_ENSURE(!moSolarMutexGuard, "Solar Mutex is locked");
        moSolarMutexGuard.emplace();
    }
    ++nSolarMutexLocked;
}

void ScXMLImport::UnlockSolarMutex()
{
    if (nSolarMutexLocked > 0)
    {
        nSolarMutexLocked--;
        if (nSolarMutexLocked == 0)
        {
            OSL_ENSURE(moSolarMutexGuard, "Solar Mutex is always unlocked");
            moSolarMutexGuard.reset();
        }
    }
}

sal_Int64 ScXMLImport::GetByteOffset() const
{
    sal_Int64 nOffset = -1;
    uno::Reference<xml::sax::XLocator> xLocator = GetLocator();
    uno::Reference<io::XSeekable> xSeek( xLocator, uno::UNO_QUERY );        //! should use different interface
    if ( xSeek.is() )
        nOffset = xSeek->getPosition();
    return nOffset;
}

void ScXMLImport::SetRangeOverflowType(ErrCode nType)
{
    //  #i31130# Overflow is stored in the document, because the ScXMLImport object
    //  isn't available in ScXMLImportWrapper::ImportFromComponent when using the
    //  OOo->Oasis transformation.

    if ( mpDoc )
        mpDoc->SetRangeOverflowType( nType );
}

void ScXMLImport::ProgressBarIncrement()
{
    nProgressCount++;
    if (nProgressCount > 100)
    {
        GetProgressBarHelper()->Increment(nProgressCount);
        nProgressCount = 0;
    }
}

void ScXMLImport::ExtractFormulaNamespaceGrammar(
        OUString& rFormula, OUString& rFormulaNmsp, FormulaGrammar::Grammar& reGrammar,
        const OUString& rAttrValue, bool bRestrictToExternalNmsp ) const
{
    // parse the attribute value, extract namespace ID, literal namespace, and formula string
    rFormulaNmsp.clear();
    sal_uInt16 nNsId = GetNamespaceMap().GetKeyByQName(rAttrValue, nullptr, &rFormula, &rFormulaNmsp, SvXMLNamespaceMap::QNameMode::AttrValue);

    // check if we have an ODF formula namespace
    if( !bRestrictToExternalNmsp ) switch( nNsId )
    {
        case XML_NAMESPACE_OOOC:
            rFormulaNmsp.clear();  // remove namespace string for built-in grammar
            reGrammar = FormulaGrammar::GRAM_PODF;
            return;
        case XML_NAMESPACE_OF:
            rFormulaNmsp.clear();  // remove namespace string for built-in grammar
            reGrammar = FormulaGrammar::GRAM_ODFF;
            return;
    }

    if (!mpDoc)
        return;

    /*  Find default grammar for formulas without namespace. There may be
        documents in the wild that stored no namespace in ODF 1.0/1.1. Use
        GRAM_PODF then (old style ODF 1.0/1.1 formulas). The default for ODF
        1.2 and later without namespace is GRAM_ODFF (OpenFormula). */
    FormulaGrammar::Grammar eDefaultGrammar =
        (mpDoc->GetStorageGrammar() == FormulaGrammar::GRAM_PODF) ?
            FormulaGrammar::GRAM_PODF : FormulaGrammar::GRAM_ODFF;

    /*  Check if we have no namespace at all. The value XML_NAMESPACE_NONE
        indicates that there is no colon. If the first character of the
        attribute value is the equality sign, the value XML_NAMESPACE_UNKNOWN
        indicates that there is a colon somewhere in the formula string. */
    if( (nNsId == XML_NAMESPACE_NONE) || ((nNsId == XML_NAMESPACE_UNKNOWN) && (rAttrValue.toChar() == '=')) )
    {
        rFormula = rAttrValue;          // return entire string as formula
        reGrammar = eDefaultGrammar;
        return;
    }

    /*  Check if a namespace URL could be resolved from the attribute value.
        Use that namespace only, if the Calc document knows an associated
        external formula parser. This prevents that the range operator in
        conjunction with defined names is confused as namespaces prefix, e.g.
        in the expression 'table:A1' where 'table' is a named reference. */
    if( ((nNsId & XML_NAMESPACE_UNKNOWN_FLAG) != 0) && !rFormulaNmsp.isEmpty() &&
        mpDoc->GetFormulaParserPool().hasFormulaParser( rFormulaNmsp ) )
    {
        reGrammar = FormulaGrammar::GRAM_EXTERNAL;
        return;
    }

    /*  All attempts failed (e.g. no namespace and no leading equality sign, or
        an invalid namespace prefix), continue with the entire attribute value. */
    rFormula = rAttrValue;
    rFormulaNmsp.clear();  // remove any namespace string
    reGrammar = eDefaultGrammar;
}

FormulaError ScXMLImport::GetFormulaErrorConstant( const OUString& rStr ) const
{
    if (!mpComp)
        return FormulaError::NONE;

    return mpComp->GetErrorConstant(rStr);
}

ScEditEngineDefaulter* ScXMLImport::GetEditEngine()
{
    if (!mpEditEngine && mpDoc)
    {
        mpEditEngine.reset(new ScEditEngineDefaulter(mpDoc->GetEditEnginePool()));
        mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        mpEditEngine->SetUpdateLayout(false);
        mpEditEngine->EnableUndo(false);
        mpEditEngine->SetControlWord(mpEditEngine->GetControlWord() & ~EEControlBits::ALLOWBIGOBJS);
    }
    return mpEditEngine.get();
}

const ScXMLEditAttributeMap& ScXMLImport::GetEditAttributeMap() const
{
    if (!mpEditAttrMap)
        mpEditAttrMap.reset(new ScXMLEditAttributeMap);
    return *mpEditAttrMap;
}

void ScXMLImport::NotifyContainsEmbeddedFont()
{
    if (mpDoc)
        mpDoc->SetEmbedFonts(true);
}

ScMyImpDetectiveOpArray* ScXMLImport::GetDetectiveOpArray()
{
    if (!pDetectiveOpArray)
        pDetectiveOpArray.reset(new ScMyImpDetectiveOpArray());
    return pDetectiveOpArray.get();
}

ScModelObj* ScXMLImport::GetScModel() const
{
    return static_cast<ScModelObj*>(GetModel().get());
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportFODS(SvStream &rStream)
{
    ScDLL::Init();

    SfxObjectShellLock xDocSh(new ScDocShell);
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new ::utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr), uno::UNO_SET_THROW);

    css::uno::Sequence<OUString> aUserData
    {
        u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
        u""_ustr,
        u"com.sun.star.comp.Calc.XMLOasisImporter"_ustr,
        u"com.sun.star.comp.Calc.XMLOasisExporter"_ustr,
        u""_ustr,
        u""_ustr,
        u"true"_ustr
    };
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(u"private:stream"_ustr) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestFODSExportXLS(SvStream &rStream)
{
    ScDLL::Init();

    SfxObjectShellLock xDocSh(new ScDocShell);
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new ::utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr), uno::UNO_SET_THROW);

    css::uno::Sequence<OUString> aUserData
    {
        u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
        u""_ustr,
        u"com.sun.star.comp.Calc.XMLOasisImporter"_ustr,
        u"com.sun.star.comp.Calc.XMLOasisExporter"_ustr,
        u""_ustr,
        u""_ustr,
        u"true"_ustr
    };
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(u"private:stream"_ustr) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    if (ret)
    {
        utl::TempFileFast aTempFile;

        uno::Reference<document::XFilter> xXLSFilter(
            xMultiServiceFactory->createInstance(u"com.sun.star.comp.oox.xls.ExcelFilter"_ustr), uno::UNO_QUERY);
        uno::Reference<document::XExporter> xExporter(xXLSFilter, uno::UNO_QUERY);
        xExporter->setSourceDocument(xModel);

        uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(*aTempFile.GetStream(StreamMode::READWRITE)));

        uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence({
        }));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
            { "FilterName", uno::Any(u"Excel 2007–365"_ustr) },
            { "OutputStream", uno::Any(xOutputStream) },
            { "FilterData", uno::Any(aFilterData) }
        }));
        xXLSFilter->filter(aDescriptor);
    }

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportXLSX(SvStream &rStream)
{
    ScDLL::Init();

    SfxObjectShellLock xDocSh(new ScDocShell);
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));

    uno::Reference<document::XFilter> xFilter(xMultiServiceFactory->createInstance(u"com.sun.star.comp.oox.xls.ExcelFilter"_ustr), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "InputMode", uno::Any(true) },
    }));
    xImporter->setTargetDocument(xModel);

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = false;
    try
    {
        SolarMutexGuard aGuard;
        ret = xFilter->filter(aArgs);
    }
    catch (const css::io::IOException&)
    {
    }
    catch (const css::lang::WrappedTargetRuntimeException&)
    {
    }
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
