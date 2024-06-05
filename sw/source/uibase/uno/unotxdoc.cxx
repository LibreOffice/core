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
#include <officecfg/Office/Common.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <AnnotationWin.hxx>
#include <o3tl/any.hxx>
#include <utility>
#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/print.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/LokControlHandler.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/ipclient.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/acorrcfg.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <wdocsh.hxx>
#include <wrtsh.hxx>
#include <pview.hxx>
#include <viewsh.hxx>
#include <pvprtdat.hxx>
#include <printdata.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <svl/stritem.hxx>
#include <unotxdoc.hxx>
#include <svl/numformat.hxx>
#include <svl/numuno.hxx>
#include <fldbas.hxx>
#include <unomap.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unosett.hxx>
#include <unocoll.hxx>
#include <unoredlines.hxx>
#include <unosrch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include <unoprnms.hxx>
#include <unostyle.hxx>
#include <unodraw.hxx>
#include <svl/eitem.hxx>
#include <unotools/datetime.hxx>
#include <unocrsr.hxx>
#include <unofieldcoll.hxx>
#include <unoidxcoll.hxx>
#include <unocrsrhelper.hxx>
#include <globdoc.hxx>
#include <viewopt.hxx>
#include <unochart.hxx>
#include <charatr.hxx>
#include <svx/xmleohlp.hxx>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/RedlineDisplayType.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <sfx2/linkmgr.hxx>
#include <svx/unofill.hxx>
#include <swmodule.hxx>
#include <docstat.hxx>
#include <modcfg.hxx>
#include <ndtxt.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include "unodefaults.hxx"
#include "SwXDocumentSettings.hxx"
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>
#include <drawdoc.hxx>
#include <SwStyleNameMapper.hxx>
#include <osl/file.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/dispatch.hxx>
#include <swruler.hxx>
#include <docufld.hxx>

#include <EnhancedPDFExportHelper.hxx>
#include <numrule.hxx>

#include <editeng/langitem.hxx>
#include <docary.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>

#include <charfmt.hxx>
#include <fmtcol.hxx>
#include <istyleaccess.hxx>

#include <swatrset.hxx>
#include <view.hxx>
#include <viscrs.hxx>
#include <srcview.hxx>
#include <edtwin.hxx>
#include <swdtflvr.hxx>
#include <PostItMgr.hxx>

#include <svtools/langtab.hxx>
#include <map>
#include <set>
#include <vector>

#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdview.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/servicehelper.hxx>
#include <memory>
#include <redline.hxx>
#include <DocumentRedlineManager.hxx>
#include <xmloff/odffields.hxx>
#include <tools/json_writer.hxx>
#include <tools/UnitConversion.hxx>

#include <svx/svdpage.hxx>
#include <o3tl/string_view.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <IDocumentOutlineNodes.hxx>
#include <SearchResultLocator.hxx>
#include <textcontentcontrol.hxx>
#include <unocontentcontrol.hxx>
#include <unoport.hxx>
#include <unobookmark.hxx>
#include <unosection.hxx>
#include <unofield.hxx>
#include <unoframe.hxx>
#include <unoxstyle.hxx>
#include <unocontentcontrol.hxx>
#include <SwXTextDefaults.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using ::osl::FileBase;

static std::unique_ptr<SwPrintUIOptions> lcl_GetPrintUIOptions(
    SwDocShell * pDocShell,
    const SfxViewShell * pView )
{
    if (!pDocShell)
        return nullptr;

    const bool bWebDoc      = nullptr != dynamic_cast< const SwWebDocShell * >(pDocShell);
    const bool bSwSrcView   = nullptr != dynamic_cast< const SwSrcView * >(pView);
    const SwView * pSwView = dynamic_cast< const SwView * >(pView);
    const bool bHasSelection    = pSwView && pSwView->HasSelection( false );  // check for any selection, not just text selection
    const bool bHasPostIts      = sw_GetPostIts(pDocShell->GetDoc()->getIDocumentFieldsAccess(), nullptr);

    // get default values to use in dialog from documents SwPrintData
    const SwPrintData &rPrintData = pDocShell->GetDoc()->getIDocumentDeviceAccess().getPrintData();

    // Get current page number
    sal_uInt16 nCurrentPage = 1;
    const SwWrtShell* pSh = pDocShell->GetWrtShell();
    const SwRootFrame *pFrame = nullptr;
    if (pSh)
    {
        SwPaM* pShellCursor = pSh->GetCursor();
        nCurrentPage = pShellCursor->GetPageNum();
        pFrame = pSh->GetLayout();
    }
    else if (!bSwSrcView)
    {
        const SwPagePreview* pPreview = dynamic_cast< const SwPagePreview* >(pView);
        OSL_ENSURE(pPreview, "Unexpected type of the view shell");
        if (pPreview)
        {
            nCurrentPage = pPreview->GetSelectedPage();
            pFrame = pPreview->GetViewShell()->GetLayout();
        }
    }

    // If blanks are skipped, account for them in initial page range value
    if (pFrame && !rPrintData.IsPrintEmptyPages())
    {
        sal_uInt16 nMax = nCurrentPage;
        const SwPageFrame *pPage = dynamic_cast<const SwPageFrame*>(pFrame->Lower());
        while (pPage && nMax-- > 0)
        {
            if (pPage->getFrameArea().Height() == 0)
                nCurrentPage--;
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
        }
    }
    return std::make_unique<SwPrintUIOptions>( nCurrentPage, bWebDoc, bSwSrcView, bHasSelection, bHasPostIts, rPrintData );
}

static SwTextFormatColl *lcl_GetParaStyle(const OUString& rCollName, SwDoc& rDoc)
{
    SwTextFormatColl* pColl = rDoc.FindTextFormatCollByName( rCollName );
    if( !pColl )
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
            rCollName, SwGetPoolIdFromName::TxtColl );
        if( USHRT_MAX != nId )
            pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( nId );
    }
    return pColl;
}

static void lcl_DisposeView( SfxViewFrame* pToClose, SwDocShell const * pDocShell )
{
    // check if the view frame still exists
    SfxViewFrame* pFound = SfxViewFrame::GetFirst( pDocShell, false );
    while(pFound)
    {
        if( pFound == pToClose)
        {
            pToClose->DoClose();
            break;
        }
        pFound = SfxViewFrame::GetNext( *pFound, pDocShell, false );
    }
}

class SwXTextDocument::Impl
{
public:
    std::mutex m_Mutex; // just for OInterfaceContainerHelper4
    ::comphelper::OInterfaceContainerHelper4<css::util::XRefreshListener> m_RefreshListeners;
};

const Sequence< sal_Int8 > & SwXTextDocument::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXTextDocumentUnoTunnelId;
    return theSwXTextDocumentUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SwXTextDocument::getSomething( const Sequence< sal_Int8 >& rId )
{
    if( comphelper::isUnoTunnelId<SwXTextDocument>(rId) )
    {
        return comphelper::getSomething_cast(this);
    }
    if( comphelper::isUnoTunnelId<SfxObjectShell>(rId) )
    {
        return comphelper::getSomething_cast(m_pDocShell);
    }

    sal_Int64 nRet = SfxBaseModel::getSomething( rId );
    if (nRet)
        return nRet;

    GetNumberFormatter();
    if (!m_xNumFormatAgg.is()) // may happen if not valid or no SwDoc
        return 0;
    Any aNumTunnel = m_xNumFormatAgg->queryAggregation(cppu::UnoType<XUnoTunnel>::get());
    Reference<XUnoTunnel> xNumTunnel;
    aNumTunnel >>= xNumTunnel;
    return (xNumTunnel.is()) ? xNumTunnel->getSomething(rId) : 0;
}

Any SAL_CALL SwXTextDocument::queryInterface( const uno::Type& rType )
{
    Any aRet = SwXTextDocumentBaseClass::queryInterface(rType);
    if ( !aRet.hasValue() )
        aRet = SfxBaseModel::queryInterface(rType);
    if ( !aRet.hasValue() &&
        rType == cppu::UnoType<lang::XMultiServiceFactory>::get())
    {
        Reference<lang::XMultiServiceFactory> xTmp = this;
        aRet <<= xTmp;
    }
    if ( !aRet.hasValue() &&
        rType == cppu::UnoType<tiledrendering::XTiledRenderable>::get())
    {
        Reference<tiledrendering::XTiledRenderable> xTmp = this;
        aRet <<= xTmp;
    }

    if ( !aRet.hasValue()
        && rType != cppu::UnoType<css::document::XDocumentEventBroadcaster>::get()
        && rType != cppu::UnoType<css::frame::XController>::get()
        && rType != cppu::UnoType<css::frame::XFrame>::get()
        && rType != cppu::UnoType<css::script::XInvocation>::get()
        && rType != cppu::UnoType<css::beans::XFastPropertySet>::get()
        && rType != cppu::UnoType<css::awt::XWindow>::get())
    {
        GetNumberFormatter();
        if(m_xNumFormatAgg.is())
            aRet = m_xNumFormatAgg->queryAggregation(rType);
    }
    return aRet;
}

void SAL_CALL SwXTextDocument::acquire()noexcept
{
    SfxBaseModel::acquire();
}

void SAL_CALL SwXTextDocument::release()noexcept
{
    SfxBaseModel::release();
}

Sequence< uno::Type > SAL_CALL SwXTextDocument::getTypes()
{
    Sequence< uno::Type > aNumTypes;
    GetNumberFormatter();
    if(m_xNumFormatAgg.is())
    {
        const uno::Type& rProvType = cppu::UnoType<XTypeProvider>::get();
        Any aNumProv = m_xNumFormatAgg->queryAggregation(rProvType);
        Reference<XTypeProvider> xNumProv;
        if(aNumProv >>= xNumProv)
        {
            aNumTypes = xNumProv->getTypes();
        }
    }
    return comphelper::concatSequences(
            SfxBaseModel::getTypes(),
            SwXTextDocumentBaseClass::getTypes(),
            aNumTypes,
            Sequence {
                cppu::UnoType<lang::XMultiServiceFactory>::get(),
                cppu::UnoType<tiledrendering::XTiledRenderable>::get()});
}

SwXTextDocument::SwXTextDocument(SwDocShell* pShell)
    : SwXTextDocumentBaseClass(pShell)
    , m_pImpl(new Impl)
    ,
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_DOCUMENT)),
    m_pDocShell(pShell),
    m_bObjectValid(pShell != nullptr),
    m_pHiddenViewFrame(nullptr),
    // #i117783#
    m_bApplyPagePrintSettingsFromXPagePrintable( false )
{
}

void SwXTextDocument::ThrowIfInvalid() const
{
    if (!m_bObjectValid)
        throw DisposedException(u"SwXTextDocument not valid"_ustr,
                                const_cast<SwXTextDocument*>(this)->getXWeak());
}

SwDoc& SwXTextDocument::GetDocOrThrow() const
{
    ThrowIfInvalid();
    if (SwDoc* pDoc = m_pDocShell->GetDoc())
        return *pDoc;
    throw css::lang::NotInitializedException(
        u"Document not initialized by a call to attachResource() or load()"_ustr,
        const_cast<SwXTextDocument*>(this)->getXWeak());
}

SdrModel& SwXTextDocument::getSdrModelFromUnoModel() const
{
    assert(GetDocOrThrow().getIDocumentDrawModelAccess().GetOrCreateDrawModel());
    return *GetDocOrThrow().getIDocumentDrawModelAccess().GetDrawModel();
}

SwXTextDocument::~SwXTextDocument()
{
    InitNewDoc();
    if(m_xNumFormatAgg.is())
    {
        Reference< XInterface >  x0;
        m_xNumFormatAgg->setDelegator(x0);
        m_xNumFormatAgg = nullptr;
    }
    m_pPrintUIOptions.reset();
    if (m_pRenderData && m_pRenderData->IsViewOptionAdjust())
    {   // rhbz#827695: this can happen if the last page is not printed
        // the SwViewShell has been deleted already by SwView::~SwView
        // FIXME: replace this awful implementation of XRenderable with
        // something less insane that has its own view
        m_pRenderData->ViewOptionAdjustCrashPreventionKludge();
    }
    m_pRenderData.reset();
}

SwXDocumentPropertyHelper * SwXTextDocument::GetPropertyHelper ()
{
    if(!mxPropertyHelper.is())
    {
        mxPropertyHelper = new SwXDocumentPropertyHelper(GetDocOrThrow());
    }
    return mxPropertyHelper.get();
}

void SwXTextDocument::GetNumberFormatter()
{
    if (!m_bObjectValid)
        return;

    if(!m_xNumFormatAgg.is())
    {
        if ( m_pDocShell->GetDoc() )
        {
            rtl::Reference<SvNumberFormatsSupplierObj> pNumFormat = new SvNumberFormatsSupplierObj(
                                m_pDocShell->GetDoc()->GetNumberFormatter());
            m_xNumFormatAgg = pNumFormat;
        }
        if(m_xNumFormatAgg.is())
            m_xNumFormatAgg->setDelegator(getXWeak());
    }
    else
    {
        const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
        Any aNumTunnel = m_xNumFormatAgg->queryAggregation(rTunnelType);
        Reference< XUnoTunnel > xNumTunnel;
        aNumTunnel >>= xNumTunnel;
        SvNumberFormatsSupplierObj* pNumFormat
            = comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>(xNumTunnel);
        OSL_ENSURE(pNumFormat, "No number formatter available");
        if (pNumFormat && !pNumFormat->GetNumberFormatter())
            pNumFormat->SetNumberFormatter(GetDocOrThrow().GetNumberFormatter());
    }
}

Reference< XText >  SwXTextDocument::getText()
{
    return getBodyText();
}

rtl::Reference< SwXBodyText > SwXTextDocument::getBodyText()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!m_xBodyText.is())
    {
        m_xBodyText = new SwXBodyText(m_pDocShell->GetDoc());
    }
    return m_xBodyText;
}

void SwXTextDocument::reformat()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
}

void SwXTextDocument::lockControllers()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    maActionArr.emplace_front(new UnoActionContext(m_pDocShell->GetDoc()));
}

void SwXTextDocument::unlockControllers()
{
    SolarMutexGuard aGuard;
    if(maActionArr.empty())
        throw RuntimeException(u"Nothing to unlock"_ustr);

    maActionArr.pop_front();
}

sal_Bool SwXTextDocument::hasControllersLocked()
{
    SolarMutexGuard aGuard;
    return !maActionArr.empty();
}

Reference< frame::XController >  SwXTextDocument::getCurrentController()
{
    return SfxBaseModel::getCurrentController();
}

void SwXTextDocument::setCurrentController(const Reference< frame::XController > & xController)
{
    SfxBaseModel::setCurrentController(xController);
}

Reference< XInterface >  SwXTextDocument::getCurrentSelection()
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xRef;
    if (m_bObjectValid)
    {
        SwView* pView = static_cast<SwView*>(SfxViewShell::GetFirst(true, checkSfxViewShell<SwView>));
        while(pView && pView->GetObjectShell() != m_pDocShell)
        {
            pView = static_cast<SwView*>(SfxViewShell::GetNext(*pView, true, checkSfxViewShell<SwView>));
        }
        if(pView)
        {
            Any aRef = pView->GetUNOObject()->getSelection();
            aRef >>= xRef;
        }
    }
    return xRef;
}

sal_Bool SwXTextDocument::attachResource(const OUString& aURL, const Sequence< beans::PropertyValue >& aArgs)
{
    return SfxBaseModel::attachResource(aURL, aArgs);
}

OUString SwXTextDocument::getURL()
{
    return SfxBaseModel::getURL();
}

Sequence< beans::PropertyValue > SwXTextDocument::getArgs()
{
    return SfxBaseModel::getArgs();
}

void SwXTextDocument::connectController(const Reference< frame::XController > & xController)
{
    SfxBaseModel::connectController(xController);
}

void SwXTextDocument::disconnectController(const Reference< frame::XController > & xController)
{
    SfxBaseModel::disconnectController(xController);
}

void SwXTextDocument::dispose()
{
    // Delete UnoActionContexts before deleting the SwDoc, as the first has unowned pointers to the
    // second.
    maActionArr.clear();

    SfxBaseModel::dispose();
}

void SwXTextDocument::close( sal_Bool bDeliverOwnership )
{
    if(m_pDocShell)
    {
        uno::Sequence< uno::Any > aArgs;
        m_pDocShell->CallAutomationDocumentEventSinks( u"Close"_ustr, aArgs );
    }
    SolarMutexGuard aGuard;
    if (m_bObjectValid && m_pHiddenViewFrame)
        lcl_DisposeView( m_pHiddenViewFrame, m_pDocShell);
    SfxBaseModel::close(bDeliverOwnership);
}

void SwXTextDocument::addEventListener(const Reference< lang::XEventListener > & aListener)
{
    SfxBaseModel::addEventListener(aListener);
}

void SwXTextDocument::removeEventListener(const Reference< lang::XEventListener > & aListener)
{
    SfxBaseModel::removeEventListener(aListener);
}

Reference< XPropertySet > SwXTextDocument::getLineNumberingProperties()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    if(!mxXLineNumberingProperties.is())
    {
        mxXLineNumberingProperties = new SwXLineNumberingProperties(m_pDocShell->GetDoc());
    }
    return mxXLineNumberingProperties;
}

Reference< XIndexReplace >  SwXTextDocument::getChapterNumberingRules()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXChapterNumbering.is())
    {
        mxXChapterNumbering = new SwXChapterNumbering(*m_pDocShell);
    }
    return mxXChapterNumbering;
}

Reference< XIndexAccess >  SwXTextDocument::getNumberingRules()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXNumberingRules.is() )
    {
        mxXNumberingRules = new SwXNumberingRulesCollection( m_pDocShell->GetDoc() );
    }
    return mxXNumberingRules;
}

Reference< XIndexAccess >  SwXTextDocument::getFootnotes()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXFootnotes.is())
    {
        mxXFootnotes = new SwXFootnotes(false, m_pDocShell->GetDoc());
    }
    return mxXFootnotes;
}

Reference< XPropertySet >  SAL_CALL
        SwXTextDocument::getFootnoteSettings()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXFootnoteSettings.is())
    {
        mxXFootnoteSettings = new SwXFootnoteProperties(m_pDocShell->GetDoc());
    }
    return mxXFootnoteSettings;
}

Reference< XIndexAccess >  SwXTextDocument::getEndnotes()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXEndnotes.is())
    {
        mxXEndnotes = new SwXFootnotes(true, m_pDocShell->GetDoc());
    }
    return mxXEndnotes;
}

Reference< XPropertySet >  SwXTextDocument::getEndnoteSettings()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXEndnoteSettings.is())
    {
        mxXEndnoteSettings = new SwXEndnoteProperties(m_pDocShell->GetDoc());
    }
    return mxXEndnoteSettings;
}

Reference< XIndexAccess >  SwXTextDocument::getContentControls()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXContentControls.is())
    {
        mxXContentControls = new SwXContentControls(m_pDocShell->GetDoc());
    }
    return mxXContentControls;
}

Reference< util::XReplaceDescriptor >  SwXTextDocument::createReplaceDescriptor()
{
    return new SwXTextSearch;
}

SwUnoCursor* SwXTextDocument::CreateCursorForSearch(Reference< XTextCursor > & xCursor)
{
    rtl::Reference<SwXTextCursor> pXTextCursor = getBodyText()->CreateTextCursor(true);
    xCursor.set( static_cast<text::XWordCursor*>(pXTextCursor.get()) );

    auto& rUnoCursor(pXTextCursor->GetCursor());
    rUnoCursor.SetRemainInSection(false);
    return &rUnoCursor;
}

sal_Int32 SwXTextDocument::replaceAll(const Reference< util::XSearchDescriptor > & xDesc)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    auto* pSearch = dynamic_cast<SwXTextSearch*>(xDesc.get());
    if (!pSearch)
        throw DisposedException(u""_ustr, getXWeak());

    Reference< XTextCursor >  xCursor;
    auto pUnoCursor(CreateCursorForSearch(xCursor));
    FindRanges eRanges(FindRanges::InBody|FindRanges::InSelAll);

    i18nutil::SearchOptions2 aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

    SwDocPositions eStart = pSearch->m_bBack ? SwDocPositions::End : SwDocPositions::Start;
    SwDocPositions eEnd = pSearch->m_bBack ? SwDocPositions::Start : SwDocPositions::End;

    // Search should take place anywhere
    pUnoCursor->SetRemainInSection(false);
    sal_Int32 nResult;
    UnoActionContext aContext(m_pDocShell->GetDoc());
    //try attribute search first
    if(pSearch->HasSearchAttributes()||pSearch->HasReplaceAttributes())
    {
        auto& pool = GetDocOrThrow().GetAttrPool();
        SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1>  aSearch(pool);
        SfxItemSetFixed<RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1>  aReplace(pool);
        pSearch->FillSearchItemSet(aSearch);
        pSearch->FillReplaceItemSet(aReplace);
        bool bCancel;
        nResult = pUnoCursor->FindAttrs(aSearch, !pSearch->m_bStyles,
                    eStart, eEnd, bCancel,
                    eRanges,
                    !pSearch->m_sSearchText.isEmpty() ? &aSearchOpt : nullptr,
                    &aReplace );
    }
    else if(pSearch->m_bStyles)
    {
        SwTextFormatColl *pSearchColl = lcl_GetParaStyle(pSearch->m_sSearchText, pUnoCursor->GetDoc());
        SwTextFormatColl *pReplaceColl = lcl_GetParaStyle(pSearch->m_sReplaceText, pUnoCursor->GetDoc());

        bool bCancel;
        nResult = pUnoCursor->FindFormat(*pSearchColl,
                    eStart, eEnd, bCancel,
                    eRanges, pReplaceColl );

    }
    else
    {
        //todo/mba: assuming that notes should be omitted
        bool bCancel;
        nResult = pUnoCursor->Find_Text(aSearchOpt, false/*bSearchInNotes*/,
            eStart, eEnd, bCancel,
            eRanges,
            true );
    }
    return nResult;

}

Reference< util::XSearchDescriptor >  SwXTextDocument::createSearchDescriptor()
{
    return new SwXTextSearch;
}

// Used for findAll/First/Next

SwUnoCursor* SwXTextDocument::FindAny(const Reference< util::XSearchDescriptor > & xDesc,
                                     Reference< XTextCursor > & xCursor,
                                     bool bAll,
                                     sal_Int32& nResult,
                                     Reference< XInterface > const & xLastResult)
{
    ThrowIfInvalid();
    const auto pSearch = dynamic_cast<SwXTextSearch*>(xDesc.get());
    if (!pSearch)
        return nullptr;

    auto pUnoCursor(CreateCursorForSearch(xCursor));

    bool bParentInExtra = false;
    if(xLastResult.is())
    {
        OTextCursorHelper* pPosCursor = dynamic_cast<OTextCursorHelper*>(xLastResult.get());
        SwPaM* pCursor = pPosCursor ? pPosCursor->GetPaM() : nullptr;
        if(pCursor)
        {
            *pUnoCursor->GetPoint() = *pCursor->End();
            pUnoCursor->DeleteMark();
        }
        else
        {
            SwXTextRange* pRange = dynamic_cast<SwXTextRange*>(xLastResult.get());
            if(!pRange)
                return nullptr;
            pRange->GetPositions(*pUnoCursor);
            if(pUnoCursor->HasMark())
            {
                if(*pUnoCursor->GetPoint() < *pUnoCursor->GetMark())
                    pUnoCursor->Exchange();
                pUnoCursor->DeleteMark();
            }
        }
        const SwNode& rRangeNode = pUnoCursor->GetPointNode();
        bParentInExtra = rRangeNode.FindFlyStartNode() ||
                            rRangeNode.FindFootnoteStartNode() ||
                            rRangeNode.FindHeaderStartNode() ||
                            rRangeNode.FindFooterStartNode() ;
    }

    i18nutil::SearchOptions2 aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

/**
 * The following combinations are allowed:
 *  - Search in the body:                   -> FindRanges::InBody
 *  - Search all in the body:               -> FindRanges::InBodyOnly | FindRanges::InSelAll
 *  - Search in selections: one / all       -> FindRanges::InSel  [ | FindRanges::InSelAll ]
 *  - Search outside the body: one / all    -> FindRanges::InOther [ | FindRanges::InSelAll ]
 *  - Search everywhere all:                -> FindRanges::InSelAll
 */
    FindRanges eRanges(FindRanges::InBody);
    if(bParentInExtra)
        eRanges = FindRanges::InOther;
    if(bAll) //always - everywhere?
        eRanges = FindRanges::InSelAll;
    SwDocPositions eStart = !bAll ? SwDocPositions::Curr : pSearch->m_bBack ? SwDocPositions::End : SwDocPositions::Start;
    SwDocPositions eEnd = pSearch->m_bBack ? SwDocPositions::Start : SwDocPositions::End;

    nResult = 0;
    for (int nSearchProc = 0; nSearchProc < 2; ++nSearchProc)
    {
        //try attribute search first
        if(pSearch->HasSearchAttributes())
        {
            SfxItemSetFixed<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_PARATR_BEGIN, RES_PARATR_END - 1,
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1>
                 aSearch( GetDocOrThrow().GetAttrPool() );
            pSearch->FillSearchItemSet(aSearch);
            bool bCancel;
            nResult = pUnoCursor->FindAttrs(aSearch, !pSearch->m_bStyles,
                        eStart, eEnd, bCancel,
                        eRanges,
                        !pSearch->m_sSearchText.isEmpty() ? &aSearchOpt : nullptr );
        }
        else if(pSearch->m_bStyles)
        {
            SwTextFormatColl *pSearchColl = lcl_GetParaStyle(pSearch->m_sSearchText, pUnoCursor->GetDoc());
            //pSearch->sReplaceText
            SwTextFormatColl *pReplaceColl = nullptr;
            bool bCancel;
            nResult = pUnoCursor->FindFormat(*pSearchColl,
                        eStart, eEnd, bCancel,
                        eRanges, pReplaceColl );
        }
        else
        {
            //todo/mba: assuming that notes should be omitted
            bool bCancel;
            nResult = pUnoCursor->Find_Text(aSearchOpt, false/*bSearchInNotes*/,
                    eStart, eEnd, bCancel,
                    eRanges );
        }
        if(nResult || (eRanges&(FindRanges::InSelAll|FindRanges::InOther)))
            break;
        //second step - find in other
        eRanges = FindRanges::InOther;
    }
    return pUnoCursor;
}

Reference< XIndexAccess >
    SwXTextDocument::findAll(const Reference< util::XSearchDescriptor > & xDesc)
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xTmp;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCursor;
    auto pResultCursor(FindAny(xDesc, xCursor, true, nResult, xTmp));
    if(!pResultCursor)
        throw RuntimeException(u"No result cursor"_ustr);
    Reference< XIndexAccess >  xRet = SwXTextRanges::Create( nResult ? &(*pResultCursor) : nullptr );
    return xRet;
}

Reference< XInterface >  SwXTextDocument::findFirst(const Reference< util::XSearchDescriptor > & xDesc)
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xTmp;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCursor;
    auto pResultCursor(FindAny(xDesc, xCursor, false, nResult, xTmp));
    if(!pResultCursor)
        throw RuntimeException(u"No result cursor"_ustr);
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(GetDocOrThrow(),
                    *pResultCursor->GetPoint());
        xRet = *new SwXTextCursor(xParent, *pResultCursor);
    }
    return xRet;
}

Reference< XInterface >  SwXTextDocument::findNext(const Reference< XInterface > & xStartAt,
            const Reference< util::XSearchDescriptor > & xDesc)
{
    SolarMutexGuard aGuard;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCursor;
    if(!xStartAt.is())
        throw RuntimeException(u"xStartAt missing"_ustr);
    auto pResultCursor(FindAny(xDesc, xCursor, false, nResult, xStartAt));
    if(!pResultCursor)
        throw RuntimeException(u"No result cursor"_ustr);
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(GetDocOrThrow(),
                    *pResultCursor->GetPoint());

        xRet = *new SwXTextCursor(xParent, *pResultCursor);
    }
    return xRet;
}

Sequence< beans::PropertyValue > SwXTextDocument::getPagePrintSettings()
{
    SolarMutexGuard aGuard;
    Sequence< beans::PropertyValue > aSeq(9);
    ThrowIfInvalid();

    beans::PropertyValue* pArray = aSeq.getArray();
    SwPagePreviewPrtData aData;
    const SwPagePreviewPrtData* pData = GetDocOrThrow().GetPreviewPrtData();
    if(pData)
        aData = *pData;
    Any aVal;
    aVal <<= aData.GetRow();
    pArray[0] = beans::PropertyValue(u"PageRows"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= aData.GetCol();
    pArray[1] = beans::PropertyValue(u"PageColumns"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetLeftSpace()));
    pArray[2] = beans::PropertyValue(u"LeftMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetRightSpace()));
    pArray[3] = beans::PropertyValue(u"RightMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetTopSpace()));
    pArray[4] = beans::PropertyValue(u"TopMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetBottomSpace()));
    pArray[5] = beans::PropertyValue(u"BottomMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetHorzSpace()));
    pArray[6] = beans::PropertyValue(u"HoriMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= static_cast<sal_Int32>(convertTwipToMm100(aData.GetVertSpace()));
    pArray[7] = beans::PropertyValue(u"VertMargin"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= aData.GetLandscape();
    pArray[8] = beans::PropertyValue(u"IsLandscape"_ustr, -1, aVal, PropertyState_DIRECT_VALUE);

    return aSeq;
}

static sal_uInt32 lcl_Any_To_ULONG(const Any& rValue, bool& bException)
{
    bException = false;
    TypeClass eType = rValue.getValueType().getTypeClass();

    sal_uInt32 nRet = 0;
    if( eType == TypeClass_UNSIGNED_LONG )
        rValue >>= nRet;
    else
    {
         sal_Int32 nVal=0;
         bException = !(rValue >>= nVal);
         if( !bException )
            nRet = static_cast<sal_uInt32>(nVal);
    }

    return nRet;
}

static OUString lcl_CreateOutlineString(const size_t nIndex, const SwDoc* pDoc)
{
    OUStringBuffer sEntry;
    const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
    const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
    const SwTextNode * pTextNd = rOutlineNodes[ nIndex ]->GetTextNode();
    SwNumberTree::tNumberVector aNumVector = pTextNd->GetNumberVector();
    if( pOutlRule && pTextNd->GetNumRule())
        for( int nLevel = 0;
             nLevel <= pTextNd->GetActualListLevel();
             nLevel++ )
        {
            tools::Long nVal = aNumVector[nLevel];
            nVal ++;
            nVal -= pOutlRule->Get(nLevel).GetStart();
            sEntry.append( OUString::number(nVal) + ".");
        }
    OUString sOutlineText = pDoc->getIDocumentOutlineNodes().getOutlineText(
                nIndex, pDoc->GetDocShell()->GetWrtShell()->GetLayout(), false);
    sEntry.append(sOutlineText);
    return sEntry.makeStringAndClear();
}

void SwXTextDocument::setPagePrintSettings(const Sequence< beans::PropertyValue >& aSettings)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    SwPagePreviewPrtData aData;
    SwDoc& rDoc = GetDocOrThrow();
    //if only a few properties are coming, then use the current settings
    const SwPagePreviewPrtData* pData = rDoc.GetPreviewPrtData();
    if(pData)
        aData = *pData;
    for(const beans::PropertyValue& rProperty : aSettings)
    {
        OUString sName = rProperty.Name;
        const Any& rVal = rProperty.Value;
        bool bException;
        sal_uInt32 nVal = lcl_Any_To_ULONG(rVal, bException);
        if( sName == "PageRows" )
        {
            if(!nVal || nVal > 0xff)
                throw RuntimeException(u"Invalid value"_ustr);
            aData.SetRow(nVal);
        }
        else if(sName == "PageColumns")
        {
            if(!nVal  || nVal > 0xff)
                throw RuntimeException(u"Invalid value"_ustr);
            aData.SetCol(nVal);
        }
        else if(sName == "LeftMargin")
        {
            aData.SetLeftSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "RightMargin")
        {
            aData.SetRightSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "TopMargin")
        {
            aData.SetTopSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "BottomMargin")
        {
            aData.SetBottomSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "HoriMargin")
        {
            aData.SetHorzSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "VertMargin")
        {
            aData.SetVertSpace(o3tl::toTwips(nVal, o3tl::Length::mm100));
        }
        else if(sName == "IsLandscape")
        {
            std::optional<const bool> b = o3tl::tryAccess<bool>(rVal);
            bException = !b.has_value();
            if (b)
            {
                aData.SetLandscape(*b);
            }
        }
        else
            bException = true;
        if(bException)
            throw RuntimeException();
    }
    rDoc.SetPreviewPrtData(&aData);

}

void SwXTextDocument::printPages(const Sequence< beans::PropertyValue >& xOptions)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    SfxViewFrame* pFrame = SfxViewFrame::LoadHiddenDocument( *m_pDocShell, SfxInterfaceId(7) );
    SfxRequest aReq(FN_PRINT_PAGEPREVIEW, SfxCallMode::SYNCHRON,
                                GetDocOrThrow().GetAttrPool());
    aReq.AppendItem(SfxBoolItem(FN_PRINT_PAGEPREVIEW, true));

    for ( const beans::PropertyValue &rProp : xOptions )
    {
        // get Property-Value from options
        Any aValue( rProp.Value );

        // FileName-Property?
        if ( rProp.Name == UNO_NAME_FILE_NAME )
        {
            OUString sFileURL;
            if ( rProp.Value >>= sFileURL )
            {
                // Convert the File URL into a system dependent path, as the SalPrinter expects
                OUString sSystemPath;
                FileBase::getSystemPathFromFileURL ( sFileURL, sSystemPath );
                aReq.AppendItem(SfxStringItem( SID_FILE_NAME, sSystemPath ) );
            }
            else if ( rProp.Value.getValueType() != cppu::UnoType<void>::get() )
                throw IllegalArgumentException();
        }

        // CopyCount-Property
        else if ( rProp.Name == UNO_NAME_COPY_COUNT )
        {
            sal_Int32 nCopies = 0;
            aValue >>= nCopies;
            aReq.AppendItem(SfxInt16Item( SID_PRINT_COPIES, static_cast<sal_Int16>(nCopies) ) );
        }

        // Collate-Property
        else if ( rProp.Name == UNO_NAME_COLLATE )
        {
            std::optional<const bool> b = o3tl::tryAccess<bool>(rProp.Value);
            if ( !b.has_value() )
                throw IllegalArgumentException();
            aReq.AppendItem(SfxBoolItem( SID_PRINT_COLLATE, *b ) );

        }

        // Sort-Property
        else if ( rProp.Name == UNO_NAME_SORT )
        {
            std::optional<const bool> b = o3tl::tryAccess<bool>(rProp.Value);
            if ( !b.has_value() )
                throw IllegalArgumentException();

            aReq.AppendItem(SfxBoolItem( SID_PRINT_SORT, *b ) );
        }

        // Pages-Property
        else if ( rProp.Name == UNO_NAME_PAGES )
        {
            OUString sTmp;
            if ( !(rProp.Value >>= sTmp) )
                throw IllegalArgumentException();

            aReq.AppendItem( SfxStringItem( SID_PRINT_PAGES, sTmp ) );

        }
    }

    // #i117783#
    m_bApplyPagePrintSettingsFromXPagePrintable = true;
    pFrame->GetViewShell()->ExecuteSlot(aReq);
    // Frame close
    pFrame->DoClose();

}

Reference< XNameAccess >  SwXTextDocument::getReferenceMarks()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXReferenceMarks.is())
    {
        mxXReferenceMarks = new SwXReferenceMarks(m_pDocShell->GetDoc());
    }
    return mxXReferenceMarks;
}

Reference< XEnumerationAccess >  SwXTextDocument::getTextFields()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXTextFieldTypes.is())
    {
        mxXTextFieldTypes = new SwXTextFieldTypes(m_pDocShell->GetDoc());
    }
    return mxXTextFieldTypes;
}

Reference< XNameAccess >  SwXTextDocument::getTextFieldMasters()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXTextFieldMasters.is())
    {
        mxXTextFieldMasters = new SwXTextFieldMasters(m_pDocShell->GetDoc());
    }
    return mxXTextFieldMasters;
}

Reference< XNameAccess >  SwXTextDocument::getEmbeddedObjects()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXEmbeddedObjects.is())
    {
        mxXEmbeddedObjects = new SwXTextEmbeddedObjects(m_pDocShell->GetDoc());
    }
    return mxXEmbeddedObjects;
}

Reference< XNameAccess >  SwXTextDocument::getBookmarks()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXBookmarks.is())
    {
        mxXBookmarks = new SwXBookmarks(m_pDocShell->GetDoc());
    }
    return mxXBookmarks;
}

Reference< XNameAccess >  SwXTextDocument::getTextSections()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXTextSections.is())
    {
        mxXTextSections = new SwXTextSections(m_pDocShell->GetDoc());
    }
    return mxXTextSections;
}

Reference< XNameAccess >  SwXTextDocument::getTextTables()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXTextTables.is())
    {
        mxXTextTables = new SwXTextTables(m_pDocShell->GetDoc());
    }
    return mxXTextTables;
}

Reference< XNameAccess >  SwXTextDocument::getGraphicObjects()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXGraphicObjects.is())
    {
        mxXGraphicObjects = new SwXTextGraphicObjects(m_pDocShell->GetDoc());
    }
    return mxXGraphicObjects;
}

Reference< XNameAccess >  SwXTextDocument::getTextFrames()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXTextFrames.is())
    {
        mxXTextFrames = new SwXTextFrames(m_pDocShell->GetDoc());
    }
    return mxXTextFrames;
}

Reference< XNameAccess >  SwXTextDocument::getStyleFamilies()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXStyleFamilies.is())
    {
        mxXStyleFamilies = new SwXStyleFamilies(*m_pDocShell);
    }
    return mxXStyleFamilies;
}

uno::Reference< style::XAutoStyles > SwXTextDocument::getAutoStyles(  )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!mxXAutoStyles.is())
    {
        mxXAutoStyles = new SwXAutoStyles(*m_pDocShell);
    }
    return mxXAutoStyles;

}

Reference< drawing::XDrawPage >  SwXTextDocument::getDrawPage()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    if(!m_xDrawPage.is())
    {
        SwDoc& rDoc = GetDocOrThrow();
        // #i52858#
        SwDrawModel* pModel = rDoc.getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        SdrPage* pPage = pModel->GetPage( 0 );
        m_xDrawPage = new SwFmDrawPage(&rDoc, pPage);
    }
    return m_xDrawPage;
}

namespace {

class SwDrawPagesObj : public cppu::WeakImplHelper<
    css::drawing::XDrawPages,
    css::lang::XServiceInfo>
{
private:
    css::uno::Reference< css::drawing::XDrawPageSupplier > m_xDoc;
public:
    SwDrawPagesObj(css::uno::Reference< css::drawing::XDrawPageSupplier > xDoc) : m_xDoc(std::move(xDoc)) {}

    // XDrawPages
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
        insertNewByIndex(sal_Int32 /*nIndex*/) override { throw css::lang::NoSupportException(); }

    virtual void SAL_CALL remove(const css::uno::Reference< css::drawing::XDrawPage >& /*xPage*/) override
    {
        throw css::lang::NoSupportException();
    }

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override { return 1; }

    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 Index) override
    {
        if (Index != 0)
            throw css::lang::IndexOutOfBoundsException(u"Writer documents have only one DrawPage!"_ustr);
        return css::uno::Any(m_xDoc->getDrawPage());
    }

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<drawing::XDrawPage>::get();
    }

    virtual sal_Bool SAL_CALL hasElements() override { return true; }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"SwDrawPagesObj"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.drawing.DrawPages"_ustr };
    }
};

}

// XDrawPagesSupplier

uno::Reference<drawing::XDrawPages> SAL_CALL SwXTextDocument::getDrawPages()
{
    SolarMutexGuard aGuard;
    return new SwDrawPagesObj(this);
}

void SwXTextDocument::Invalidate()
{
    m_bObjectValid = false;
    if(m_xNumFormatAgg.is())
    {
        const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
        Any aNumTunnel = m_xNumFormatAgg->queryAggregation(rTunnelType);
        Reference< XUnoTunnel > xNumTunnel;
        aNumTunnel >>= xNumTunnel;
        SvNumberFormatsSupplierObj* pNumFormat
            = comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>(xNumTunnel);
        OSL_ENSURE(pNumFormat, "No number formatter available");
        if (pNumFormat)
            pNumFormat->SetNumberFormatter(nullptr);
        OSL_ENSURE(pNumFormat, "No number formatter available");
    }
    InitNewDoc();
    m_pDocShell = nullptr;
    lang::EventObject const ev(getXWeak());
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_RefreshListeners.disposeAndClear(aGuard, ev);
}

void SwXTextDocument::Reactivate(SwDocShell* pNewDocShell)
{
    if(m_pDocShell && m_pDocShell != pNewDocShell)
        Invalidate();
    m_pDocShell = pNewDocShell;
    m_bObjectValid = true;
}

void    SwXTextDocument::InitNewDoc()
{
    // first invalidate all collections, then delete references and Set to zero
    if(mxXTextTables.is())
    {
        mxXTextTables->Invalidate();
        mxXTextTables.clear();
    }

    if(mxXTextFrames.is())
    {
        mxXTextFrames->Invalidate();
        mxXTextFrames.clear();
    }

    if(mxXGraphicObjects.is())
    {
        mxXGraphicObjects->Invalidate();
        mxXGraphicObjects.clear();
    }

    if(mxXEmbeddedObjects.is())
    {
        mxXEmbeddedObjects->Invalidate();
        mxXEmbeddedObjects.clear();
    }

    m_xBodyText.clear();

    if(m_xNumFormatAgg.is())
    {
        const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
        Any aNumTunnel = m_xNumFormatAgg->queryAggregation(rTunnelType);
        Reference< XUnoTunnel > xNumTunnel;
        aNumTunnel >>= xNumTunnel;
        SvNumberFormatsSupplierObj* pNumFormat
            = comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>(xNumTunnel);
        OSL_ENSURE(pNumFormat, "No number formatter available");
        if (pNumFormat)
            pNumFormat->SetNumberFormatter(nullptr);
    }

    if(mxXTextFieldTypes.is())
    {
        mxXTextFieldTypes->Invalidate();
        mxXTextFieldTypes.clear();
    }

    if(mxXTextFieldMasters.is())
    {
        mxXTextFieldMasters->Invalidate();
        mxXTextFieldMasters.clear();
    }

    if(mxXTextSections.is())
    {
        mxXTextSections->Invalidate();
        mxXTextSections.clear();
    }

    if(m_xDrawPage.is())
    {
        // #i91798#, #i91895#
        // dispose XDrawPage here. We are the owner and know that it is no longer in a valid condition.
        m_xDrawPage->dispose();
        m_xDrawPage->InvalidateSwDoc();
        m_xDrawPage.clear();
    }

    if ( mxXNumberingRules.is() )
    {
        mxXNumberingRules->Invalidate();
        mxXNumberingRules.clear();
    }

    if(mxXFootnotes.is())
    {
        mxXFootnotes->Invalidate();
        mxXFootnotes.clear();
    }

    if(mxXEndnotes.is())
    {
        mxXEndnotes->Invalidate();
        mxXEndnotes.clear();
    }

    if(mxXContentControls.is())
    {
        mxXContentControls->Invalidate();
        mxXContentControls.clear();
    }

    if(mxXDocumentIndexes.is())
    {
        mxXDocumentIndexes->Invalidate();
        mxXDocumentIndexes.clear();
    }

    if(mxXStyleFamilies.is())
    {
        mxXStyleFamilies->Invalidate();
        mxXStyleFamilies.clear();
    }
    if(mxXAutoStyles.is())
    {
        mxXAutoStyles->Invalidate();
        mxXAutoStyles.clear();
    }

    if(mxXBookmarks.is())
    {
        mxXBookmarks->Invalidate();
        mxXBookmarks.clear();
    }

    if(mxXChapterNumbering.is())
    {
        mxXChapterNumbering->Invalidate();
        mxXChapterNumbering.clear();
    }

    if(mxXFootnoteSettings.is())
    {
        mxXFootnoteSettings->Invalidate();
        mxXFootnoteSettings.clear();
    }

    if(mxXEndnoteSettings.is())
    {
        mxXEndnoteSettings->Invalidate();
        mxXEndnoteSettings.clear();
    }

    if(mxXLineNumberingProperties.is())
    {
        mxXLineNumberingProperties->Invalidate();
        mxXLineNumberingProperties.clear();
    }
    if(mxXReferenceMarks.is())
    {
        mxXReferenceMarks->Invalidate();
        mxXReferenceMarks.clear();
    }
    if(mxLinkTargetSupplier.is())
    {
        mxLinkTargetSupplier->Invalidate();
        mxLinkTargetSupplier.clear();
    }
    if(mxXRedlines.is())
    {
        mxXRedlines->Invalidate();
        mxXRedlines.clear();
    }
    if(mxPropertyHelper.is())
    {
        mxPropertyHelper->Invalidate();
        mxPropertyHelper.clear();
    }
}

css::uno::Reference<css::uno::XInterface> SwXTextDocument::create(
    OUString const & rServiceName,
    css::uno::Sequence<css::uno::Any> const * arguments)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const SwServiceType nType = SwXServiceProvider::GetProviderType(rServiceName);
    if (nType != SwServiceType::Invalid)
    {
        return SwXServiceProvider::MakeInstance(nType, GetDocOrThrow());
    }
    if (rServiceName == "com.sun.star.drawing.DashTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Dash);
    }
    if (rServiceName == "com.sun.star.drawing.GradientTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Gradient);
    }
    if (rServiceName == "com.sun.star.drawing.HatchTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Hatch);
    }
    if (rServiceName == "com.sun.star.drawing.BitmapTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Bitmap);
    }
    if (rServiceName == "com.sun.star.drawing.TransparencyGradientTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::TransGradient);
    }
    if (rServiceName == "com.sun.star.drawing.MarkerTable")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Marker);
    }
    if (rServiceName == "com.sun.star.drawing.Defaults")
    {
        return GetPropertyHelper()->GetDrawTable(SwCreateDrawTable::Defaults);
    }
    if (rServiceName == "com.sun.star.document.Settings")
    {
        return Reference<XInterface>(*new SwXDocumentSettings(this));
    }
    if (rServiceName == "com.sun.star.document.ImportEmbeddedObjectResolver")
    {
        return cppu::getXWeak(
            new SvXMLEmbeddedObjectHelper(
                *m_pDocShell, SvXMLEmbeddedObjectHelperMode::Read));
    }
    if (rServiceName == "com.sun.star.text.DocumentSettings")
    {
        return Reference<XInterface>(*new SwXDocumentSettings(this));
    }
    if (rServiceName == "com.sun.star.chart2.data.DataProvider")
    {
        return Reference<XInterface>(
            cppu::getXWeak(
                m_pDocShell->getIDocumentChartDataProviderAccess().
                GetChartDataProvider()));
    }
    if (!rServiceName.startsWith("com.sun.star.")
        || rServiceName.endsWith(".OLE2Shape"))
    {
        // We do not want to insert OLE2 Shapes (e.g.,
        // "com.sun.star.drawing.OLE2Shape", ...) like this (by creating them
        // with the documents factory and adding the shapes to the draw page);
        // for inserting OLE objects the proper way is to use
        // "com.sun.star.text.TextEmbeddedObject":
        throw ServiceNotRegisteredException();
    }
    // The XML import is allowed to create instances of
    // "com.sun.star.drawing.OLE2Shape"; thus, a temporary service name is
    // introduced to make this possible:
    OUString aTmpServiceName(rServiceName);
    if (rServiceName == "com.sun.star.drawing.temporaryForXMLImportOLE2Shape")
    {
        aTmpServiceName = "com.sun.star.drawing.OLE2Shape";
    }
    Reference<XInterface> xTmp(
        arguments == nullptr
        ? SvxFmMSFactory::createInstance(aTmpServiceName)
        : SvxFmMSFactory::createInstanceWithArguments(
            aTmpServiceName, *arguments));
    if (rServiceName == "com.sun.star.drawing.GroupShape"
        || rServiceName == "com.sun.star.drawing.Shape3DSceneObject")
    {
        return *new SwXGroupShape(xTmp, m_pDocShell->GetDoc());
    }
    if (rServiceName.startsWith("com.sun.star.drawing."))
    {
        return *new SwXShape(xTmp, m_pDocShell->GetDoc());
    }
    return xTmp;
}

rtl::Reference< SwXDocumentSettings > SwXTextDocument::createDocumentSettings()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return new SwXDocumentSettings(this);
}

rtl::Reference< SwXTextDefaults > SwXTextDocument::createTextDefaults()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return new SwXTextDefaults(&GetDocOrThrow());
}

rtl::Reference< SwXBookmark > SwXTextDocument::createBookmark()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXBookmark::CreateXBookmark(GetDocOrThrow(), nullptr);
}

rtl::Reference< SwXBookmark > SwXTextDocument::createFieldmark()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXFieldmark::CreateXFieldmark(GetDocOrThrow(), nullptr);
}

rtl::Reference< SwXTextSection > SwXTextDocument::createTextSection()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXTextSection::CreateXTextSection(nullptr, false);
}

rtl::Reference< SwXTextField > SwXTextDocument::createFieldAnnotation()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXTextField::CreateXTextField(&GetDocOrThrow(), nullptr, SwServiceType::FieldTypeAnnotation);
}

rtl::Reference< SwXLineBreak > SwXTextDocument::createLineBreak()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXLineBreak::CreateXLineBreak(nullptr);
}

rtl::Reference< SwXTextFrame > SwXTextDocument::createTextFrame()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXTextFrame::CreateXTextFrame(GetDocOrThrow(), nullptr);
}

rtl::Reference< SwXTextGraphicObject > SwXTextDocument::createTextGraphicObject()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXTextGraphicObject::CreateXTextGraphicObject(GetDocOrThrow(), nullptr);
}

rtl::Reference< SwXStyle > SwXTextDocument::createNumberingStyle()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXStyleFamilies::CreateStyleCharOrParaOrPseudo(SfxStyleFamily::Pseudo, GetDocOrThrow());
}

rtl::Reference< SwXStyle > SwXTextDocument::createCharacterStyle()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXStyleFamilies::CreateStyleCharOrParaOrPseudo(SfxStyleFamily::Char, GetDocOrThrow());
}

rtl::Reference< SwXStyle > SwXTextDocument::createParagraphStyle()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXStyleFamilies::CreateStyleCharOrParaOrPseudo(SfxStyleFamily::Para, GetDocOrThrow());
}

rtl::Reference< SwXPageStyle > SwXTextDocument::createPageStyle()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXStyleFamilies::CreateStylePage(GetDocOrThrow());
}

rtl::Reference< SwXContentControl > SwXTextDocument::createContentControl()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXContentControl::CreateXContentControl(GetDocOrThrow());
}

rtl::Reference< SwXFootnote > SwXTextDocument::createFootnote()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXFootnote::CreateXFootnote(GetDocOrThrow(), nullptr);
}

rtl::Reference< SwXFootnote > SwXTextDocument::createEndnote()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXFootnote::CreateXFootnote(GetDocOrThrow(), nullptr, true);
}

rtl::Reference< SwXTextEmbeddedObject > SwXTextDocument::createTextEmbeddedObject()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return SwXTextEmbeddedObject::CreateXTextEmbeddedObject(GetDocOrThrow(), nullptr);
}

rtl::Reference< SvXMLEmbeddedObjectHelper > SwXTextDocument::createEmbeddedObjectResolver()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();
    return new SvXMLEmbeddedObjectHelper(*m_pDocShell, SvXMLEmbeddedObjectHelperMode::Read);
}

Reference< XInterface >  SwXTextDocument::createInstance(const OUString& rServiceName)
{
    return create(rServiceName, nullptr);
}

Reference< XInterface >  SwXTextDocument::createInstanceWithArguments(
        const OUString& ServiceSpecifier,
        const Sequence< Any >& Arguments)
{
    return create(ServiceSpecifier, &Arguments);
}

Sequence< OUString > SwXTextDocument::getAvailableServiceNames()
{
    static Sequence< OUString > aServices;
    if ( !aServices.hasElements() )
    {
        Sequence< OUString > aRet =  SvxFmMSFactory::getAvailableServiceNames();
        auto i = comphelper::findValue(aRet, "com.sun.star.drawing.OLE2Shape");
        if (i != -1)
        {
            auto nLength = aRet.getLength();
            aRet.getArray()[i] = aRet[nLength - 1];
            aRet.realloc( nLength - 1 );
        }
        Sequence< OUString > aOwn = SwXServiceProvider::GetAllServiceNames();
        aServices = comphelper::concatSequences(aRet, aOwn);
    }

    return aServices;
}

OUString SwXTextDocument::getImplementationName()
{
    return u"SwXTextDocument"_ustr;
    /* // Matching the .component information:
       return dynamic_cast<SwGlobalDocShell*>( pDocShell ) != nullptr
           ? OUString("com.sun.star.comp.Writer.GlobalDocument")
           : dynamic_cast<SwWebDocShell*>( pDocShell ) != nullptr
           ? OUString("com.sun.star.comp.Writer.WebDocument")
           : OUString("com.sun.star.comp.Writer.TextDocument");
    */
}

sal_Bool SwXTextDocument::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextDocument::getSupportedServiceNames()
{
    bool bWebDoc    = (dynamic_cast<SwWebDocShell*>(    m_pDocShell) !=  nullptr );
    bool bGlobalDoc = (dynamic_cast<SwGlobalDocShell*>( m_pDocShell) !=  nullptr );
    bool bTextDoc   = (!bWebDoc && !bGlobalDoc);

    Sequence< OUString > aRet (3);
    OUString* pArray = aRet.getArray();

    pArray[0] = "com.sun.star.document.OfficeDocument";
    pArray[1] = "com.sun.star.text.GenericTextDocument";

    if (bTextDoc)
        pArray[2] = "com.sun.star.text.TextDocument";
    else if (bWebDoc)
        pArray[2] = "com.sun.star.text.WebDocument";
    else if (bGlobalDoc)
        pArray[2] = "com.sun.star.text.GlobalDocument";

    return aRet;
}

Reference< XIndexAccess >  SwXTextDocument::getDocumentIndexes()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    if(!mxXDocumentIndexes.is())
    {
        mxXDocumentIndexes = new SwXDocumentIndexes(m_pDocShell->GetDoc());
    }
    return mxXDocumentIndexes;
}

Reference< XPropertySetInfo >  SwXTextDocument::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = m_pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXTextDocument::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw UnknownPropertyException(rPropertyName);
    if(pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    switch(pEntry->nWID)
    {
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
            throw RuntimeException(
                u"bad WID"_ustr,
                getXWeak());
        case  WID_DOC_WORD_SEPARATOR :
        {
            OUString sDelim;
            aValue >>= sDelim;
            SW_MOD()->GetModuleConfig()->SetWordDelimiter(sDelim);
        }
        break;
        case WID_DOC_CHANGES_RECORD:
        case WID_DOC_CHANGES_SHOW:
        {
            SwDoc& rDoc = GetDocOrThrow();
            sw::DocumentRedlineManager& rRedlineManager = rDoc.GetDocumentRedlineManager();
            bool bSet = *o3tl::doAccess<bool>(aValue);
            RedlineFlags eMode = rRedlineManager.GetRedlineFlags();
            if(WID_DOC_CHANGES_SHOW == pEntry->nWID)
            {
                eMode |= RedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
                if( !bSet )
                    rRedlineManager.SetHideRedlines(true);
            }
            else if(WID_DOC_CHANGES_RECORD == pEntry->nWID)
            {
                eMode = bSet ? eMode|RedlineFlags::On : eMode&~RedlineFlags::On;
            }
            rRedlineManager.SetRedlineFlags(eMode);
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(aValue >>= aNew)
            {
                auto& rRedlineAccess = GetDocOrThrow().getIDocumentRedlineAccess();
                rRedlineAccess.SetRedlinePassword(aNew);
                if(aNew.hasElements())
                {
                    RedlineFlags eMode = rRedlineAccess.GetRedlineFlags();
                    eMode |= RedlineFlags::On;
                    rRedlineAccess.SetRedlineFlags(eMode);
                }
            }
        }
        break;
        case WID_DOC_AUTO_MARK_URL :
        {
            OUString sURL;
            aValue >>= sURL;
            GetDocOrThrow().SetTOIAutoMarkURL(sURL);
        }
        break;
        case WID_DOC_HIDE_TIPS :
            SW_MOD()->GetModuleConfig()->SetHideFieldTips(*o3tl::doAccess<bool>(aValue));
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            auto& rRedlineAccess = GetDocOrThrow().getIDocumentRedlineAccess();
            RedlineFlags eRedMode = rRedlineAccess.GetRedlineFlags();
            eRedMode = eRedMode & (~RedlineFlags::ShowMask);
            sal_Int16 nSet = 0;
            aValue >>= nSet;
            switch(nSet)
            {
                case RedlineDisplayType::NONE: break;
                case RedlineDisplayType::INSERTED: eRedMode |= RedlineFlags::ShowInsert; break;
                case RedlineDisplayType::REMOVED: eRedMode |= RedlineFlags::ShowDelete;  break;
                case RedlineDisplayType::
                        INSERTED_AND_REMOVED: eRedMode |= RedlineFlags::ShowInsert|RedlineFlags::ShowDelete;
                break;
                default: throw IllegalArgumentException();
            }
            rRedlineAccess.SetRedlineFlags(eRedMode);
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            sal_Int16 nYear = 0;
            aValue >>= nYear;
            SfxRequest aRequest ( SID_ATTR_YEAR2000, SfxCallMode::SLOT, GetDocOrThrow().GetAttrPool());
            aRequest.AppendItem(SfxUInt16Item( SID_ATTR_YEAR2000, static_cast < sal_uInt16 > ( nYear ) ) );
            m_pDocShell->Execute ( aRequest );
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            auto& rDrawModelAccess = GetDocOrThrow().getIDocumentDrawModelAccess();
            bool bAuto = *o3tl::doAccess<bool>(aValue);
            // if setting to true, and we don't have an
            // SdrModel, then we are changing the default and
            // must thus create an SdrModel, if we don't have an
            // SdrModel and we are leaving the default at false,
            // we don't need to make an SdrModel and can do nothing
            // #i52858# - method name changed
            SwDrawModel* pDrawDoc
                = bAuto ? rDrawModelAccess.GetOrCreateDrawModel() : rDrawModelAccess.GetDrawModel();

            if ( nullptr != pDrawDoc )
                pDrawDoc->SetAutoControlFocus( bAuto );
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            auto& rDrawModelAccess = GetDocOrThrow().getIDocumentDrawModelAccess();
            bool bMode = *o3tl::doAccess<bool>(aValue);
            // if setting to false, and we don't have an
            // SdrModel, then we are changing the default and
            // must thus create an SdrModel, if we don't have an
            // SdrModel and we are leaving the default at true,
            // we don't need to make an SdrModel and can do
            // nothing
            // #i52858# - method name changed
            SwDrawModel* pDrawDoc
                = bMode ? rDrawModelAccess.GetDrawModel() : rDrawModelAccess.GetOrCreateDrawModel();

            if ( nullptr != pDrawDoc )
                pDrawDoc->SetOpenInDesignMode( bMode );
        }
        break;
        // #i42634# New property to set the bInReading
        // flag at the document, used during binary import
        case WID_DOC_LOCK_UPDATES :
        {
            bool bBool (false);
            if( aValue >>= bBool )
            {
                GetDocOrThrow().SetInReading( bBool );
            }
        }
        break;
        case WID_DOC_WRITERFILTER:
        {
            SwDoc& rDoc = GetDocOrThrow();
            bool bBool = {};
            if (aValue >>= bBool)
            { // HACK: writerfilter has to use API to set this :(
                bool bOld = rDoc.IsInWriterfilterImport();
                rDoc.SetInWriterfilterImport(bBool);
                if (bOld && !bBool)
                {
                    rDoc.getIDocumentFieldsAccess().SetFieldsDirty(false, nullptr, SwNodeOffset(0));
                }
            }
        }
        break;
        case WID_DOC_BUILDID:
            aValue >>= maBuildId;
        break;

        case WID_DOC_DEFAULT_PAGE_MODE:
        {
            bool bDefaultPageMode( false );
            aValue >>= bDefaultPageMode;
            GetDocOrThrow().SetDefaultPageMode( bDefaultPageMode );
        }
        break;
        case WID_DOC_INTEROP_GRAB_BAG:
             setGrabBagItem(aValue);
        break;

        default:
        {
            SwDoc& rDoc = GetDocOrThrow();
            const SfxPoolItem& rItem = rDoc.GetDefault(pEntry->nWID);
            std::unique_ptr<SfxPoolItem> pNewItem(rItem.Clone());
            pNewItem->PutValue(aValue, pEntry->nMemberId);
            rDoc.SetDefault(*pNewItem);
        }
    }
}

Any SwXTextDocument::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    if (rPropertyName == "ODFExport_ListNodes")
    {
        // A hack to avoid writing random list ids to ODF when they are not referred later
        // see XMLTextParagraphExport::DocumentListNodes ctor

        // Sequence of nodes, each of them represented by three-element sequence:
        // [ index, styleIntPtr, list_id ]
        std::vector<css::uno::Sequence<css::uno::Any>> nodes;

        const SwDoc& rDoc = GetDocOrThrow();
        for (const SwNumRule* pNumRule : rDoc.GetNumRuleTable())
        {
            SwNumRule::tTextNodeList textNodes;
            pNumRule->GetTextNodeList(textNodes);
            css::uno::Any styleIntPtr(reinterpret_cast<sal_uInt64>(pNumRule));

            for (const SwTextNode* pTextNode : textNodes)
            {
                css::uno::Any index(pTextNode->GetIndex().get());
                css::uno::Any list_id(pTextNode->GetListId());

                nodes.push_back({ index, styleIntPtr, list_id });
            }
        }
        return css::uno::Any(comphelper::containerToSequence(nodes));
    }

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw UnknownPropertyException(rPropertyName);
    Any aAny;
    switch(pEntry->nWID)
    {
        case WID_DOC_ISTEMPLATEID    :
            aAny <<= m_pDocShell->IsTemplate();
            break;
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
        {
            const SwDocStat& rStat(GetDocOrThrow().getIDocumentStatistics().GetUpdatedDocStat( false, true ));
            sal_Int32 nValue;
            switch(pEntry->nWID)
            {
                case  WID_DOC_CHAR_COUNT     :nValue = rStat.nChar;break;
                case  WID_DOC_PARA_COUNT     :nValue = rStat.nPara;break;
                case  WID_DOC_WORD_COUNT     :nValue = rStat.nWord;break;
            }
            aAny <<= nValue;
        }
        break;
        case  WID_DOC_WORD_SEPARATOR :
        {
            aAny <<= SW_MOD()->GetDocStatWordDelim();
        }
        break;
        case WID_DOC_CHANGES_RECORD:
        case WID_DOC_CHANGES_SHOW:
        {
            const RedlineFlags eMode = GetDocOrThrow().getIDocumentRedlineAccess().GetRedlineFlags();
            bool bSet = false;
            if(WID_DOC_CHANGES_SHOW == pEntry->nWID)
            {
                bSet = IDocumentRedlineAccess::IsShowChanges(eMode);
            }
            else if(WID_DOC_CHANGES_RECORD == pEntry->nWID)
            {
                bSet = bool(eMode & RedlineFlags::On);
            }
            aAny <<= bSet;
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
            aAny <<= GetDocOrThrow().getIDocumentRedlineAccess().GetRedlinePassword();
        break;
        case WID_DOC_AUTO_MARK_URL :
            aAny <<= GetDocOrThrow().GetTOIAutoMarkURL();
        break;
        case WID_DOC_HIDE_TIPS :
            aAny <<= SW_MOD()->GetModuleConfig()->IsHideFieldTips();
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            RedlineFlags eRedMode = GetDocOrThrow().getIDocumentRedlineAccess().GetRedlineFlags();
            eRedMode = eRedMode & RedlineFlags::ShowMask;
            sal_Int16 nRet = RedlineDisplayType::NONE;
            if(RedlineFlags::ShowInsert == eRedMode)
                nRet = RedlineDisplayType::INSERTED;
            else if(RedlineFlags::ShowDelete == eRedMode)
                nRet = RedlineDisplayType::REMOVED;
            else if(RedlineFlags::ShowMask == eRedMode)
                nRet = RedlineDisplayType::INSERTED_AND_REMOVED;
            aAny <<= nRet;
        }
        break;
        case WID_DOC_FORBIDDEN_CHARS:
        {
            GetPropertyHelper();
            Reference<XForbiddenCharacters> xRet = mxPropertyHelper;
            aAny <<= xRet;
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            aAny <<= static_cast < sal_Int16 > (GetDocOrThrow().GetNumberFormatter ()->GetYear2000());
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawModel * pDrawDoc = GetDocOrThrow().getIDocumentDrawModelAccess().GetDrawModel();
            bool bAuto;
            if ( nullptr != pDrawDoc )
                bAuto = pDrawDoc->GetAutoControlFocus();
            else
                bAuto = false;
            aAny <<= bAuto;
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawModel * pDrawDoc = GetDocOrThrow().getIDocumentDrawModelAccess().GetDrawModel();
            bool bMode;
            if ( nullptr != pDrawDoc )
                bMode = pDrawDoc->GetOpenInDesignMode();
            else
                bMode = true;
            aAny <<= bMode;
        }
        break;
        case WID_DOC_BASIC_LIBRARIES:
            aAny <<= m_pDocShell->GetBasicContainer();
        break;
        case WID_DOC_DIALOG_LIBRARIES:
            aAny <<= m_pDocShell->GetDialogContainer();
        break;
        case WID_DOC_VBA_DOCOBJ:
        {
            /* #i111553# This property provides the name of the constant that
               will be used to store this model in the global Basic manager.
               That constant will be equivalent to 'ThisComponent' but for
               each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
               constant can co-exist, as required by VBA. */
            aAny <<= u"ThisWordDoc"_ustr;
        }
        break;
        case WID_DOC_RUNTIME_UID:
            aAny <<= getRuntimeUID();
        break;
        case WID_DOC_LOCK_UPDATES :
            aAny <<= GetDocOrThrow().IsInReading();
        break;
        case WID_DOC_BUILDID:
            aAny <<= maBuildId;
        break;
        case WID_DOC_HAS_VALID_SIGNATURES:
            aAny <<= hasValidSignatures();
        break;
        case WID_DOC_INTEROP_GRAB_BAG:
             getGrabBagItem(aAny);
        break;
        case WID_DOC_ALLOW_LINK_UPDATE:
        {
            comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = m_pDocShell->getEmbeddedObjectContainer();
            aAny <<= rEmbeddedObjectContainer.getUserAllowsLinkUpdate();
        }
        break;

        default:
        {
            const SfxPoolItem& rItem = GetDocOrThrow().GetDefault(pEntry->nWID);
            rItem.QueryValue(aAny, pEntry->nMemberId);
        }
    }
    return aAny;
}

void SwXTextDocument::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const Reference< XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const Reference< XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const Reference< XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::removeVetoableChangeListener(const OUString& /*PropertyName*/,
                        const Reference< XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

Reference< XNameAccess >  SwXTextDocument::getLinks()
{
    if(!mxLinkTargetSupplier.is())
    {
        mxLinkTargetSupplier = new SwXLinkTargetSupplier(*this);
    }
    return mxLinkTargetSupplier;
}

Reference< XEnumerationAccess > SwXTextDocument::getRedlines(  )
{
    if(!mxXRedlines.is())
    {
        mxXRedlines = new SwXRedlines(m_pDocShell->GetDoc());
    }
    return mxXRedlines;
}

void SwXTextDocument::NotifyRefreshListeners()
{
    // why does SwBaseShell not just call refresh? maybe because it's rSh is
    // (sometimes) a different shell than GetWrtShell()?
    lang::EventObject const ev(getXWeak());
    std::unique_lock aGuard(m_pImpl->m_Mutex);
    m_pImpl->m_RefreshListeners.notifyEach(aGuard,
            & util::XRefreshListener::refreshed, ev);
}

void SwXTextDocument::refresh()
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    SwViewShell *pViewShell = m_pDocShell->GetWrtShell();
    NotifyRefreshListeners();
    if(pViewShell)
        pViewShell->Reformat();
}

void SAL_CALL SwXTextDocument::addRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
{
    if (xListener)
    {
        std::unique_lock aGuard(m_pImpl->m_Mutex);
        m_pImpl->m_RefreshListeners.addInterface(aGuard, xListener);
    }
}

void SAL_CALL SwXTextDocument::removeRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
{
    if (xListener)
    {
        std::unique_lock aGuard(m_pImpl->m_Mutex);
        m_pImpl->m_RefreshListeners.removeInterface(aGuard, xListener);
    }
}

void SwXTextDocument::updateLinks(  )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    SwDoc& rDoc = GetDocOrThrow();
    sfx2::LinkManager& rLnkMan = rDoc.getIDocumentLinksAdministration().GetLinkManager();
    if( !rLnkMan.GetLinks().empty() )
    {
        UnoActionContext aAction(&rDoc);
        rLnkMan.UpdateAllLinks( false, true, nullptr );
    }
}

//XPropertyState
PropertyState SAL_CALL SwXTextDocument::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException(rPropertyName);
    return PropertyState_DIRECT_VALUE;
}

Sequence< PropertyState > SAL_CALL SwXTextDocument::getPropertyStates( const Sequence< OUString >& rPropertyNames )
{
    const sal_Int32 nCount = rPropertyNames.getLength();
    Sequence < PropertyState > aRet ( nCount );

    std::transform(rPropertyNames.begin(), rPropertyNames.end(), aRet.getArray(),
        [this](const OUString& rName) -> PropertyState { return getPropertyState(rName); });

    return aRet;
}

void SAL_CALL SwXTextDocument::setPropertyToDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException(rPropertyName);
    switch(pEntry->nWID)
    {
        case 0:default:break;
    }
}

Any SAL_CALL SwXTextDocument::getPropertyDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException(rPropertyName);
    Any aAny;
    switch(pEntry->nWID)
    {
        case 0:default:break;
    }
    return aAny;
}

static VclPtr< OutputDevice > lcl_GetOutputDevice( const SwPrintUIOptions &rPrintUIOptions )
{
    VclPtr< OutputDevice > pOut;

    uno::Any aAny( rPrintUIOptions.getValue( u"RenderDevice"_ustr ));
    uno::Reference< awt::XDevice >  xRenderDevice;
    aAny >>= xRenderDevice;
    if (xRenderDevice.is())
    {
        VCLXDevice* pDevice = dynamic_cast<VCLXDevice*>( xRenderDevice.get() );
        pOut = pDevice ? pDevice->GetOutputDevice() : VclPtr< OutputDevice >();
    }

    return pOut;
}

static bool lcl_SeqHasProperty(
    const uno::Sequence< beans::PropertyValue >& rOptions,
    const char *pPropName )
{
    return std::any_of(rOptions.begin(), rOptions.end(),
        [&pPropName](const beans::PropertyValue& rProp) {
            return rProp.Name.equalsAscii( pPropName ); });
}

static bool lcl_GetBoolProperty(
    const uno::Sequence< beans::PropertyValue >& rOptions,
    const char *pPropName )
{
    bool bRes = false;
    auto pOption = std::find_if(rOptions.begin(), rOptions.end(),
        [&pPropName](const beans::PropertyValue& rProp) {
            return rProp.Name.equalsAscii( pPropName ); });
    if (pOption != rOptions.end())
        pOption->Value >>= bRes;
    return bRes;
}

SfxViewShell * SwXTextDocument::GetRenderView(
    bool &rbIsSwSrcView,
    const uno::Sequence< beans::PropertyValue >& rOptions,
    bool bIsPDFExport )
{
    // get view shell to use
    SfxViewShell *pView = nullptr;
    if (bIsPDFExport)
        pView = GuessViewShell( rbIsSwSrcView );
    else
    {
        uno::Any aTmp;
        auto pOption = std::find_if(rOptions.begin(), rOptions.end(),
            [](const beans::PropertyValue& rProp) { return rProp.Name == "View"; });
        if (pOption != rOptions.end())
            aTmp = pOption->Value;

        uno::Reference< frame::XController > xController;
        if (aTmp >>= xController)
        {
            OSL_ENSURE( xController.is(), "controller is empty!" );
            pView = GuessViewShell( rbIsSwSrcView, xController );
        }
    }
    return pView;
}

/*
 *  GetRenderDoc:
 *  returns the document to be rendered, usually this will be the 'regular'
 *  document but in case of PDF export of (multi-)selection it will
 *  be a temporary document that gets created if not already done.
 *  The rpView variable will be set (if not already done) to the used
 *  SfxViewShell.
*/
SwDoc * SwXTextDocument::GetRenderDoc(
    SfxViewShell *&rpView,
    const uno::Any& rSelection,
    bool bIsPDFExport )
{
    SwDoc *pDoc = nullptr;

    uno::Reference< frame::XModel > xModel;
    rSelection >>= xModel;
    if (xModel == m_pDocShell->GetModel())
        pDoc = m_pDocShell->GetDoc();
    else
    {
        OSL_ENSURE( !xModel.is(), "unexpected model found" );

        if (rSelection.hasValue())     // is anything selected ?
        {
            // this part should only be called when a temporary document needs to be created,
            // for example for PDF export or printing of (multi-)selection only.

            if (!rpView)
            {
                bool bIsSwSrcView = false;
                // aside from maybe PDF export the view should always have been provided!
                OSL_ENSURE( bIsPDFExport, "view is missing, guessing one..." );

                rpView = GuessViewShell( bIsSwSrcView );
            }
            OSL_ENSURE( rpView, "SwViewShell missing" );
            // the view shell should be SwView for documents PDF export.
            // for the page preview no selection should be possible
            // (the export dialog does not allow for this option)
            if (auto pSwView = dynamic_cast<SwView *>( rpView ))
            {
                if (!m_pRenderData)
                {
                    OSL_FAIL("GetRenderDoc: no renderdata");
                    return nullptr;
                }
                SfxObjectShellLock xDocSh(m_pRenderData->GetTempDocShell());
                if (!xDocSh.Is())
                {
                    xDocSh = pSwView->CreateTmpSelectionDoc();
                    m_pRenderData->SetTempDocShell(xDocSh);
                }
                if (xDocSh.Is())
                {
                    pDoc = static_cast<SwDocShell*>(&xDocSh)->GetDoc();
                    rpView = pDoc->GetDocShell()->GetView();
                }
            }
            else
            {
                OSL_FAIL("unexpected SwViewShell" );
            }
        }
    }
    return pDoc;
}

static void lcl_SavePrintUIOptionsToDocumentPrintData(
    SwDoc &rDoc,
    const SwPrintUIOptions &rPrintUIOptions,
    bool bIsPDFEXport )
{
    SwPrintData aDocPrintData( rDoc.getIDocumentDeviceAccess().getPrintData() );

    aDocPrintData.SetPrintGraphic( rPrintUIOptions.IsPrintGraphics() );
    aDocPrintData.SetPrintControl( rPrintUIOptions.IsPrintFormControls() );
    aDocPrintData.SetPrintLeftPage( rPrintUIOptions.IsPrintLeftPages() );
    aDocPrintData.SetPrintRightPage( rPrintUIOptions.IsPrintRightPages() );
    aDocPrintData.SetPaperFromSetup( rPrintUIOptions.IsPaperFromSetup() );
    aDocPrintData.SetPrintEmptyPages( rPrintUIOptions.IsPrintEmptyPages( bIsPDFEXport ) );
    aDocPrintData.SetPrintPostIts( rPrintUIOptions.GetPrintPostItsType() );
    aDocPrintData.SetPrintProspect( rPrintUIOptions.IsPrintProspect() );
    aDocPrintData.SetPrintProspect_RTL( rPrintUIOptions.IsPrintProspectRTL() );
    aDocPrintData.SetPrintPageBackground( rPrintUIOptions.IsPrintPageBackground() );
    aDocPrintData.SetPrintBlackFont( rPrintUIOptions.IsPrintWithBlackTextColor() );
    // arDocPrintData.SetFaxName( s ); n/a in File/Print dialog
    aDocPrintData.SetPrintHiddenText( rPrintUIOptions.IsPrintHiddenText() );
    aDocPrintData.SetPrintTextPlaceholder( rPrintUIOptions.IsPrintTextPlaceholders() );

    rDoc.getIDocumentDeviceAccess().setPrintData( aDocPrintData );
}

sal_Int32 SAL_CALL SwXTextDocument::getRendererCount(
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const bool bIsPDFExport = !lcl_SeqHasProperty( rxOptions, "IsPrinter" );
    bool bIsSwSrcView = false;
    SfxViewShell *pView = GetRenderView( bIsSwSrcView, rxOptions, bIsPDFExport );

    if (!bIsSwSrcView && !m_pRenderData)
        m_pRenderData.reset(new SwRenderData);
    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = lcl_GetPrintUIOptions( m_pDocShell, pView );
    bool bFormat = m_pPrintUIOptions->processPropertiesAndCheckFormat( rxOptions );

    SwDoc *pDoc = GetRenderDoc( pView, rSelection, bIsPDFExport );
    OSL_ENSURE( pDoc && pView, "doc or view shell missing!" );
    if (!pDoc || !pView)
        return 0;

    // save current UI options from the print dialog for the next call to that dialog
    lcl_SavePrintUIOptionsToDocumentPrintData( *pDoc, *m_pPrintUIOptions, bIsPDFExport );

    sal_Int32 nRet = 0;
    if (bIsSwSrcView)
    {
        SwSrcView& rSwSrcView = dynamic_cast<SwSrcView&>(*pView);
        VclPtr< OutputDevice> pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
        nRet = rSwSrcView.PrintSource( pOutDev, 1 /* dummy */, true /* get page count only */ );
    }
    else
    {
        SwDocShell *pRenderDocShell = pDoc->GetDocShell();

        // TODO/mba: we really need a generic way to get the SwViewShell!
        SwViewShell* pViewShell = nullptr;
        SwView* pSwView = dynamic_cast<SwView*>( pView );
        if ( pSwView )
        {
            pViewShell = pSwView->GetWrtShellPtr();
        }
        else
        {
            if ( bIsPDFExport && bFormat )
            {
                //create a hidden view to be able to export as PDF also in print preview
                //pView and pSwView are not changed intentionally!
                m_pHiddenViewFrame = SfxViewFrame::LoadHiddenDocument( *pRenderDocShell, SFX_INTERFACE_SFXDOCSH );
                pViewShell = static_cast<SwView*>(m_pHiddenViewFrame->GetViewShell())->GetWrtShellPtr();
            }
            else
                pViewShell = static_cast<SwPagePreview*>(pView)->GetViewShell();
        }

        if (!pViewShell || !pViewShell->GetLayout())
            return 0;

        if (bFormat)
        {
            // #i38289
            if( pViewShell->GetViewOptions()->getBrowseMode() ||
                pViewShell->GetViewOptions()->IsWhitespaceHidden() )
            {
                SwViewOption aOpt( *pViewShell->GetViewOptions() );
                aOpt.setBrowseMode( false );
                aOpt.SetHideWhitespaceMode( false );
                pViewShell->ApplyViewOptions( aOpt );
                if (pSwView)
                {
                    pSwView->RecheckBrowseMode();
                }
            }

            // reformatting the document for printing will show the changes in the view
            // which is likely to produce many unwanted and not nice to view actions.
            // We don't want that! Thus we disable updating of the view.
            pViewShell->StartAction();

            if (pSwView)
            {
                if (m_pRenderData && m_pRenderData->NeedNewViewOptionAdjust( *pViewShell ) )
                    m_pRenderData->ViewOptionAdjustStop();
                if (m_pRenderData && !m_pRenderData->IsViewOptionAdjust())
                {
                    m_pRenderData->ViewOptionAdjustStart(
                        *pViewShell, *pViewShell->GetViewOptions() );
                }
            }

            m_pRenderData->MakeSwPrtOptions( pRenderDocShell,
                    m_pPrintUIOptions.get(), bIsPDFExport );

            if (pSwView)
            {
                // PDF export should not make use of the SwPrtOptions
                const SwPrintData *pPrtOptions = bIsPDFExport
                    ? nullptr : m_pRenderData->GetSwPrtOptions();
                bool setShowPlaceHoldersInPDF = false;
                if(bIsPDFExport)
                    setShowPlaceHoldersInPDF = lcl_GetBoolProperty( rxOptions, "ExportPlaceholders" );
                m_pRenderData->ViewOptionAdjust( pPrtOptions, setShowPlaceHoldersInPDF );
            }

            // since printing now also use the API for PDF export this option
            // should be set for printing as well ...
            pViewShell->SetPDFExportOption( true );

            // there is some redundancy between those two function calls, but right now
            // there is no time to sort this out.
            //TODO: check what exactly needs to be done and make just one function for that
            pViewShell->CalcLayout();

            // #122919# Force field update before PDF export, but after layout init (tdf#121962)
            bool bStateChanged = false;
            // check configuration: shall update of printing information in DocInfo set the document to "modified"?
            if (pRenderDocShell->IsEnableSetModified() && !officecfg::Office::Common::Print::PrintingModifiesDocument::get())
            {
                pRenderDocShell->EnableSetModified( false );
                bStateChanged = true;
            }
            pViewShell->SwViewShell::UpdateFields(true);
            if( bStateChanged )
                pRenderDocShell->EnableSetModified();

            pViewShell->CalcPagesForPrint( pViewShell->GetPageCount() );

            pViewShell->SetPDFExportOption( false );

            // enable view again
            pViewShell->EndAction();
        }

        const sal_Int32 nPageCount = pViewShell->GetPageCount();

        // get number of pages to be rendered

        const bool bPrintProspect = m_pPrintUIOptions->getBoolValue( "PrintProspect" );
        if (bPrintProspect)
        {
            SwDoc::CalculatePagePairsForProspectPrinting( *pViewShell->GetLayout(), *m_pRenderData, *m_pPrintUIOptions, nPageCount );
            nRet = m_pRenderData->GetPagePairsForProspectPrinting().size();
        }
        else
        {
            const SwPostItMode nPostItMode = static_cast<SwPostItMode>( m_pPrintUIOptions->getIntValue( "PrintAnnotationMode", 0 ) );
            if (nPostItMode != SwPostItMode::NONE)
            {
                VclPtr< OutputDevice > pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
                m_pRenderData->CreatePostItData(*pDoc, pViewShell->GetViewOptions(), pOutDev);
            }

            // get set of valid document pages (according to the current settings)
            // and their start frames
            SwDoc::CalculatePagesForPrinting( *pViewShell->GetLayout(), *m_pRenderData, *m_pPrintUIOptions, bIsPDFExport, nPageCount );

            if (nPostItMode != SwPostItMode::NONE)
            {
                SwDoc::UpdatePagesForPrintingWithPostItData( *m_pRenderData,
                        *m_pPrintUIOptions, nPageCount );
            }

            nRet = m_pRenderData->GetPagesToPrint().size();
        }
    }
    OSL_ENSURE( nRet >= 0, "negative number of pages???" );
    // tdf#144989 the layout is complete now - prevent DoIdleJobs() from
    // messing it up, particulary SwDocUpdateField::MakeFieldList_() unhiding
    // sections
    pDoc->getIDocumentTimerAccess().BlockIdling();

    return nRet;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SwXTextDocument::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    const bool bIsPDFExport = !lcl_SeqHasProperty( rxOptions, "IsPrinter" );
    bool bIsSwSrcView = false;
    SfxViewShell *pView = GetRenderView( bIsSwSrcView, rxOptions, bIsPDFExport );

    // m_pRenderData should NOT be created here!
    // That should only be done in getRendererCount. If this function is called before
    // getRendererCount was called then the caller will probably just retrieve the extra UI options
    // and is not interested in getting valid information about the other data that would
    // otherwise be provided here!
//    if( ! m_pRenderData )
//        m_pRenderData = new SwRenderData;
    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = lcl_GetPrintUIOptions( m_pDocShell, pView );
    m_pPrintUIOptions->processProperties( rxOptions );
    const bool bPrintProspect    = m_pPrintUIOptions->getBoolValue( "PrintProspect" );
    const bool bIsSkipEmptyPages = !m_pPrintUIOptions->IsPrintEmptyPages( bIsPDFExport );
    const bool bPrintPaperFromSetup = m_pPrintUIOptions->getBoolValue( "PrintPaperFromSetup" );

    SwDoc *pDoc = GetRenderDoc( pView, rSelection, bIsPDFExport );
    OSL_ENSURE( pDoc && pView, "doc or view shell missing!" );
    if (!pDoc || !pView)
        return uno::Sequence< beans::PropertyValue >();

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (0 > nRenderer)
        throw IllegalArgumentException();

    sal_Int32 nMaxRenderer = 0;
    if (!bIsSwSrcView && m_pRenderData)
    {
        OSL_ENSURE( m_pRenderData, "m_pRenderData missing!!" );
        nMaxRenderer = bPrintProspect?
            m_pRenderData->GetPagePairsForProspectPrinting().size() - 1 :
            m_pRenderData->GetPagesToPrint().size() - 1;
    }
    // since SwSrcView::PrintSource is a poor implementation to get the number of pages to print
    // we obmit checking of the upper bound in this case.
    if (!bIsSwSrcView && m_pRenderData && nRenderer > nMaxRenderer)
        return uno::Sequence< beans::PropertyValue >();

    uno::Sequence< beans::PropertyValue > aRenderer;
    if (m_pRenderData)
    {
        // #i114210#
        // determine the correct page number from the renderer index
        // #i114875
        // consider brochure print
        const sal_Int32 nPage = bPrintProspect
                             ? nRenderer + 1
                             : m_pRenderData->GetPagesToPrint()[ nRenderer ];

        // get paper tray to use ...
        sal_Int32 nPrinterPaperTray = -1;
        if (! bPrintPaperFromSetup)
        {
            // ... from individual page style (see the page tab in Format/Page dialog)
            const std::map< sal_Int32, sal_Int32 > &rPaperTrays = m_pRenderData->GetPrinterPaperTrays();
            std::map< sal_Int32, sal_Int32 >::const_iterator aIt( rPaperTrays.find( nPage ) );
            if (aIt != rPaperTrays.end())
                nPrinterPaperTray = aIt->second;
        }

        awt::Size aPageSize;
        awt::Point aPagePos;
        awt::Size aPreferredPageSize;
        Size aTmpSize;
        if (bIsSwSrcView || bPrintProspect)
        {
            // for printing of HTML source code and prospect printing we should use
            // the printers paper size since
            // a) HTML source view has no page size
            // b) prospect printing has a different page size from the documents page
            //    since two document pages will get rendered on one printer page

            // since PageIncludesNonprintableArea will be set to true we can return the
            // printers paper size here.
            // Sometimes 'getRenderer' is only called to get "ExtraPrintUIOptions", in this
            // case we won't get an OutputDevice here, but then the caller also has no need
            // for the correct PageSize right now...
            VclPtr< Printer > pPrinter = dynamic_cast< Printer * >(lcl_GetOutputDevice( *m_pPrintUIOptions ).get());
            if (pPrinter)
            {
                // HTML source view and prospect adapt to the printer's paper size
                aTmpSize = pPrinter->GetPaperSize();
                aTmpSize = OutputDevice::LogicToLogic( aTmpSize,
                            pPrinter->GetMapMode(), MapMode( MapUnit::Map100thMM ));
                aPageSize = awt::Size( aTmpSize.Width(), aTmpSize.Height() );
                #if 0
                // #i115048# it seems users didn't like getting double the formatted page size
                // revert to "old" behavior scaling to the current paper size of the printer
                if (bPrintProspect)
                {
                    // we just state what output size we would need
                    // which may cause vcl to set that page size on the printer
                    // (if available and not overridden by the user)
                    aTmpSize = pVwSh->GetPageSize( nPage, bIsSkipEmptyPages );
                    aPreferredPageSize = awt::Size ( convertTwipToMm100( 2 * aTmpSize.Width() ),
                                                     convertTwipToMm100( aTmpSize.Height() ));
                }
                #else
                if( bPrintProspect )
                {
                    // just switch to an appropriate portrait/landscape format
                    // FIXME: brochure printing with landscape pages puts the
                    // pages next to each other, so landscape is currently always
                    // the better choice
                    if( aPageSize.Width < aPageSize.Height )
                    {
                        aPreferredPageSize.Width = aPageSize.Height;
                        aPreferredPageSize.Height = aPageSize.Width;
                    }
                }
                #endif
            }
        }
        else
        {
            // TODO/mba: we really need a generic way to get the SwViewShell!
            SwViewShell* pVwSh = nullptr;
            SwView* pSwView = dynamic_cast<SwView*>( pView );
            if ( pSwView )
                pVwSh = pSwView->GetWrtShellPtr();
            else
                pVwSh = static_cast<SwPagePreview*>(pView)->GetViewShell();

            if (pVwSh)
            {
                aTmpSize = pVwSh->GetPageSize( nPage, bIsSkipEmptyPages );
                aPageSize = awt::Size ( convertTwipToMm100( aTmpSize.Width() ),
                                        convertTwipToMm100( aTmpSize.Height() ));
                Point aPoint = pVwSh->GetPagePos(nPage);
                aPagePos = awt::Point(convertTwipToMm100(aPoint.X()), convertTwipToMm100(aPoint.Y()));
            }
        }

        sal_Int32 nLen = 3;
        aRenderer = { comphelper::makePropertyValue(u"PageSize"_ustr, aPageSize),
                      comphelper::makePropertyValue(u"PageIncludesNonprintableArea"_ustr, true),
                      comphelper::makePropertyValue(u"PagePos"_ustr, aPagePos) };
        if (aPreferredPageSize.Width && aPreferredPageSize.Height)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            auto pRenderer = aRenderer.getArray();
            pRenderer[ nLen - 1 ].Name  = "PreferredPageSize";
            pRenderer[ nLen - 1 ].Value <<= aPreferredPageSize;
        }
        if (nPrinterPaperTray >= 0)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            auto pRenderer = aRenderer.getArray();
            pRenderer[ nLen - 1 ].Name  = "PrinterPaperTray";
            pRenderer[ nLen - 1 ].Value <<= nPrinterPaperTray;
        }
    }

    // #i117783#
    if ( m_bApplyPagePrintSettingsFromXPagePrintable )
    {
        SwDoc& rDoc = GetDocOrThrow();
        const SwPagePreviewPrtData* pPagePrintSettings = rDoc.GetPreviewPrtData();
        if ( pPagePrintSettings &&
             ( pPagePrintSettings->GetRow() > 1 ||
               pPagePrintSettings->GetCol() > 1 ) )
        {
            // extend render data by page print settings attributes
            sal_Int32 nLen = aRenderer.getLength();
            const sal_Int32 nRenderDataIdxStart = nLen;
            nLen += 9;
            aRenderer.realloc( nLen );
            auto pRenderer = aRenderer.getArray();
            // put page print settings attribute into render data
            const sal_Int32 nRow = pPagePrintSettings->GetRow();
            pRenderer[ nRenderDataIdxStart + 0 ].Name  = "NUpRows";
            pRenderer[ nRenderDataIdxStart + 0 ].Value <<= std::max<sal_Int32>( nRow, 1);
            const sal_Int32 nCol = pPagePrintSettings->GetCol();
            pRenderer[ nRenderDataIdxStart + 1 ].Name  = "NUpColumns";
            pRenderer[ nRenderDataIdxStart + 1 ].Value <<= std::max<sal_Int32>( nCol, 1);
            pRenderer[ nRenderDataIdxStart + 2 ].Name  = "NUpPageMarginLeft";
            pRenderer[ nRenderDataIdxStart + 2 ].Value <<= pPagePrintSettings->GetLeftSpace();
            pRenderer[ nRenderDataIdxStart + 3 ].Name  = "NUpPageMarginRight";
            pRenderer[ nRenderDataIdxStart + 3 ].Value <<= pPagePrintSettings->GetRightSpace();
            pRenderer[ nRenderDataIdxStart + 4 ].Name  = "NUpPageMarginTop";
            pRenderer[ nRenderDataIdxStart + 4 ].Value <<= pPagePrintSettings->GetTopSpace();
            pRenderer[ nRenderDataIdxStart + 5 ].Name  = "NUpPageMarginBottom";
            pRenderer[ nRenderDataIdxStart + 5 ].Value <<= pPagePrintSettings->GetBottomSpace();
            pRenderer[ nRenderDataIdxStart + 6 ].Name  = "NUpHorizontalSpacing";
            pRenderer[ nRenderDataIdxStart + 6 ].Value <<= pPagePrintSettings->GetHorzSpace();
            pRenderer[ nRenderDataIdxStart + 7 ].Name  = "NUpVerticalSpacing";
            pRenderer[ nRenderDataIdxStart + 7 ].Value <<= pPagePrintSettings->GetVertSpace();
            if (Printer* pPrinter = rDoc.getIDocumentDeviceAccess().getPrinter(false))
            {
                awt::Size aNewPageSize;
                const Size aPageSize = pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
                aNewPageSize = awt::Size( aPageSize.Width(), aPageSize.Height() );
                if ( ( pPagePrintSettings->GetLandscape() &&
                       aPageSize.Width() < aPageSize.Height() ) ||
                     ( !pPagePrintSettings->GetLandscape() &&
                       aPageSize.Width() > aPageSize.Height() ) )
                {
                    aNewPageSize = awt::Size( aPageSize.Height(), aPageSize.Width() );
                }
                pRenderer[ nRenderDataIdxStart + 8 ].Name  = "NUpPaperSize";
                pRenderer[ nRenderDataIdxStart + 8 ].Value <<= aNewPageSize;
            }
        }

        m_bApplyPagePrintSettingsFromXPagePrintable = false;
    }

    m_pPrintUIOptions->appendPrintUIOptions( aRenderer );

    return aRenderer;
}

SfxViewShell * SwXTextDocument::GuessViewShell(
    /* out */ bool &rbIsSwSrcView,
    const uno::Reference< css::frame::XController >& rController )
{
    // #130810# SfxViewShell::Current() / SfxViewShell::GetObjectShell()
    // must not be used (see comment from MBA)

    SfxViewShell    *pView = nullptr;
    SwView          *pSwView = nullptr;
    SwPagePreview   *pSwPagePreview = nullptr;
    SwSrcView       *pSwSrcView = nullptr;
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( m_pDocShell, false );

    // look for the view shell with the same controller in use,
    // otherwise look for a suitable view, preferably a SwView,
    // if that one is not found use a SwPagePreview if found.
    while (pFrame)
    {
        pView = pFrame->GetViewShell();
        pSwView = dynamic_cast< SwView * >(pView);
        pSwSrcView = dynamic_cast< SwSrcView * >(pView);
        if (!pSwPagePreview)
            pSwPagePreview = dynamic_cast< SwPagePreview * >(pView);
        if (rController.is())
        {
            if (pView && pView->GetController() == rController)
                break;
        }
        else if (pSwView || pSwSrcView)
            break;
        pFrame = SfxViewFrame::GetNext( *pFrame, m_pDocShell,  false );
    }

    OSL_ENSURE( pSwView || pSwPagePreview || pSwSrcView, "failed to get view shell" );
    if (pView)
        rbIsSwSrcView = pSwSrcView != nullptr;
    return pView;
}

void SAL_CALL SwXTextDocument::render(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (0 > nRenderer)
        throw IllegalArgumentException();

    // tdf#135244: prevent jumping to cursor at any temporary modification
    auto aLock = m_pDocShell->LockAllViews();

    const bool bHasPDFExtOutDevData = lcl_SeqHasProperty( rxOptions, "HasPDFExtOutDevData" );
    const bool bIsPDFExport = !lcl_SeqHasProperty( rxOptions, "IsPrinter" ) || bHasPDFExtOutDevData;
    bool bIsSwSrcView = false;
    SfxViewShell *pView = GetRenderView( bIsSwSrcView, rxOptions, bIsPDFExport );

    OSL_ENSURE( m_pRenderData, "data should have been created already in getRendererCount..." );
    OSL_ENSURE( m_pPrintUIOptions, "data should have been created already in getRendererCount..." );
    if (!bIsSwSrcView && !m_pRenderData)
        m_pRenderData.reset(new SwRenderData);
    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = lcl_GetPrintUIOptions( m_pDocShell, pView );
    m_pPrintUIOptions->processProperties( rxOptions );
    const bool bPrintProspect   = m_pPrintUIOptions->getBoolValue( "PrintProspect" );
    const bool bLastPage        = m_pPrintUIOptions->getBoolValue( "IsLastPage" );

    SwDoc *pDoc = GetRenderDoc( pView, rSelection, bIsPDFExport );
    OSL_ENSURE( pDoc && pView, "doc or view shell missing!" );
    if (pDoc && pView)
    {
        sal_Int32 nMaxRenderer = 0;
        if (!bIsSwSrcView)
        {
            OSL_ENSURE( m_pRenderData, "m_pRenderData missing!!" );
            nMaxRenderer = bPrintProspect?
                m_pRenderData->GetPagePairsForProspectPrinting().size() - 1 :
                m_pRenderData->GetPagesToPrint().size() - 1;
        }
        // since SwSrcView::PrintSource is a poor implementation to get the number of pages to print
        // we obmit checking of the upper bound in this case.
        if (bIsSwSrcView || nRenderer <= nMaxRenderer)
        {
            if (bIsSwSrcView)
            {
                SwSrcView& rSwSrcView = dynamic_cast<SwSrcView&>(*pView);
                VclPtr< OutputDevice > pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
                rSwSrcView.PrintSource(pOutDev, nRenderer + 1, false);
            }
            else
            {
                // the view shell should be SwView for documents PDF export
                // or SwPagePreview for PDF export of the page preview
                SwViewShell* pVwSh = nullptr;
                // TODO/mba: we really need a generic way to get the SwViewShell!
                const SwView* pSwView = dynamic_cast<const SwView*>(pView);
                if (pSwView)
                    pVwSh = pSwView->GetWrtShellPtr();
                else
                    pVwSh = static_cast<SwPagePreview*>(pView)->GetViewShell();

                // get output device to use
                VclPtr< OutputDevice > pOut = lcl_GetOutputDevice( *m_pPrintUIOptions );

                if(pVwSh && pOut && m_pRenderData->HasSwPrtOptions())
                {
                    const OUString aPageRange  = m_pPrintUIOptions->getStringValue( "PageRange" );
                    const bool bFirstPage           = m_pPrintUIOptions->getBoolValue( "IsFirstPage" );
                    bool bIsSkipEmptyPages          = !m_pPrintUIOptions->IsPrintEmptyPages( bIsPDFExport );

                    OSL_ENSURE((pSwView && m_pRenderData->IsViewOptionAdjust())
                            || (!pSwView && !m_pRenderData->IsViewOptionAdjust()),
                            "SwView / SwViewOptionAdjust_Impl availability mismatch" );

                    // since printing now also use the API for PDF export this option
                    // should be set for printing as well ...
                    pVwSh->SetPDFExportOption( true );

                    // #i12836# enhanced pdf export

                    // First, we have to export hyperlinks, notes, and outline to pdf.
                    // During this process, additional information required for tagging
                    // the pdf file are collected, which are evaluated during painting.

                    SwWrtShell* pWrtShell = pSwView ? pSwView->GetWrtShellPtr() : nullptr;

                    SwPrintData rSwPrtOptions = *m_pRenderData->GetSwPrtOptions();
                    if (bIsPDFExport)
                    {
                        rSwPrtOptions.SetPrintPostIts(
                                lcl_GetBoolProperty(rxOptions, "ExportNotesInMargin")
                                    ? SwPostItMode::InMargins
                                    : SwPostItMode::NONE);
                    }

                    if (bIsPDFExport && (bFirstPage || bHasPDFExtOutDevData) && pWrtShell)
                    {
                        SwEnhancedPDFExportHelper aHelper( *pWrtShell, *pOut, aPageRange, bIsSkipEmptyPages, false, rSwPrtOptions );
                    }

                    if (bPrintProspect)
                        pVwSh->PrintProspect( pOut, rSwPrtOptions, nRenderer );
                    else    // normal printing and PDF export
                        pVwSh->PrintOrPDFExport( pOut, rSwPrtOptions, nRenderer, bIsPDFExport );

                    // #i35176#

                    // After printing the last page, we take care for the links coming
                    // from the EditEngine. The links are generated during the painting
                    // process, but the destinations are still missing.

                    if (bIsPDFExport && bLastPage && pWrtShell)
                    {
                        SwEnhancedPDFExportHelper aHelper( *pWrtShell, *pOut, aPageRange, bIsSkipEmptyPages, true, rSwPrtOptions );
                    }

                    pVwSh->SetPDFExportOption( false );

                    // last page to be rendered? (not necessarily the last page of the document)
                    // -> do clean-up of data
                    if (bLastPage)
                    {
                        // #i96167# haggai: delete ViewOptionsAdjust here because it makes use
                        // of the shell, which might get destroyed in lcl_DisposeView!
                        if (m_pRenderData->IsViewOptionAdjust())
                            m_pRenderData->ViewOptionAdjustStop();

                        if (m_pRenderData->HasPostItData())
                            m_pRenderData->DeletePostItData();
                        if (m_pHiddenViewFrame)
                        {
                            lcl_DisposeView( m_pHiddenViewFrame, m_pDocShell );
                            m_pHiddenViewFrame = nullptr;

                            // prevent crash described in #i108805
                            SwDocShell *pRenderDocShell = pDoc->GetDocShell();
                            pRenderDocShell->GetMedium()->GetItemSet().Put( SfxBoolItem( SID_HIDDEN, false ) );

                        }
                    }
                }
            }
        }
    }
    if( bLastPage )
    {
        // tdf#144989 enable DoIdleJobs() again after last page
        pDoc->getIDocumentTimerAccess().UnblockIdling();
        m_pRenderData.reset();
        m_pPrintUIOptions.reset();
    }
}

// xforms::XFormsSupplier
Reference<XNameContainer> SAL_CALL SwXTextDocument::getXForms()
{
    SolarMutexGuard aGuard;
    if ( !m_pDocShell )
        throw DisposedException(OUString(), getXWeak());
    return GetDocOrThrow().getXForms();
}

uno::Reference< text::XFlatParagraphIterator > SAL_CALL SwXTextDocument::getFlatParagraphIterator(::sal_Int32 nTextMarkupType, sal_Bool bAutomatic)
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    return SwUnoCursorHelper::CreateFlatParagraphIterator(
            GetDocOrThrow(), nTextMarkupType, bAutomatic);
}

uno::Reference< util::XCloneable > SwXTextDocument::createClone(  )
{
    SolarMutexGuard aGuard;
    ThrowIfInvalid();

    // create a new document - hidden - copy the storage and return it
    // SfxObjectShellRef is used here, since the model should control object lifetime after creation
    // and thus SfxObjectShellLock is not allowed here
    // the model holds reference to the shell, so the shell will not destructed at the end of method
    SfxObjectShellRef pShell = GetDocOrThrow().CreateCopy(false, false);
    uno::Reference< frame::XModel > xNewModel = pShell->GetModel();
    uno::Reference< embed::XStorage > xNewStorage = ::comphelper::OStorageHelper::GetTemporaryStorage( );
    uno::Sequence< beans::PropertyValue > aTempMediaDescriptor;
    storeToStorage( xNewStorage, aTempMediaDescriptor );
    uno::Reference< document::XStorageBasedDocument > xStorageDoc( xNewModel, uno::UNO_QUERY );
    xStorageDoc->loadFromStorage( xNewStorage, aTempMediaDescriptor );
    return uno::Reference< util::XCloneable >( xNewModel, UNO_QUERY );
}

void SwXTextDocument::addPasteEventListener(const uno::Reference<text::XPasteListener>& xListener)
{
    SolarMutexGuard aGuard;

    if (m_bObjectValid && xListener.is())
        m_pDocShell->GetWrtShell()->GetPasteListeners().addInterface(xListener);
}

void SwXTextDocument::removePasteEventListener(
    const uno::Reference<text::XPasteListener>& xListener)
{
    SolarMutexGuard aGuard;

    if (m_bObjectValid && xListener.is())
        m_pDocShell->GetWrtShell()->GetPasteListeners().removeInterface(xListener);
}

void SwXTextDocument::paintTile( VirtualDevice &rDevice,
                                 int nOutputWidth, int nOutputHeight,
                                 int nTilePosX, int nTilePosY,
                                 tools::Long nTileWidth, tools::Long nTileHeight )
{
    SwViewShell* pViewShell = m_pDocShell->GetWrtShell();
    pViewShell->PaintTile(rDevice, nOutputWidth, nOutputHeight,
                          nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    LokChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    // Draw Form controls
    comphelper::LibreOfficeKit::setTiledPainting(true);
    SwDrawModel* pDrawLayer = GetDocOrThrow().getIDocumentDrawModelAccess().GetDrawModel();
    SdrPage* pPage = pDrawLayer->GetPage(sal_uInt16(0));
    SdrView* pDrawView = pViewShell->GetDrawView();
    SwEditWin& rEditWin = m_pDocShell->GetView()->GetEditWin();
    tools::Rectangle aTileRect(Point(nTilePosX, nTilePosY), Size(nTileWidth, nTileHeight));
    Size aOutputSize(nOutputWidth, nOutputHeight);
    LokControlHandler::paintControlTile(pPage, pDrawView, rEditWin, rDevice, aOutputSize, aTileRect);
    comphelper::LibreOfficeKit::setTiledPainting(false);
}

Size SwXTextDocument::getDocumentSize()
{
    SwViewShell* pViewShell = m_pDocShell->GetWrtShell();
    Size aDocSize = pViewShell->GetDocSize();

    return Size(aDocSize.Width()  + 2 * DOCUMENTBORDER,
                aDocSize.Height() + 2 * DOCUMENTBORDER);
}

void SwXTextDocument::setPart(int nPart, bool /*bAllowChangeFocus*/)
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (!pWrtShell)
        return;

    pWrtShell->GotoPage(nPart + 1, true);
}

int SwXTextDocument::getParts()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (!pWrtShell)
        return 0;

    return pWrtShell->GetPageCnt();
}

OUString SwXTextDocument::getPartPageRectangles()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (!pWrtShell)
        return OUString();

    return pWrtShell->getPageRectangles();
}

void SwXTextDocument::setClipboard(const uno::Reference<datatransfer::clipboard::XClipboard>& xClipboard)
{
    SolarMutexGuard aGuard;

    SwView* pView = m_pDocShell->GetView();
    if (pView)
        pView->GetEditWin().SetClipboard(xClipboard);
}

bool SwXTextDocument::isMimeTypeSupported()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (!pWrtShell)
        return false;

    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(&pWrtShell->GetView().GetEditWin()));
    if (SdrView* pSdrView = pWrtShell->GetDrawView())
    {
        if (pSdrView->GetTextEditObject())
            // Editing shape text
            return EditEngine::HasValidData(aDataHelper.GetTransferable());
    }

    return aDataHelper.GetXTransferable().is() && SwTransferable::IsPaste(*pWrtShell, aDataHelper);
}

void SwXTextDocument::setClientVisibleArea(const tools::Rectangle& rRectangle)
{
    if (SwView* pView = m_pDocShell->GetView())
    {
        // set the PgUp/PgDown offset
        pView->ForcePageUpDownOffset(2 * rRectangle.GetHeight() / 3);
    }

    if (SwViewShell* pViewShell = m_pDocShell->GetWrtShell())
    {
        pViewShell->setLOKVisibleArea(rRectangle);
    }
}

void SwXTextDocument::setClientZoom(int nTilePixelWidth_, int /*nTilePixelHeight_*/,
                                    int nTileTwipWidth_, int /*nTileTwipHeight_*/)
{
    // Here we set the zoom value as it has been set by the user in the client.
    // This value is used in postMouseEvent and setGraphicSelection methods
    // for in place chart editing. We assume that x and y scale is roughly
    // the same.
    SfxInPlaceClient* pIPClient = m_pDocShell->GetView()->GetIPClient();
    if (!pIPClient)
        return;

    SwViewShell* pWrtViewShell = m_pDocShell->GetWrtShell();
    double fScale = 100.0 * nTilePixelWidth_ / nTileTwipWidth_
                    * o3tl::convert(1.0, o3tl::Length::px, o3tl::Length::twip);
    SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
    if (aOption.GetZoom() != fScale)
    {
        aOption.SetZoom(fScale);
        pWrtViewShell->ApplyViewOptions(aOption);

        // Changing the zoom value doesn't always trigger the updating of
        // the client ole object area, so we call it directly.
        pIPClient->VisAreaChanged();
    }
}

PointerStyle SwXTextDocument::getPointer()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (!pWrtShell)
        return PointerStyle::Arrow;

    return pWrtShell->GetView().GetEditWin().GetPointer();
}

void SwXTextDocument::getTrackedChanges(tools::JsonWriter& rJson)
{
    auto redlinesNode = rJson.startArray("redlines");

    // Disable since usability is very low beyond some small number of changes.
    static bool bDisableRedlineComments = getenv("DISABLE_REDLINE") != nullptr;
    if (bDisableRedlineComments)
        return;

    const SwRedlineTable& rRedlineTable
        = GetDocOrThrow().getIDocumentRedlineAccess().GetRedlineTable();
    for (SwRedlineTable::size_type i = 0; i < rRedlineTable.size(); ++i)
    {
        auto redlineNode = rJson.startStruct();
        rJson.put("index", rRedlineTable[i]->GetId());
        rJson.put("author", rRedlineTable[i]->GetAuthorString(1));
        rJson.put("type", SwRedlineTypeToOUString(
                                       rRedlineTable[i]->GetRedlineData().GetType()));
        rJson.put("comment",
                           rRedlineTable[i]->GetRedlineData().GetComment());
        rJson.put("description", rRedlineTable[i]->GetDescr());
        OUString sDateTime = utl::toISO8601(
            rRedlineTable[i]->GetRedlineData().GetTimeStamp().GetUNODateTime());
        rJson.put("dateTime", sDateTime);

        SwContentNode* pContentNd = rRedlineTable[i]->GetPointContentNode();
        SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
        if (pView && pContentNd)
        {
            SwShellCursor aCursor(pView->GetWrtShell(), *(rRedlineTable[i]->Start()));
            aCursor.SetMark();
            aCursor.GetMark()->Assign(*pContentNd, rRedlineTable[i]->End()->GetContentIndex());

            aCursor.FillRects();

            SwRects* pRects(&aCursor);
            std::vector<OString> aRects;
            for (const SwRect& rNextRect : *pRects)
                aRects.push_back(rNextRect.SVRect().toString());

            const OString sRects = comphelper::string::join("; ", aRects);
            rJson.put("textRange", sRects);
        }
    }
}

void SwXTextDocument::getTrackedChangeAuthors(tools::JsonWriter& rJsonWriter)
{
    SW_MOD()->GetRedlineAuthorInfo(rJsonWriter);
}

void SwXTextDocument::getRulerState(tools::JsonWriter& rJsonWriter)
{
    SwView* pView = m_pDocShell->GetView();
    dynamic_cast<SwCommentRuler&>(pView->GetHRuler()).CreateJsonNotification(rJsonWriter);
}

void SwXTextDocument::getPostIts(tools::JsonWriter& rJsonWriter)
{
    SolarMutexGuard aGuard;
    auto commentsNode = rJsonWriter.startArray("comments");
    for (auto const& sidebarItem : *m_pDocShell->GetView()->GetPostItMgr())
    {
        sw::annotation::SwAnnotationWin* pWin = sidebarItem->mpPostIt.get();

        if (!pWin)
        {
            continue;
        }

        const SwPostItField* pField = pWin->GetPostItField();
        const SwRect& aRect = pWin->GetAnchorRect();
        tools::Rectangle aSVRect(aRect.Pos().getX(),
                                aRect.Pos().getY(),
                                aRect.Pos().getX() + aRect.SSize().Width(),
                                aRect.Pos().getY() + aRect.SSize().Height());

        if (!sidebarItem->maLayoutInfo.mPositionFromCommentAnchor)
        {
            // Comments on frames: anchor position is the corner position, not the whole frame.
            aSVRect.SetSize(Size(0, 0));
        }

        std::vector<OString> aRects;
        for (const basegfx::B2DRange& aRange : pWin->GetAnnotationTextRanges())
        {
            const SwRect rect(aRange.getMinX(), aRange.getMinY(), aRange.getWidth(), aRange.getHeight());
            aRects.push_back(rect.SVRect().toString());
        }
        const OString sRects = comphelper::string::join("; ", aRects);

        auto commentNode = rJsonWriter.startStruct();
        rJsonWriter.put("id", pField->GetPostItId());
        rJsonWriter.put("parentId", pField->GetParentPostItId());
        rJsonWriter.put("author", pField->GetPar1());
        rJsonWriter.put("text", pField->GetPar2());
        rJsonWriter.put("resolved", pField->GetResolved() ? "true" : "false");
        rJsonWriter.put("dateTime", utl::toISO8601(pField->GetDateTime().GetUNODateTime()));
        rJsonWriter.put("anchorPos", aSVRect.toString());
        rJsonWriter.put("textRange", sRects);
        rJsonWriter.put("layoutStatus", static_cast< sal_Int16 >(pWin->GetLayoutStatus()));
    }
}

void SwXTextDocument::executeFromFieldEvent(const StringMap& aArguments)
{
    auto aIter = aArguments.find(u"type"_ustr);
    if (aIter == aArguments.end() || aIter->second != "drop-down")
        return;

    aIter = aArguments.find(u"cmd"_ustr);
    if (aIter == aArguments.end() || aIter->second != "selected")
        return;

    aIter = aArguments.find(u"data"_ustr);
    if (aIter == aArguments.end())
        return;

    sal_Int32 nSelection = aIter->second.toInt32();
    SwPosition aPos(*m_pDocShell->GetWrtShell()->GetCursor()->GetPoint());
    sw::mark::IFieldmark* pFieldBM = m_pDocShell->GetWrtShell()->getIDocumentMarkAccess()->getInnerFieldmarkFor(aPos);
    if ( !pFieldBM )
    {
        aPos.AdjustContent(-1);
        pFieldBM = m_pDocShell->GetWrtShell()->getIDocumentMarkAccess()->getInnerFieldmarkFor(aPos);
    }
    if (pFieldBM && pFieldBM->GetFieldname() == ODF_FORMDROPDOWN)
    {
        if (nSelection >= 0)
        {
            (*pFieldBM->GetParameters())[ODF_FORMDROPDOWN_RESULT] <<= nSelection;
            pFieldBM->Invalidate();
            m_pDocShell->GetWrtShell()->SetModified();
            m_pDocShell->GetView()->GetEditWin().LogicInvalidate(nullptr);
        }
    }
}

std::vector<basegfx::B2DRange>
SwXTextDocument::getSearchResultRectangles(const char* pPayload)
{
    SwDoc* pDoc = m_pDocShell->GetDoc();
    if (!pDoc)
        return std::vector<basegfx::B2DRange>();

    sw::search::SearchResultLocator aLocator(pDoc);
    sw::search::LocationResult aResult = aLocator.findForPayload(pPayload);
    if (aResult.mbFound)
    {
        return aResult.maRectangles;
    }
    return std::vector<basegfx::B2DRange>();
}

OString SwXTextDocument::getViewRenderState(SfxViewShell* pViewShell)
{
    OStringBuffer aState;
    SwView* pView = pViewShell ? dynamic_cast<SwView*>(pViewShell) : m_pDocShell->GetView();
    if (pView && pView->GetWrtShellPtr())
    {
        const SwViewOption* pVOpt = pView->GetWrtShell().GetViewOptions();
        if (pVOpt)
        {
            if (pVOpt->IsViewMetaChars())
                aState.append('P');
            if (pVOpt->IsOnlineSpell())
                aState.append('S');
            aState.append(';');

            OString aThemeName = OUStringToOString(pVOpt->GetThemeName(), RTL_TEXTENCODING_UTF8);
            aState.append(aThemeName);
        }
    }
    return aState.makeStringAndClear();
}

namespace
{
inline constexpr OUString SELECTED_DATE_FORMAT = u"YYYY-MM-DD"_ustr;
}

void SwXTextDocument::executeContentControlEvent(const StringMap& rArguments)
{
    auto it = rArguments.find(u"type"_ustr);
    if (it == rArguments.end())
    {
        return;
    }

    if (it->second == "drop-down")
    {
        SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
        const SwPosition* pStart = pWrtShell->GetCursor()->Start();
        SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
        if (!pTextNode)
        {
            return;
        }

        SwTextAttr* pAttr = pTextNode->GetTextAttrAt(pStart->GetContentIndex(),
                                                     RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
        if (!pAttr)
        {
            return;
        }

        auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
        const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
        std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
        if (!pContentControl->GetComboBox() && !pContentControl->GetDropDown())
        {
            return;
        }

        it = rArguments.find(u"selected"_ustr);
        if (it == rArguments.end())
        {
            return;
        }

        sal_Int32 nSelection = it->second.toInt32();
        pContentControl->SetSelectedListItem(nSelection);
        pWrtShell->GotoContentControl(rFormatContentControl);
    }
    else if (it->second == "picture")
    {
        it = rArguments.find(u"changed"_ustr);
        if (it == rArguments.end())
        {
            return;
        }

        SwView* pView = m_pDocShell->GetView();
        if (!pView)
        {
            return;
        }

        // The current placeholder is selected, so this will replace, not insert.
        SfxStringItem aItem(SID_INSERT_GRAPHIC, it->second);
        pView->GetViewFrame().GetDispatcher()->ExecuteList(SID_CHANGE_PICTURE,
                                                            SfxCallMode::SYNCHRON, { &aItem });
    }
    else if (it->second == "date")
    {
        SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
        const SwPosition* pStart = pWrtShell->GetCursor()->Start();
        SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
        if (!pTextNode)
        {
            return;
        }

        SwTextAttr* pAttr = pTextNode->GetTextAttrAt(pStart->GetContentIndex(),
                                                     RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
        if (!pAttr)
        {
            return;
        }

        auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
        const SwFormatContentControl& rFormatContentControl
            = pTextContentControl->GetContentControl();
        std::shared_ptr<SwContentControl> pContentControl
            = rFormatContentControl.GetContentControl();
        if (!pContentControl->GetDate())
        {
            return;
        }

        it = rArguments.find(u"selected"_ustr);
        if (it == rArguments.end())
        {
            return;
        }

        OUString aSelectedDate = it->second.replaceAll("T00:00:00Z", "");
        SwDoc& rDoc = pTextNode->GetDoc();
        SvNumberFormatter* pNumberFormatter = rDoc.GetNumberFormatter();
        sal_uInt32 nFormat
            = pNumberFormatter->GetEntryKey(SELECTED_DATE_FORMAT, LANGUAGE_ENGLISH_US);
        if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType;
            OUString sFormat = SELECTED_DATE_FORMAT;
            pNumberFormatter->PutEntry(sFormat, nCheckPos, nType, nFormat, LANGUAGE_ENGLISH_US);
        }

        if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            return;
        }

        double dCurrentDate = 0;
        pNumberFormatter->IsNumberFormat(aSelectedDate, nFormat, dCurrentDate);
        pContentControl->SetSelectedDate(dCurrentDate);
        pWrtShell->GotoContentControl(rFormatContentControl);
    }
}

int SwXTextDocument::getPart()
{
    SolarMutexGuard aGuard;

    SwView* pView = m_pDocShell->GetView();
    if (!pView)
        return 0;

    return pView->getPart();
}

OUString SwXTextDocument::getPartName(int nPart)
{
    return SwResId(STR_PAGE) + OUString::number(nPart + 1);
}

OUString SwXTextDocument::getPartHash(int nPart)
{
    OUString sPart(SwResId(STR_PAGE) + OUString::number(nPart + 1));

    return OUString::number(sPart.hashCode());
}

VclPtr<vcl::Window> SwXTextDocument::getDocWindow()
{
    SolarMutexGuard aGuard;
    SwView* pView = m_pDocShell->GetView();
    if (!pView)
        return {};

    if (VclPtr<vcl::Window> pWindow = SfxLokHelper::getInPlaceDocWindow(pView))
        return pWindow;

    return &(pView->GetEditWin());
}

void SwXTextDocument::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    SolarMutexGuard aGuard;

    SwViewShell* pViewShell = m_pDocShell->GetWrtShell();

    SwView* pView = m_pDocShell->GetView();
    if (!pView)
        return;

    pView->SetViewLayout(1/*nColumns*/, false/*bBookMode*/, true);

    // Tiled rendering defaults.
    SwViewOption aViewOption(*pViewShell->GetViewOptions());
    aViewOption.SetHardBlank(false);

    // Disable field shadings: the result would depend on the cursor position.
    aViewOption.SetAppearanceFlag(ViewOptFlags::FieldShadings, false);
    // The fancy header/footer controls don't work in tiled mode anyway, so
    // explicitly disable them to enable skipping invalidating the view for
    // the case of clicking in the header area of a document with no headers
    aViewOption.SetUseHeaderFooterMenu(false);

    OUString sThemeName;
    OUString sOrigAuthor = SW_MOD()->GetRedlineAuthor(SW_MOD()->GetRedlineAuthor());
    OUString sAuthor;

    for (const beans::PropertyValue& rValue : rArguments)
    {
        if (rValue.Name == ".uno:HideWhitespace" && rValue.Value.has<bool>())
            aViewOption.SetHideWhitespaceMode(rValue.Value.get<bool>());
        else if (rValue.Name == ".uno:ShowBorderShadow" && rValue.Value.has<bool>())
            aViewOption.SetAppearanceFlag(ViewOptFlags::Shadow , rValue.Value.get<bool>());
        else if (rValue.Name == ".uno:Author" && rValue.Value.has<OUString>())
        {
            sAuthor = rValue.Value.get<OUString>();
            // Store the author name in the view.
            pView->SetRedlineAuthor(sAuthor);
            // Let the actual author name pick up the value from the current
            // view, which would normally happen only during the next view
            // switch.
            m_pDocShell->SetView(pView);
        }
        else if (rValue.Name == ".uno:SpellOnline" && rValue.Value.has<bool>())
            aViewOption.SetOnlineSpell(rValue.Value.get<bool>());
        else if (rValue.Name == ".uno:ChangeTheme" && rValue.Value.has<OUString>())
            sThemeName = rValue.Value.get<OUString>();
    }

    if (!sAuthor.isEmpty() && sAuthor != sOrigAuthor)
    {
        SwView* pFirstView = static_cast<SwView*>(SfxViewShell::GetFirst());
        if (pFirstView && SfxViewShell::GetNext(*pFirstView) == nullptr)
        {
            if (SwEditShell* pShell = &pFirstView->GetWrtShell())
            {
                pShell->SwViewShell::UpdateFields(true);
                pShell->ResetModified();
            }
        }
    }

    // Set the initial zoom value to 1; usually it is set in setClientZoom and
    // SwViewShell::PaintTile; zoom value is used for chart in place
    // editing, see postMouseEvent and setGraphicSelection methods.
    aViewOption.SetZoom(1 * 100);

    aViewOption.SetPostIts(comphelper::LibreOfficeKit::isTiledAnnotations());
    pViewShell->ApplyViewOptions(aViewOption);

    // position the pages again after setting view options. Eg: if postit
    // rendering is false, then there would be no sidebar, so width of the
    // document needs to be adjusted
    pViewShell->GetLayout()->CheckViewLayout( pViewShell->GetViewOptions(), nullptr );

    // Disable map mode, so that it's possible to send mouse event coordinates
    // directly in twips.
    SwEditWin& rEditWin = m_pDocShell->GetView()->GetEditWin();
    rEditWin.EnableMapMode(false);

    // when the "This document may contain formatting or content that cannot
    // be saved..." dialog appears, it is auto-cancelled with tiled rendering,
    // causing 'Save' being disabled; so let's always save to the original
    // format
    auto xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Save::Document::WarnAlienFormat::set(false, xChanges);
    xChanges->commit();

    // disable word auto-completion suggestions, the tooltips are not visible,
    // and the editeng-like auto-completion is annoying
    SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags().bAutoCompleteWords = false;

    // don't change the whitespace at the beginning of paragraphs, this is
    // annoying when taking minutes without further formatting
    SwEditShell::GetAutoFormatFlags()->bAFormatByInpDelSpacesAtSttEnd = false;

    // if we know what theme the user wants, then we can dispatch that now early
    if (!sThemeName.isEmpty())
    {
        css::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            { "NewTheme", uno::Any(sThemeName) }
        }));
        comphelper::dispatchCommand(u".uno:ChangeTheme"_ustr, aPropertyValues);
    }
}

void SwXTextDocument::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;
    SfxLokHelper::postKeyEventAsync(getDocWindow(), nType, nCharCode, nKeyCode);
}

void SwXTextDocument::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    SwViewShell* pWrtViewShell = m_pDocShell->GetWrtShell();
    if (!pWrtViewShell)
    {
        return;
    }

    SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
    double fScale = aOption.GetZoom() / o3tl::convert(100.0, o3tl::Length::px, o3tl::Length::twip);

    if (SfxLokHelper::testInPlaceComponentMouseEventHit(
            m_pDocShell->GetView(), nType, nX, nY, nCount, nButtons, nModifier, fScale, fScale))
        return;

    // try to forward mouse event to controls
    SwDrawModel* pDrawLayer = GetDocOrThrow().getIDocumentDrawModelAccess().GetDrawModel();
    SdrPage* pPage = pDrawLayer->GetPage(sal_uInt16(0));
    SdrView* pDrawView = pWrtViewShell->GetDrawView();
    SwEditWin& rEditWin = m_pDocShell->GetView()->GetEditWin();
    Point aPointTwip(nX, nY);
    Point aPointHMMDraw = o3tl::convert(aPointTwip, o3tl::Length::twip, o3tl::Length::mm100);
    if (LokControlHandler::postMouseEvent(pPage, pDrawView, rEditWin, nType, aPointHMMDraw, nCount, nButtons, nModifier))
            return;

    LokMouseEventData aMouseEventData(nType, Point(nX, nY), nCount,
                                      MouseEventModifiers::SIMPLECLICK,
                                      nButtons, nModifier);
    SfxLokHelper::postMouseEventAsync(&rEditWin, aMouseEventData);
}

void SwXTextDocument::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    SfxViewShell* pViewShell = m_pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setTextSelection(nType, nX, nY))
        return;

    SwEditWin& rEditWin = m_pDocShell->GetView()->GetEditWin();
    switch (nType)
    {
    case LOK_SETTEXTSELECTION_START:
        rEditWin.SetCursorTwipPosition(Point(nX, nY), /*bPoint=*/false, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_END:
        rEditWin.SetCursorTwipPosition(Point(nX, nY), /*bPoint=*/true, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_RESET:
        rEditWin.SetCursorTwipPosition(Point(nX, nY), /*bPoint=*/true, /*bClearMark=*/true);
        break;
    default:
        assert(false);
        break;
    }
}

uno::Reference<datatransfer::XTransferable> SwXTextDocument::getSelection()
{
    SolarMutexGuard aGuard;

    uno::Reference<datatransfer::XTransferable> xTransferable;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    if (SdrView* pSdrView = pWrtShell ? pWrtShell->GetDrawView() : nullptr)
    {
        if (pSdrView->GetTextEditObject())
        {
            // Editing shape text
            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
            xTransferable = rEditView.getEditEngine().CreateTransferable(rEditView.GetSelection());
        }
    }

    if (SwPostItMgr* pPostItMgr = m_pDocShell->GetView()->GetPostItMgr())
    {
        if (sw::annotation::SwAnnotationWin* pWin = pPostItMgr->GetActiveSidebarWin())
        {
            // Editing postit text.
            EditView& rEditView = pWin->GetOutlinerView()->GetEditView();
            xTransferable = rEditView.getEditEngine().CreateTransferable(rEditView.GetSelection());
        }
    }

    if (!xTransferable.is() && pWrtShell)
        xTransferable = new SwTransferable(*pWrtShell);

    return xTransferable;
}

void SwXTextDocument::setGraphicSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    SwViewShell* pWrtViewShell = m_pDocShell->GetWrtShell();
    SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
    double fScale = aOption.GetZoom() / o3tl::convert(100.0, o3tl::Length::px, o3tl::Length::twip);

    SfxViewShell* pViewShell = m_pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setGraphicSelection(nType, nX, nY, fScale, fScale))
        return;

    SwEditWin& rEditWin = m_pDocShell->GetView()->GetEditWin();
    switch (nType)
    {
    case LOK_SETGRAPHICSELECTION_START:
        rEditWin.SetGraphicTwipPosition(/*bStart=*/true, Point(nX, nY));
        break;
    case LOK_SETGRAPHICSELECTION_END:
        rEditWin.SetGraphicTwipPosition(/*bStart=*/false, Point(nX, nY));
        break;
    default:
        assert(false);
        break;
    }
}

void SwXTextDocument::resetSelection()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = m_pDocShell->GetWrtShell();
    pWrtShell->ResetSelect(nullptr, false);
}

void SAL_CALL SwXTextDocument::paintTile( const ::css::uno::Any& Parent, ::sal_Int32 nOutputWidth, ::sal_Int32 nOutputHeight, ::sal_Int32 nTilePosX, ::sal_Int32 nTilePosY, ::sal_Int32 nTileWidth, ::sal_Int32 nTileHeight )
{
    SystemGraphicsData aData;
    aData.nSize = sizeof(SystemGraphicsData);
    #if defined(_WIN32)
    sal_Int64 nWindowHandle;
    Parent >>= nWindowHandle;
    aData.hWnd = reinterpret_cast<HWND>(nWindowHandle);
    ScopedVclPtrInstance<VirtualDevice> xDevice(aData, Size(1, 1), DeviceFormat::WITHOUT_ALPHA);
    paintTile(*xDevice, nOutputWidth, nOutputHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    #else
    // TODO: support other platforms
    (void)Parent;
    (void)nOutputWidth;
    (void)nOutputHeight;
    (void)nTilePosX;
    (void)nTilePosY;
    (void)nTileWidth;
    (void)nTileHeight;
    #endif
}

/**
 * retrieve languages already used in current document
 */
uno::Sequence< lang::Locale > SAL_CALL SwXTextDocument::getDocumentLanguages(
        ::sal_Int16 nScriptTypes,
        ::sal_Int16 nMaxCount )
{
    SolarMutexGuard aGuard;

    // possible canonical values for nScriptTypes
    // any bit wise combination is allowed
    const sal_Int16 nLatin   = 0x001;
    const sal_Int16 nAsian   = 0x002;
    const sal_Int16 nComplex = 0x004;

    // script types for which to get the languages
    const bool bLatin   = 0 != (nScriptTypes & nLatin);
    const bool bAsian   = 0 != (nScriptTypes & nAsian);
    const bool bComplex = 0 != (nScriptTypes & nComplex);

    if (nScriptTypes < nLatin || nScriptTypes > (nLatin | nAsian | nComplex))
        throw IllegalArgumentException(u"nScriptTypes ranges from 1 to 7!"_ustr, Reference< XInterface >(), 1);
    if (!m_pDocShell)
        throw DisposedException();
    SwDoc& rDoc = GetDocOrThrow();

    // avoid duplicate values
    std::set< LanguageType > aAllLangs;

    //USER STYLES

    const SwCharFormats *pFormats = rDoc.GetCharFormats();
    for(size_t i = 0; i < pFormats->size(); ++i)
    {
        const SwAttrSet &rAttrSet = (*pFormats)[i]->GetAttrSet();
        LanguageType nLang = LANGUAGE_DONTKNOW;
        if (bLatin)
        {
            nLang = rAttrSet.GetLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bAsian)
        {
            nLang = rAttrSet.GetCJKLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bComplex)
        {
            nLang = rAttrSet.GetCTLLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
    }

    const SwTextFormatColls *pColls = rDoc.GetTextFormatColls();
    for (size_t i = 0; i < pColls->size(); ++i)
    {
        const SwAttrSet &rAttrSet = (*pColls)[i]->GetAttrSet();
        LanguageType nLang = LANGUAGE_DONTKNOW;
        if (bLatin)
        {
            nLang = rAttrSet.GetLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bAsian)
        {
            nLang = rAttrSet.GetCJKLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bComplex)
        {
            nLang = rAttrSet.GetCTLLanguage( false ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
    }

    //AUTO STYLES
    const IStyleAccess::SwAutoStyleFamily aFam[2] =
    {
      IStyleAccess::AUTO_STYLE_CHAR,
      IStyleAccess::AUTO_STYLE_PARA
    };
    for (IStyleAccess::SwAutoStyleFamily i : aFam)
    {
        std::vector< std::shared_ptr<SfxItemSet> > rStyles;
        rDoc.GetIStyleAccess().getAllStyles(rStyles, i);
        while (!rStyles.empty())
        {
            std::shared_ptr<SfxItemSet> pStyle = rStyles.back();
            rStyles.pop_back();
            const SfxItemSet *pSet = pStyle.get();

            LanguageType nLang = LANGUAGE_DONTKNOW;
            if (bLatin)
            {
                assert(pSet);
                nLang = pSet->Get( RES_CHRATR_LANGUAGE, false ).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bAsian)
            {
                assert(pSet);
                nLang = pSet->Get( RES_CHRATR_CJK_LANGUAGE, false ).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bComplex)
            {
                assert(pSet);
                nLang = pSet->Get( RES_CHRATR_CTL_LANGUAGE, false ).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
        }
    }

    //TODO/mba: it's a strange concept that a view is needed to retrieve core data
    SwWrtShell *pWrtSh = m_pDocShell->GetWrtShell();
    SdrView *pSdrView = pWrtSh->GetDrawView();

    if( pSdrView )
    {
        SdrOutliner* pOutliner = pSdrView->GetTextEditOutliner();
        if(pOutliner)
        {
            EditEngine& rEditEng = const_cast<EditEngine&>(pOutliner->GetEditEngine());
            sal_Int32 nParCount = pOutliner->GetParagraphCount();
            for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
            {
                //every paragraph
                std::vector<sal_Int32> aPortions;
                rEditEng.GetPortions( nPar, aPortions );

                for ( size_t nPos = aPortions.size(); nPos; )
                {
                    //every position
                    --nPos;
                    sal_Int32 nEnd = aPortions[ nPos ];
                    sal_Int32 nStart = nPos ? aPortions[ nPos - 1 ] : 0;
                    ESelection aSelection( nPar, nStart, nPar, nEnd );
                    SfxItemSet aAttr = rEditEng.GetAttribs( aSelection );

                    LanguageType nLang = LANGUAGE_DONTKNOW;
                    if (bLatin)
                    {
                        nLang = aAttr.Get( EE_CHAR_LANGUAGE, false ).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bAsian)
                    {
                        nLang = aAttr.Get( EE_CHAR_LANGUAGE_CJK, false ).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bComplex)
                    {
                        nLang = aAttr.Get( EE_CHAR_LANGUAGE_CTL, false ).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                }
            }
        }
    }
    // less than nMaxCount languages
    if (nMaxCount > static_cast< sal_Int16 >( aAllLangs.size() ))
        nMaxCount = static_cast< sal_Int16 >( aAllLangs.size() );

    // build return value
    uno::Sequence< lang::Locale > aLanguages( nMaxCount );
    lang::Locale* pLanguage = aLanguages.getArray();
    if (nMaxCount > 0)
    {
        sal_Int32 nCount = 0;
        for (const auto& rLang : aAllLangs)
        {
            if (nCount >= nMaxCount)
                break;
            if (LANGUAGE_NONE != rLang)
            {
                pLanguage[nCount] = LanguageTag::convertToLocale( rLang );
                pLanguage[nCount].Language = SvtLanguageTable::GetLanguageString( rLang );
                nCount += 1;
            }
        }
    }

    return aLanguages;
}

SwXLinkTargetSupplier::SwXLinkTargetSupplier(SwXTextDocument& rxDoc) :
    m_pxDoc(&rxDoc)
{
    m_sTables     = SwResId(STR_CONTENT_TYPE_TABLE);
    m_sFrames     = SwResId(STR_CONTENT_TYPE_FRAME);
    m_sGraphics   = SwResId(STR_CONTENT_TYPE_GRAPHIC);
    m_sOLEs       = SwResId(STR_CONTENT_TYPE_OLE);
    m_sSections   = SwResId(STR_CONTENT_TYPE_REGION);
    m_sOutlines   = SwResId(STR_CONTENT_TYPE_OUTLINE);
    m_sBookmarks  = SwResId(STR_CONTENT_TYPE_BOOKMARK);
    m_sDrawingObjects = SwResId(STR_CONTENT_TYPE_DRAWOBJECT);
}

SwXLinkTargetSupplier::~SwXLinkTargetSupplier()
{
}

Any SwXLinkTargetSupplier::getByName(const OUString& rName)
{
    Any aRet;
    if(!m_pxDoc)
        throw RuntimeException(u"No document available"_ustr);
    OUString sSuffix(u"|"_ustr);
    if(rName == m_sTables)
    {
        sSuffix += "table";

        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getTextTables(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sFrames)
    {
        sSuffix += "frame";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getTextFrames(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sSections)
    {
        sSuffix += "region";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getTextSections(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sGraphics)
    {
        sSuffix += "graphic";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getGraphicObjects(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sOLEs)
    {
        sSuffix += "ole";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getEmbeddedObjects(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sOutlines)
    {
        sSuffix += "outline";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        *m_pxDoc, rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == m_sBookmarks)
    {
        sSuffix.clear();
        Reference< XNameAccess >  xBkms = new SwXLinkNameAccessWrapper(
                                        m_pxDoc->getBookmarks(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xBkms, UNO_QUERY);
    }
    else if(rName == m_sDrawingObjects)
    {
        sSuffix += "drawingobject";
        Reference<XNameAccess> xDrawingObjects = new SwXLinkNameAccessWrapper(
                    *m_pxDoc, rName, sSuffix);
        aRet <<= Reference<XPropertySet>(xDrawingObjects, UNO_QUERY);
    }
    else
        throw NoSuchElementException();
    return aRet;
}

Sequence< OUString > SwXLinkTargetSupplier::getElementNames()
{
    return { m_sTables,
             m_sFrames,
             m_sGraphics,
             m_sOLEs,
             m_sSections,
             m_sOutlines,
             m_sBookmarks,
             m_sDrawingObjects };
}

sal_Bool SwXLinkTargetSupplier::hasByName(const OUString& rName)
{
    if( rName == m_sTables  ||
        rName == m_sFrames  ||
        rName == m_sGraphics||
        rName == m_sOLEs   ||
        rName == m_sSections ||
        rName == m_sOutlines ||
        rName == m_sBookmarks ||
        rName == m_sDrawingObjects )
        return true;
    return false;
}

uno::Type  SwXLinkTargetSupplier::getElementType()
{
    return cppu::UnoType<XPropertySet>::get();

}

sal_Bool SwXLinkTargetSupplier::hasElements()
{
    return nullptr != m_pxDoc;
}

OUString SwXLinkTargetSupplier::getImplementationName()
{
    return u"SwXLinkTargetSupplier"_ustr;
}

sal_Bool SwXLinkTargetSupplier::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXLinkTargetSupplier::getSupportedServiceNames()
{
    Sequence< OUString > aRet { u"com.sun.star.document.LinkTargets"_ustr };
    return aRet;
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(
            Reference< XNameAccess > const & xAccess, OUString aLinkDisplayName, OUString sSuffix ) :
    m_xRealAccess(xAccess),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    m_sLinkSuffix(std::move(sSuffix)),
    m_sLinkDisplayName(std::move(aLinkDisplayName)),
    m_pxDoc(nullptr)
{
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            OUString aLinkDisplayName, OUString sSuffix) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    m_sLinkSuffix(std::move(sSuffix)),
    m_sLinkDisplayName(std::move(aLinkDisplayName)),
    m_pxDoc(&rxDoc)
{
}

SwXLinkNameAccessWrapper::~SwXLinkNameAccessWrapper()
{
}

Any SwXLinkNameAccessWrapper::getByName(const OUString& rName)
{
    Any aRet;
    bool bFound = false;
    //cut link extension and call the real NameAccess
    OUString sParam = rName;
    OUString sSuffix(m_sLinkSuffix);
    if(sParam.getLength() > sSuffix.getLength() )
    {
        std::u16string_view sCmp = sParam.subView(sParam.getLength() - sSuffix.getLength(),
                                                    sSuffix.getLength());
        if(sCmp == sSuffix)
        {
            if(m_pxDoc)
            {
                sParam = sParam.copy(0, sParam.getLength() - sSuffix.getLength());
                if(!m_pxDoc->GetDocShell())
                    throw RuntimeException(u"No document shell available"_ustr);
                SwDoc* pDoc = m_pxDoc->GetDocShell()->GetDoc();

                if (sSuffix == "|outline")
                {
                    const size_t nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                    for (size_t i = 0; i < nOutlineCount && !bFound; ++i)
                    {
                        if(sParam == lcl_CreateOutlineString(i, pDoc))
                        {
                            OUString sOutlineText =
                                    pDoc->getIDocumentOutlineNodes().getOutlineText(
                                        i, pDoc->GetDocShell()->GetWrtShell()->GetLayout());
                            sal_Int32 nOutlineLevel = pDoc->getIDocumentOutlineNodes().getOutlineLevel(i);
                            Reference<XPropertySet> xOutline =
                                    new SwXOutlineTarget(sParam, sOutlineText, nOutlineLevel);
                            aRet <<= xOutline;
                            bFound = true;
                        }
                    }
                }
                else if (sSuffix == "|drawingobject")
                {
                    SwDrawModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
                    if (pModel)
                    {
                        SdrPage* pPage = pModel->GetPage(0);
                        for (const rtl::Reference<SdrObject>& pObj : *pPage)
                        {
                            if (sParam == pObj->GetName())
                            {
                                Reference<XPropertySet> xDrawingObject = new SwXDrawingObjectTarget(sParam);
                                aRet <<= xDrawingObject;
                                bFound = true;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                aRet = m_xRealAccess->getByName(sParam.copy(0, sParam.getLength() - sSuffix.getLength()));
                Reference< XInterface > xInt;
                if(!(aRet >>= xInt))
                    throw RuntimeException(u"Could not retrieve property"_ustr);
                Reference< XPropertySet >  xProp(xInt, UNO_QUERY);
                aRet <<= xProp;
                bFound = true;
            }
        }
    }
    if(!bFound)
        throw NoSuchElementException();
    return aRet;
}

Sequence< OUString > SwXLinkNameAccessWrapper::getElementNames()
{
    Sequence< OUString > aRet;
    if(m_pxDoc)
    {
        if(!m_pxDoc->GetDocShell())
            throw RuntimeException(u"No document shell available"_ustr);
        SwDoc* pDoc = m_pxDoc->GetDocShell()->GetDoc();
        if (m_sLinkSuffix == "|outline")
        {
            const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
            const size_t nOutlineCount = rOutlineNodes.size();
            aRet.realloc(nOutlineCount);
            OUString* pResArr = aRet.getArray();
            for (size_t i = 0; i < nOutlineCount; ++i)
            {
                pResArr[i] = lcl_CreateOutlineString(i, pDoc) + "|outline";
            }
        }
        else if (m_sLinkSuffix == "|drawingobject")
        {
            SwDrawModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
            if(pModel)
            {
                SdrPage* pPage = pModel->GetPage(0);
                const size_t nObjCount = pPage->GetObjCount();
                aRet.realloc(nObjCount);
                OUString* pResArr = aRet.getArray();
                auto j = 0;
                for (const rtl::Reference<SdrObject>& pObj : *pPage)
                {
                    if (!pObj->GetName().isEmpty())
                        pResArr[j++] = pObj->GetName() + "|drawingobject";
                }
            }
        }
    }
    else
    {
        const Sequence< OUString > aOrg = m_xRealAccess->getElementNames();
        aRet.realloc(aOrg.getLength());
        std::transform(aOrg.begin(), aOrg.end(), aRet.getArray(),
            [this](const OUString& rOrg) -> OUString { return rOrg + m_sLinkSuffix; });
    }
    return aRet;
}

sal_Bool SwXLinkNameAccessWrapper::hasByName(const OUString& rName)
{
    bool bRet = false;
    OUString sParam(rName);
    if(sParam.getLength() > m_sLinkSuffix.getLength() )
    {
        std::u16string_view sCmp = sParam.subView(sParam.getLength() - m_sLinkSuffix.getLength(),
                                                    m_sLinkSuffix.getLength());
        if(sCmp == m_sLinkSuffix)
        {
            sParam = sParam.copy(0, sParam.getLength() - m_sLinkSuffix.getLength());
            if(m_pxDoc)
            {
                if(!m_pxDoc->GetDocShell())
                    throw RuntimeException(u"No document shell available"_ustr);
                SwDoc* pDoc = m_pxDoc->GetDocShell()->GetDoc();
                if (m_sLinkSuffix == "|outline")
                {
                    const size_t nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                    for (size_t i = 0; i < nOutlineCount && !bRet; ++i)
                    {
                        if(sParam == lcl_CreateOutlineString(i, pDoc))
                        {
                            bRet = true;
                        }
                    }
                }
                else if (m_sLinkSuffix == "|drawingobject")
                {
                    SwDrawModel* pModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
                    if (pModel)
                    {
                        SdrPage* pPage = pModel->GetPage(0);
                        for (const rtl::Reference<SdrObject>& pObj : *pPage)
                        {
                            if (sParam == pObj->GetName())
                            {
                                bRet = true;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                bRet = m_xRealAccess->hasByName(sParam);
            }
        }
    }
    return bRet;
}

uno::Type  SwXLinkNameAccessWrapper::getElementType()
{
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SwXLinkNameAccessWrapper::hasElements()
{
    bool bRet = false;
    if(m_pxDoc)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        bRet = m_xRealAccess->hasElements();
    }
    return bRet;
}

Reference< XPropertySetInfo >  SwXLinkNameAccessWrapper::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = m_pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXLinkNameAccessWrapper::setPropertyValue(
    const OUString& rPropName, const Any& )
{
    throw UnknownPropertyException(rPropName);
}

static Any lcl_GetDisplayBitmap(std::u16string_view sLinkSuffix)
{
    Any aRet;
    if(!sLinkSuffix.empty())
        sLinkSuffix = sLinkSuffix.substr(1);
    OUString sImgId;

    if(sLinkSuffix == u"outline")
        sImgId = RID_BMP_NAVI_OUTLINE;
    else if(sLinkSuffix == u"table")
        sImgId = RID_BMP_NAVI_TABLE;
    else if(sLinkSuffix == u"frame")
        sImgId = RID_BMP_NAVI_FRAME;
    else if(sLinkSuffix == u"graphic")
        sImgId = RID_BMP_NAVI_GRAPHIC;
    else if(sLinkSuffix == u"ole")
        sImgId = RID_BMP_NAVI_OLE;
    else if(sLinkSuffix.empty())
        sImgId = RID_BMP_NAVI_BOOKMARK;
    else if(sLinkSuffix == u"region")
        sImgId = RID_BMP_NAVI_REGION;
    else if(sLinkSuffix == u"drawingobject")
        sImgId = RID_BMP_NAVI_DRAWOBJECT;

    if (!sImgId.isEmpty())
    {
        aRet <<= VCLUnoHelper::CreateBitmap(BitmapEx(sImgId));
    }
    return aRet;
}

Any SwXLinkNameAccessWrapper::getPropertyValue(const OUString& rPropertyName)
{
    Any aRet;
    if( rPropertyName == UNO_LINK_DISPLAY_NAME )
    {
        aRet <<= m_sLinkDisplayName;
    }
    else if( rPropertyName == UNO_LINK_DISPLAY_BITMAP )
    {
        aRet = lcl_GetDisplayBitmap(m_sLinkSuffix);
    }
    else
        throw UnknownPropertyException(rPropertyName);
    return aRet;
}

void SwXLinkNameAccessWrapper::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{}

void SwXLinkNameAccessWrapper::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{}

void SwXLinkNameAccessWrapper::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{}

void SwXLinkNameAccessWrapper::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{}

Reference< XNameAccess >  SwXLinkNameAccessWrapper::getLinks()
{
    return this;
}

OUString SwXLinkNameAccessWrapper::getImplementationName()
{
    return u"SwXLinkNameAccessWrapper"_ustr;
}

sal_Bool SwXLinkNameAccessWrapper::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXLinkNameAccessWrapper::getSupportedServiceNames()
{
    Sequence< OUString > aRet { u"com.sun.star.document.LinkTargets"_ustr };
    return aRet;
}

SwXOutlineTarget::SwXOutlineTarget(OUString aOutlineText, OUString aActualText,
                                   const sal_Int32 nOutlineLevel) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    m_sOutlineText(std::move(aOutlineText)),
    m_sActualText(std::move(aActualText)),
    m_nOutlineLevel(nOutlineLevel)
{
}

SwXOutlineTarget::~SwXOutlineTarget()
{
}

Reference< XPropertySetInfo >  SwXOutlineTarget::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = m_pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXOutlineTarget::setPropertyValue(
    const OUString& rPropertyName, const Any& /*aValue*/)
{
    throw UnknownPropertyException(rPropertyName);
}

Any SwXOutlineTarget::getPropertyValue(const OUString& rPropertyName)
{
    if (rPropertyName != UNO_LINK_DISPLAY_NAME && rPropertyName != "ActualOutlineName" &&
            rPropertyName != "OutlineLevel")
        throw UnknownPropertyException(rPropertyName);

    if (rPropertyName == "ActualOutlineName")
        return Any(m_sActualText);

    if (rPropertyName == "OutlineLevel")
        return Any(m_nOutlineLevel);

    return Any(m_sOutlineText);
}

void SwXOutlineTarget::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
}

void SwXOutlineTarget::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
}

void SwXOutlineTarget::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
}

void SwXOutlineTarget::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
}

OUString SwXOutlineTarget::getImplementationName()
{
    return u"SwXOutlineTarget"_ustr;
}

sal_Bool SwXOutlineTarget::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SwXOutlineTarget::getSupportedServiceNames()
{
    Sequence<OUString> aRet { u"com.sun.star.document.LinkTarget"_ustr };

    return aRet;
}

SwXDrawingObjectTarget::SwXDrawingObjectTarget(OUString aDrawingObjectText) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    m_sDrawingObjectText(std::move(aDrawingObjectText))
{
}

SwXDrawingObjectTarget::~SwXDrawingObjectTarget()
{
}

Reference< XPropertySetInfo >  SwXDrawingObjectTarget::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = m_pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXDrawingObjectTarget::setPropertyValue(
    const OUString& rPropertyName, const Any& /*aValue*/)
{
    throw UnknownPropertyException(rPropertyName);
}

Any SwXDrawingObjectTarget::getPropertyValue(const OUString& rPropertyName)
{
    if(rPropertyName != UNO_LINK_DISPLAY_NAME)
        throw UnknownPropertyException(rPropertyName);

    return Any(m_sDrawingObjectText);
}

void SwXDrawingObjectTarget::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
}

void SwXDrawingObjectTarget::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
{
}

void SwXDrawingObjectTarget::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
}

void SwXDrawingObjectTarget::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
{
}

OUString SwXDrawingObjectTarget::getImplementationName()
{
    return u"SwXDrawingObjectTarget"_ustr;
}

sal_Bool SwXDrawingObjectTarget::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SwXDrawingObjectTarget::getSupportedServiceNames()
{
    Sequence<OUString> aRet { u"com.sun.star.document.LinkTarget"_ustr };

    return aRet;
}

SwXDocumentPropertyHelper::SwXDocumentPropertyHelper(SwDoc& rDoc) :
SvxUnoForbiddenCharsTable ( rDoc.getIDocumentSettingAccess().getForbiddenCharacterTable() )
,m_pDoc(&rDoc)
{
}

SwXDocumentPropertyHelper::~SwXDocumentPropertyHelper()
{
}

Reference<XInterface> SwXDocumentPropertyHelper::GetDrawTable(SwCreateDrawTable nWhich)
{
    Reference<XInterface> xRet;
    if(m_pDoc)
    {
        switch(nWhich)
        {
            // #i52858#
            // assure that Draw model is created, if it doesn't exist.
            case SwCreateDrawTable::Dash         :
                if(!m_xDashTable.is())
                    m_xDashTable = SvxUnoDashTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xDashTable;
            break;
            case SwCreateDrawTable::Gradient     :
                if(!m_xGradientTable.is())
                    m_xGradientTable = SvxUnoGradientTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xGradientTable;
            break;
            case SwCreateDrawTable::Hatch        :
                if(!m_xHatchTable.is())
                    m_xHatchTable = SvxUnoHatchTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xHatchTable;
            break;
            case SwCreateDrawTable::Bitmap       :
                if(!m_xBitmapTable.is())
                    m_xBitmapTable = SvxUnoBitmapTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xBitmapTable;
            break;
            case SwCreateDrawTable::TransGradient:
                if(!m_xTransGradientTable.is())
                    m_xTransGradientTable = SvxUnoTransGradientTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xTransGradientTable;
            break;
            case SwCreateDrawTable::Marker       :
                if(!m_xMarkerTable.is())
                    m_xMarkerTable = SvxUnoMarkerTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = m_xMarkerTable;
            break;
            case  SwCreateDrawTable::Defaults:
                if(!m_xDrawDefaults.is())
                    m_xDrawDefaults = cppu::getXWeak(new SwSvxUnoDrawPool(*m_pDoc));
                xRet = m_xDrawDefaults;
            break;
#if OSL_DEBUG_LEVEL > 0
            default: OSL_FAIL("which table?");
#endif
        }
    }
    return xRet;
}

void SwXDocumentPropertyHelper::Invalidate()
{
    m_xDashTable = nullptr;
    m_xGradientTable = nullptr;
    m_xHatchTable = nullptr;
    m_xBitmapTable = nullptr;
    m_xTransGradientTable = nullptr;
    m_xMarkerTable = nullptr;
    m_xDrawDefaults = nullptr;
    m_pDoc = nullptr;
    SvxUnoForbiddenCharsTable::mxForbiddenChars.reset();
}

void SwXDocumentPropertyHelper::onChange()
{
    if(m_pDoc)
       m_pDoc->getIDocumentState().SetModified();
}

SwViewOptionAdjust_Impl::SwViewOptionAdjust_Impl(
            SwViewShell& rSh, const SwViewOption &rViewOptions)
    : m_pShell(&rSh)
    , m_aOldViewOptions( rViewOptions )
{
}

SwViewOptionAdjust_Impl::~SwViewOptionAdjust_Impl()
{
    if (m_pShell)
    {
        m_pShell->ApplyViewOptions( m_aOldViewOptions );
    }
}

void
SwViewOptionAdjust_Impl::AdjustViewOptions(SwPrintData const*const pPrtOptions, bool setShowPlaceHoldersInPDF)
{
    // to avoid unnecessary reformatting the view options related to the content
    // below should only change if necessary, that is if respective content is present
    const bool bContainsHiddenChars         = m_pShell->GetDoc()->ContainsHiddenChars();
    const SwFieldType* pFieldType = m_pShell->GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenText );
    const bool bContainsHiddenFields        = pFieldType && pFieldType->HasWriterListeners();
    pFieldType = m_pShell->GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::HiddenPara );
    const bool bContainsHiddenParagraphs    = pFieldType && pFieldType->HasWriterListeners();
    pFieldType = m_pShell->GetDoc()->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::JumpEdit );
    const bool bContainsPlaceHolders        = pFieldType && pFieldType->HasWriterListeners();
    const bool bContainsFields              = m_pShell->IsAnyFieldInDoc();

    SwViewOption aRenderViewOptions( m_aOldViewOptions );

    // disable anything in the view that should not be printed (or exported to PDF) by default
    // (see also dialog "Tools/Options - StarOffice Writer - Formatting Aids"
    // in section "Display of ...")
    aRenderViewOptions.SetParagraph( false );             // paragraph end
    aRenderViewOptions.SetSoftHyph( false );              // aka custom hyphens
    aRenderViewOptions.SetBlank( false );                 // spaces
    aRenderViewOptions.SetHardBlank( false );             // non-breaking spaces
    aRenderViewOptions.SetTab( false );                   // tabs
    aRenderViewOptions.SetShowBookmarks( false );         // bookmarks
    aRenderViewOptions.SetLineBreak( false );             // breaks (type 1)
    aRenderViewOptions.SetPageBreak( false );             // breaks (type 2)
    aRenderViewOptions.SetColumnBreak( false );           // breaks (type 3)
    bool bVal = pPrtOptions && pPrtOptions->m_bPrintHiddenText;
    if (bContainsHiddenChars)
        aRenderViewOptions.SetShowHiddenChar( bVal );     // hidden text
    if (bContainsHiddenFields)
        aRenderViewOptions.SetShowHiddenField( bVal );
    if (bContainsHiddenParagraphs)
        aRenderViewOptions.SetShowHiddenPara( bVal );

    if (bContainsPlaceHolders)
    {
        // should always be printed in PDF export!
        bVal = !pPrtOptions ? setShowPlaceHoldersInPDF : pPrtOptions->m_bPrintTextPlaceholder;
        aRenderViewOptions.SetShowPlaceHolderFields( bVal );
    }

    if (bContainsFields)
        aRenderViewOptions.SetFieldName( false );

    // we need to set this flag in order to get to see the visible effect of
    // some of the above settings (needed for correct rendering)
    aRenderViewOptions.SetViewMetaChars( true );

    if (m_aOldViewOptions != aRenderViewOptions)  // check if reformatting is necessary
    {
        aRenderViewOptions.SetPrinting( pPrtOptions != nullptr );
        m_pShell->ApplyViewOptions( aRenderViewOptions );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
