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

#include <boost/property_tree/json_parser.hpp>

#include <sal/config.h>

#include <comphelper/string.hxx>
#include <AnnotationWin.hxx>
#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/print.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/printer.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/lokcharthelper.hxx>
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
#include <svl/numuno.hxx>
#include <fldbas.hxx>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unosett.hxx>
#include <unocoll.hxx>
#include <unoredlines.hxx>
#include <unosrch.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objsh.hxx>
#include <unoprnms.hxx>
#include <unostyle.hxx>
#include <unodraw.hxx>
#include <svl/eitem.hxx>
#include <pagedesc.hxx>
#include <unotools/datetime.hxx>
#include <unotools/textsearch.hxx>
#include <unocrsr.hxx>
#include <unofieldcoll.hxx>
#include <unoidxcoll.hxx>
#include <unocrsrhelper.hxx>
#include <unotxvw.hxx>
#include <poolfmt.hxx>
#include <globdoc.hxx>
#include <viewopt.hxx>
#include <unochart.hxx>
#include <charatr.hxx>
#include <svx/xmleohlp.hxx>
#include <globals.hrc>
#include <unomid.h>
#include <unotools/printwarningoptions.hxx>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/document/RedlineDisplayType.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <sfx2/linkmgr.hxx>
#include <svx/unofill.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/progress.hxx>
#include <swmodule.hxx>
#include <docstat.hxx>
#include <modcfg.hxx>
#include <ndtxt.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <swcont.hxx>
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
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svl/zforlist.hxx>
#include <drawdoc.hxx>
#include <SwStyleNameMapper.hxx>
#include <osl/file.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/saveopt.hxx>
#include <swruler.hxx>


#include <EnhancedPDFExportHelper.hxx>
#include <numrule.hxx>

#include <editeng/langitem.hxx>
#include <docary.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>

#include <format.hxx>
#include <charfmt.hxx>
#include <fmtcol.hxx>
#include <istyleaccess.hxx>

#include <svl/stylepool.hxx>
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
#include <svl/languageoptions.hxx>
#include <svx/svdview.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <SwSpellDialogChildWindow.hxx>
#include <memory>

#define TWIPS_PER_PIXEL 15

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using ::osl::FileBase;

static SwPrintUIOptions * lcl_GetPrintUIOptions(
    SwDocShell * pDocShell,
    const SfxViewShell * pView )
{
    if (!pDocShell)
        return nullptr;

    const bool bWebDoc      = nullptr != dynamic_cast< const SwWebDocShell * >(pDocShell);
    const bool bSwSrcView   = nullptr != dynamic_cast< const SwSrcView * >(pView);
    const SwView * pSwView = dynamic_cast< const SwView * >(pView);
    const bool bHasSelection    = pSwView && pSwView->HasSelection( false );  // check for any selection, not just text selection
    const bool bHasPostIts      = sw_GetPostIts( &pDocShell->GetDoc()->getIDocumentFieldsAccess(), nullptr );

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
    return new SwPrintUIOptions( nCurrentPage, bWebDoc, bSwSrcView, bHasSelection, bHasPostIts, rPrintData );
}

static SwTextFormatColl *lcl_GetParaStyle(const OUString& rCollName, SwDoc* pDoc)
{
    SwTextFormatColl* pColl = pDoc->FindTextFormatCollByName( rCollName );
    if( !pColl )
    {
        const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(
            rCollName, SwGetPoolIdFromName::TxtColl );
        if( USHRT_MAX != nId )
            pColl = pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nId );
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
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    ::comphelper::OInterfaceContainerHelper2 m_RefreshListeners;

    Impl() : m_RefreshListeners(m_Mutex) { }

};

namespace
{
    class theSwXTextDocumentUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextDocumentUnoTunnelId > {};
}

const Sequence< sal_Int8 > & SwXTextDocument::getUnoTunnelId()
{
    return theSwXTextDocumentUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXTextDocument::getSomething( const Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
    if( rId.getLength() == 16
        && 0 == memcmp( SfxObjectShell::getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pDocShell ));
    }

    sal_Int64 nRet = SfxBaseModel::getSomething( rId );
    if (nRet)
        return nRet;

    GetNumberFormatter();
    if (!xNumFormatAgg.is()) // may happen if not valid or no SwDoc
        return 0;
    Any aNumTunnel = xNumFormatAgg->queryAggregation(cppu::UnoType<XUnoTunnel>::get());
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
        if(xNumFormatAgg.is())
            aRet = xNumFormatAgg->queryAggregation(rType);
    }
    return aRet;
}

void SAL_CALL SwXTextDocument::acquire()throw()
{
    SfxBaseModel::acquire();
}

void SAL_CALL SwXTextDocument::release()throw()
{
    SfxBaseModel::release();
}

Reference< XAdapter > SwXTextDocument::queryAdapter(  )
{
    return SfxBaseModel::queryAdapter();
}

Sequence< uno::Type > SAL_CALL SwXTextDocument::getTypes()
{
    Sequence< uno::Type > aBaseTypes = SfxBaseModel::getTypes();
    Sequence< uno::Type > aTextTypes = SwXTextDocumentBaseClass::getTypes();

    Sequence< uno::Type > aNumTypes;
    GetNumberFormatter();
    if(xNumFormatAgg.is())
    {
        const uno::Type& rProvType = cppu::UnoType<XTypeProvider>::get();
        Any aNumProv = xNumFormatAgg->queryAggregation(rProvType);
        Reference<XTypeProvider> xNumProv;
        if(aNumProv >>= xNumProv)
        {
            aNumTypes = xNumProv->getTypes();
        }
    }
    long nIndex = aBaseTypes.getLength();
    // don't forget the lang::XMultiServiceFactory and the XTiledRenderable
    aBaseTypes.realloc(aBaseTypes.getLength() + aTextTypes.getLength() + aNumTypes.getLength() + 2);
    uno::Type* pBaseTypes = aBaseTypes.getArray();
    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos < aTextTypes.getLength(); nPos++)
    {
        pBaseTypes[nIndex++] = pTextTypes[nPos];
    }
    const uno::Type* pNumTypes = aNumTypes.getConstArray();
    for(nPos = 0; nPos < aNumTypes.getLength(); nPos++)
    {
        pBaseTypes[nIndex++] = pNumTypes[nPos];
    }
    pBaseTypes[nIndex++] = cppu::UnoType<lang::XMultiServiceFactory>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<tiledrendering::XTiledRenderable>::get();
    return aBaseTypes;
}

SwXTextDocument::SwXTextDocument(SwDocShell* pShell)
    : SfxBaseModel(pShell)
    , m_pImpl(new Impl)
    ,
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_DOCUMENT)),

    pDocShell(pShell),

    bObjectValid(pShell != nullptr),

    pDrawPage(nullptr),
    mxXDrawPage(),
    pBodyText(nullptr),
    mxXNumberingRules(),
    mxXFootnotes(),
    mxXFootnoteSettings(),
    mxXEndnotes(),
    mxXEndnoteSettings(),
    mxXReferenceMarks(),
    mxXTextFieldTypes(),
    mxXTextFieldMasters(),
    mxXTextSections(),
    mxXBookmarks(),
    mxXTextTables(),
    mxXTextFrames(),
    mxXGraphicObjects(),
    mxXEmbeddedObjects(),
    mxXStyleFamilies(),
    mxXAutoStyles(),
    mxXChapterNumbering(),
    mxXDocumentIndexes(),

    mxXLineNumberingProperties(),
    mxLinkTargetSupplier(),
    mxXRedlines(),
    m_pHiddenViewFrame(nullptr),
    m_pPrintUIOptions( nullptr ),
    m_pRenderData( nullptr ),
    // #i117783#
    bApplyPagePrintSettingsFromXPagePrintable( false )
{
}

SwXTextDocument::~SwXTextDocument()
{
    InitNewDoc();
    if(xNumFormatAgg.is())
    {
        Reference< XInterface >  x0;
        xNumFormatAgg->setDelegator(x0);
        xNumFormatAgg = nullptr;
    }
    delete m_pPrintUIOptions;
    if (m_pRenderData && m_pRenderData->IsViewOptionAdjust())
    {   // rhbz#827695: this can happen if the last page is not printed
        // the SwViewShell has been deleted already by SwView::~SwView
        // FIXME: replace this awful implementation of XRenderable with
        // something less insane that has its own view
        m_pRenderData->ViewOptionAdjustCrashPreventionKludge();
    }
    delete m_pRenderData;
}

SwXDocumentPropertyHelper * SwXTextDocument::GetPropertyHelper ()
{
    if(!mxPropertyHelper.is())
    {
        mxPropertyHelper = new SwXDocumentPropertyHelper(*pDocShell->GetDoc());
    }
    return mxPropertyHelper.get();
}

void SwXTextDocument::GetNumberFormatter()
{
    if(IsValid())
    {
        if(!xNumFormatAgg.is())
        {
            if ( pDocShell->GetDoc() )
            {
                SvNumberFormatsSupplierObj* pNumFormat = new SvNumberFormatsSupplierObj(
                                    pDocShell->GetDoc()->GetNumberFormatter());
                Reference< util::XNumberFormatsSupplier >  xTmp = pNumFormat;
                xNumFormatAgg.set(xTmp, UNO_QUERY);
            }
            if(xNumFormatAgg.is())
                xNumFormatAgg->setDelegator(static_cast<cppu::OWeakObject*>(static_cast<SwXTextDocumentBaseClass*>(this)));
        }
        else
        {
            const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
            Any aNumTunnel = xNumFormatAgg->queryAggregation(rTunnelType);
            SvNumberFormatsSupplierObj* pNumFormat = nullptr;
            Reference< XUnoTunnel > xNumTunnel;
            if(aNumTunnel >>= xNumTunnel)
            {
                pNumFormat = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                        xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));

            }
            OSL_ENSURE(pNumFormat, "No number formatter available");
            if (pNumFormat && !pNumFormat->GetNumberFormatter())
                pNumFormat->SetNumberFormatter(pDocShell->GetDoc()->GetNumberFormatter());
        }
    }
}

Reference< XText >  SwXTextDocument::getText()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!xBodyText.is())
    {
        pBodyText = new SwXBodyText(pDocShell->GetDoc());
        xBodyText = pBodyText;
    }
    return xBodyText;
}

void SwXTextDocument::reformat()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
}

void SwXTextDocument::lockControllers()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    UnoActionContext* pContext = new UnoActionContext(pDocShell->GetDoc());
    aActionArr.push_front(pContext);
}

void SwXTextDocument::unlockControllers()
{
    SolarMutexGuard aGuard;
    if(aActionArr.empty())
        throw RuntimeException("Nothing to unlock");

    UnoActionContext* pContext = aActionArr.front();
    aActionArr.pop_front();
    delete pContext;
}

sal_Bool SwXTextDocument::hasControllersLocked()
{
    SolarMutexGuard aGuard;
    return !aActionArr.empty();
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
    if(IsValid())
    {
        SwView* pView = static_cast<SwView*>(SfxViewShell::GetFirst(true, checkSfxViewShell<SwView>));
        while(pView && pView->GetObjectShell() != pDocShell)
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
    SfxBaseModel::dispose();
}

void SwXTextDocument::close( sal_Bool bDeliverOwnership )
{
    SolarMutexGuard aGuard;
    if(IsValid() && m_pHiddenViewFrame)
        lcl_DisposeView( m_pHiddenViewFrame, pDocShell);
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
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    if(!mxXLineNumberingProperties.is())
    {
        mxXLineNumberingProperties = new SwXLineNumberingProperties(pDocShell->GetDoc());
    }
    return mxXLineNumberingProperties;
}

Reference< XIndexReplace >  SwXTextDocument::getChapterNumberingRules()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXChapterNumbering.is())
    {
        mxXChapterNumbering = new SwXChapterNumbering(*pDocShell);
    }
    return mxXChapterNumbering;
}

Reference< XIndexAccess >  SwXTextDocument::getNumberingRules()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXNumberingRules.is() )
    {
        mxXNumberingRules = new SwXNumberingRulesCollection( pDocShell->GetDoc() );
    }
    return mxXNumberingRules;
}

Reference< XIndexAccess >  SwXTextDocument::getFootnotes()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXFootnotes.is())
    {
        mxXFootnotes = new SwXFootnotes(false, pDocShell->GetDoc());
    }
    return mxXFootnotes;
}

Reference< XPropertySet >  SAL_CALL
        SwXTextDocument::getFootnoteSettings()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXFootnoteSettings.is())
    {
        mxXFootnoteSettings = new SwXFootnoteProperties(pDocShell->GetDoc());
    }
    return mxXFootnoteSettings;
}

Reference< XIndexAccess >  SwXTextDocument::getEndnotes()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXEndnotes.is())
    {
        mxXEndnotes = new SwXFootnotes(true, pDocShell->GetDoc());
    }
    return mxXEndnotes;
}

Reference< XPropertySet >  SwXTextDocument::getEndnoteSettings()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXEndnoteSettings.is())
    {
        mxXEndnoteSettings = new SwXEndnoteProperties(pDocShell->GetDoc());
    }
    return mxXEndnoteSettings;
}

Reference< util::XReplaceDescriptor >  SwXTextDocument::createReplaceDescriptor()
{
    SolarMutexGuard aGuard;
    Reference< util::XReplaceDescriptor >  xRet = new SwXTextSearch;
    return xRet;
}

SwUnoCursor* SwXTextDocument::CreateCursorForSearch(Reference< XTextCursor > & xCursor)
{
    getText();
    XText *const pText = xBodyText.get();
    SwXBodyText* pBText = static_cast<SwXBodyText*>(pText);
    SwXTextCursor *const pXTextCursor = pBText->CreateTextCursor(true);
    xCursor.set( static_cast<text::XWordCursor*>(pXTextCursor) );

    auto& rUnoCursor(pXTextCursor->GetCursor());
    rUnoCursor.SetRemainInSection(false);
    return &rUnoCursor;
}

sal_Int32 SwXTextDocument::replaceAll(const Reference< util::XSearchDescriptor > & xDesc)
{
    SolarMutexGuard aGuard;
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY_THROW);
    if(!IsValid() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        throw DisposedException("", static_cast< XTextDocument* >(this));

    Reference< XTextCursor >  xCursor;
    auto pUnoCursor(CreateCursorForSearch(xCursor));

    const SwXTextSearch* pSearch = reinterpret_cast<const SwXTextSearch*>(
            xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()));

    int eRanges(FindRanges::InBody|FindRanges::InSelAll);

    i18nutil::SearchOptions2 aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

    SwDocPositions eStart = pSearch->m_bBack ? SwDocPositions::End : SwDocPositions::Start;
    SwDocPositions eEnd = pSearch->m_bBack ? SwDocPositions::Start : SwDocPositions::End;

    // Search should take place anywhere
    pUnoCursor->SetRemainInSection(false);
    sal_uInt32 nResult;
    UnoActionContext aContext(pDocShell->GetDoc());
    //try attribute search first
    if(pSearch->HasSearchAttributes()||pSearch->HasReplaceAttributes())
    {
        SfxItemSet aSearch(pDocShell->GetDoc()->GetAttrPool(),
                            svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1>{});
        SfxItemSet aReplace(pDocShell->GetDoc()->GetAttrPool(),
                            svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1>{});
        pSearch->FillSearchItemSet(aSearch);
        pSearch->FillReplaceItemSet(aReplace);
        bool bCancel;
        nResult = (sal_Int32)pUnoCursor->Find( aSearch, !pSearch->m_bStyles,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges,
                    !pSearch->m_sSearchText.isEmpty() ? &aSearchOpt : nullptr,
                    &aReplace );
    }
    else if(pSearch->m_bStyles)
    {
        SwTextFormatColl *pSearchColl = lcl_GetParaStyle(pSearch->m_sSearchText, pUnoCursor->GetDoc());
        SwTextFormatColl *pReplaceColl = lcl_GetParaStyle(pSearch->m_sReplaceText, pUnoCursor->GetDoc());

        bool bCancel;
        nResult = pUnoCursor->Find( *pSearchColl,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges, pReplaceColl );

    }
    else
    {
        //todo/mba: assuming that notes should be omitted
        bool bCancel;
        nResult = pUnoCursor->Find( aSearchOpt, false/*bSearchInNotes*/,
            eStart, eEnd, bCancel,
            (FindRanges)eRanges,
            true );
    }
    return (sal_Int32)nResult;

}

Reference< util::XSearchDescriptor >  SwXTextDocument::createSearchDescriptor()
{
    SolarMutexGuard aGuard;
    Reference< util::XSearchDescriptor >  xRet = new SwXTextSearch;
    return xRet;

}

// Used for findAll/First/Next

SwUnoCursor* SwXTextDocument::FindAny(const Reference< util::XSearchDescriptor > & xDesc,
                                     Reference< XTextCursor > & xCursor,
                                     bool bAll,
                                     sal_Int32& nResult,
                                     Reference< XInterface > const & xLastResult)
{
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY);
    if(!IsValid() || !xDescTunnel.is() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        return nullptr;

    auto pUnoCursor(CreateCursorForSearch(xCursor));
    const SwXTextSearch* pSearch = reinterpret_cast<const SwXTextSearch*>(
        xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()));

    bool bParentInExtra = false;
    if(xLastResult.is())
    {
        Reference<XUnoTunnel> xCursorTunnel( xLastResult, UNO_QUERY);
        OTextCursorHelper* pPosCursor = nullptr;
        if(xCursorTunnel.is())
        {
            pPosCursor = reinterpret_cast<OTextCursorHelper*>(xCursorTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
        }
        SwPaM* pCursor = pPosCursor ? pPosCursor->GetPaM() : nullptr;
        if(pCursor)
        {
            *pUnoCursor->GetPoint() = *pCursor->End();
            pUnoCursor->DeleteMark();
        }
        else
        {
            SwXTextRange* pRange = nullptr;
            if(xCursorTunnel.is())
            {
                pRange = reinterpret_cast<SwXTextRange*>(xCursorTunnel->getSomething(
                                        SwXTextRange::getUnoTunnelId()));
            }
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
        const SwNode& rRangeNode = pUnoCursor->GetNode();
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
            SfxItemSet aSearch(
                pDocShell->GetDoc()->GetAttrPool(),
                svl::Items<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_PARATR_BEGIN, RES_PARATR_END - 1,
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1>{});
            pSearch->FillSearchItemSet(aSearch);
            bool bCancel;
            nResult = (sal_Int32)pUnoCursor->Find( aSearch, !pSearch->m_bStyles,
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
            nResult = (sal_Int32)pUnoCursor->Find( *pSearchColl,
                        eStart, eEnd, bCancel,
                        eRanges, pReplaceColl );
        }
        else
        {
            //todo/mba: assuming that notes should be omitted
            bool bCancel;
            nResult = (sal_Int32)pUnoCursor->Find( aSearchOpt, false/*bSearchInNotes*/,
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
        throw RuntimeException("No result cursor");
    Reference< XIndexAccess >  xRet;
    xRet = SwXTextRanges::Create( nResult ? &(*pResultCursor) : nullptr );
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
        throw RuntimeException("No result cursor");
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(*pDocShell->GetDoc(),
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
        throw RuntimeException("xStartAt missing");
    auto pResultCursor(FindAny(xDesc, xCursor, false, nResult, xStartAt));
    if(!pResultCursor)
        throw RuntimeException("No result cursor");
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(*pDocShell->GetDoc(),
                    *pResultCursor->GetPoint());

        xRet = *new SwXTextCursor(xParent, *pResultCursor);
    }
    return xRet;
}

Sequence< beans::PropertyValue > SwXTextDocument::getPagePrintSettings()
{
    SolarMutexGuard aGuard;
    Sequence< beans::PropertyValue > aSeq(9);
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    beans::PropertyValue* pArray = aSeq.getArray();
    SwPagePreviewPrtData aData;
    const SwPagePreviewPrtData* pData = pDocShell->GetDoc()->GetPreviewPrtData();
    if(pData)
        aData = *pData;
    Any aVal;
    aVal <<= (sal_Int16)aData.GetRow();
    pArray[0] = beans::PropertyValue("PageRows", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int16)aData.GetCol();
    pArray[1] = beans::PropertyValue("PageColumns", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetLeftSpace());
    pArray[2] = beans::PropertyValue("LeftMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetRightSpace());
    pArray[3] = beans::PropertyValue("RightMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetTopSpace());
    pArray[4] = beans::PropertyValue("TopMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetBottomSpace());
    pArray[5] = beans::PropertyValue("BottomMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetHorzSpace());
    pArray[6] = beans::PropertyValue("HoriMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= (sal_Int32)convertTwipToMm100(aData.GetVertSpace());
    pArray[7] = beans::PropertyValue("VertMargin", -1, aVal, PropertyState_DIRECT_VALUE);
    aVal <<= aData.GetLandscape();
    pArray[8] = beans::PropertyValue("IsLandscape", -1, aVal, PropertyState_DIRECT_VALUE);

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
            nRet = (sal_uInt32)nVal;
    }

    return nRet;
}

static OUString lcl_CreateOutlineString( size_t nIndex,
            const SwOutlineNodes& rOutlineNodes, const SwNumRule* pOutlRule)
{
    OUString sEntry;
    const SwTextNode * pTextNd = rOutlineNodes[ nIndex ]->GetTextNode();
    SwNumberTree::tNumberVector aNumVector = pTextNd->GetNumberVector();
    if( pOutlRule && pTextNd->GetNumRule())
        for( int nLevel = 0;
             nLevel <= pTextNd->GetActualListLevel();
             nLevel++ )
        {
            long nVal = aNumVector[nLevel];
            nVal ++;
            nVal -= pOutlRule->Get(nLevel).GetStart();
            sEntry += OUString::number( nVal );
            sEntry += ".";
        }
    sEntry += rOutlineNodes[ nIndex ]->
                    GetTextNode()->GetExpandText();
    return sEntry;
}

void SwXTextDocument::setPagePrintSettings(const Sequence< beans::PropertyValue >& aSettings)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    SwPagePreviewPrtData aData;
    //if only a few properties are coming, then use the current settings
    const SwPagePreviewPrtData* pData = pDocShell->GetDoc()->GetPreviewPrtData();
    if(pData)
        aData = *pData;
    const beans::PropertyValue* pProperties = aSettings.getConstArray();
    int nCount = aSettings.getLength();
    for(int i = 0; i < nCount; i++)
    {
        OUString sName = pProperties[i].Name;
        const Any& rVal = pProperties[i].Value;
        bool bException;
        sal_uInt32 nVal = lcl_Any_To_ULONG(rVal, bException);
        if( sName == "PageRows" )
        {
            if(!nVal || nVal > 0xff)
                throw RuntimeException("Invalid value");
            aData.SetRow((sal_uInt8)nVal);
        }
        else if(sName == "PageColumns")
        {
            if(!nVal  || nVal > 0xff)
                throw RuntimeException("Invalid value");
            aData.SetCol((sal_uInt8)nVal);
        }
        else if(sName == "LeftMargin")
        {
            aData.SetLeftSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "RightMargin")
        {
            aData.SetRightSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "TopMargin")
        {
            aData.SetTopSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "BottomMargin")
        {
            aData.SetBottomSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "HoriMargin")
        {
            aData.SetHorzSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "VertMargin")
        {
            aData.SetVertSpace(convertMm100ToTwip(nVal));
        }
        else if(sName == "IsLandscape")
        {
            auto b = o3tl::tryAccess<bool>(rVal);
            bException = !b;
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
    pDocShell->GetDoc()->SetPreviewPrtData(&aData);

}

void SwXTextDocument::printPages(const Sequence< beans::PropertyValue >& xOptions)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    SfxViewFrame* pFrame = SfxViewFrame::LoadHiddenDocument( *pDocShell, SfxInterfaceId(7) );
    SfxRequest aReq(FN_PRINT_PAGEPREVIEW, SfxCallMode::SYNCHRON,
                                pDocShell->GetDoc()->GetAttrPool());
        aReq.AppendItem(SfxBoolItem(FN_PRINT_PAGEPREVIEW, true));

    for ( int n = 0; n < xOptions.getLength(); ++n )
    {
        // get Property-Value from options
        const beans::PropertyValue &rProp = xOptions.getConstArray()[n];
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
            aReq.AppendItem(SfxInt16Item( SID_PRINT_COPIES, (sal_Int16)nCopies ) );
        }

        // Collate-Property
        else if ( rProp.Name == UNO_NAME_COLLATE )
        {
            auto b = o3tl::tryAccess<bool>(rProp.Value);
            if ( !b )
                throw IllegalArgumentException();
            aReq.AppendItem(SfxBoolItem( SID_PRINT_COLLATE, *b ) );

        }

        // Sort-Property
        else if ( rProp.Name == UNO_NAME_SORT )
        {
            auto b = o3tl::tryAccess<bool>(rProp.Value);
            if ( !b )
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
    bApplyPagePrintSettingsFromXPagePrintable = true;
    pFrame->GetViewShell()->ExecuteSlot(aReq);
    // Frame close
    pFrame->DoClose();

}

Reference< XNameAccess >  SwXTextDocument::getReferenceMarks()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXReferenceMarks.is())
    {
        mxXReferenceMarks = new SwXReferenceMarks(pDocShell->GetDoc());
    }
    return mxXReferenceMarks;
}

Reference< XEnumerationAccess >  SwXTextDocument::getTextFields()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXTextFieldTypes.is())
    {
        mxXTextFieldTypes = new SwXTextFieldTypes(pDocShell->GetDoc());
    }
    return mxXTextFieldTypes;
}

Reference< XNameAccess >  SwXTextDocument::getTextFieldMasters()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXTextFieldMasters.is())
    {
        mxXTextFieldMasters = new SwXTextFieldMasters(pDocShell->GetDoc());
    }
    return mxXTextFieldMasters;
}

Reference< XNameAccess >  SwXTextDocument::getEmbeddedObjects()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXEmbeddedObjects.is())
    {
        mxXEmbeddedObjects = new SwXTextEmbeddedObjects(pDocShell->GetDoc());
    }
    return mxXEmbeddedObjects;
}

Reference< XNameAccess >  SwXTextDocument::getBookmarks()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXBookmarks.is())
    {
        mxXBookmarks = new SwXBookmarks(pDocShell->GetDoc());
    }
    return mxXBookmarks;
}

Reference< XNameAccess >  SwXTextDocument::getTextSections()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXTextSections.is())
    {
        mxXTextSections = new SwXTextSections(pDocShell->GetDoc());
    }
    return mxXTextSections;
}

Reference< XNameAccess >  SwXTextDocument::getTextTables()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXTextTables.is())
    {
        mxXTextTables = new SwXTextTables(pDocShell->GetDoc());
    }
    return mxXTextTables;
}

Reference< XNameAccess >  SwXTextDocument::getGraphicObjects()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXGraphicObjects.is())
    {
        mxXGraphicObjects = new SwXTextGraphicObjects(pDocShell->GetDoc());
    }
    return mxXGraphicObjects;
}

Reference< XNameAccess >  SwXTextDocument::getTextFrames()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXTextFrames.is())
    {
        mxXTextFrames = new SwXTextFrames(pDocShell->GetDoc());
    }
    return mxXTextFrames;
}

Reference< XNameAccess >  SwXTextDocument::getStyleFamilies()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXStyleFamilies.is())
    {
        mxXStyleFamilies = new SwXStyleFamilies(*pDocShell);
    }
    return mxXStyleFamilies;
}

uno::Reference< style::XAutoStyles > SwXTextDocument::getAutoStyles(  )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXAutoStyles.is())
    {
        mxXAutoStyles = new SwXAutoStyles(*pDocShell);
    }
    return mxXAutoStyles;

}

Reference< drawing::XDrawPage >  SwXTextDocument::getDrawPage()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));
    if(!mxXDrawPage.is())
    {
        pDrawPage = new SwXDrawPage(pDocShell->GetDoc());
        mxXDrawPage = pDrawPage;
        // Create a Reference to trigger the complete initialization of the
        // object. Otherwise in some corner cases it would get initialized
        // at ::InitNewDoc -> which would get called during
        // close() or dispose() -> n#681746
        uno::Reference<lang::XComponent> xComp( mxXDrawPage, uno::UNO_QUERY );
    }
    return mxXDrawPage;
}

void SwXTextDocument::Invalidate()
{
    bObjectValid = false;
    if(xNumFormatAgg.is())
    {
        const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
        Any aNumTunnel = xNumFormatAgg->queryAggregation(rTunnelType);
        SvNumberFormatsSupplierObj* pNumFormat = nullptr;
        Reference< XUnoTunnel > xNumTunnel;
        if(aNumTunnel >>= xNumTunnel)
        {
            pNumFormat = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));
            pNumFormat->SetNumberFormatter(nullptr);
        }
        OSL_ENSURE(pNumFormat, "No number formatter available");
    }
    InitNewDoc();
    pDocShell = nullptr;
    lang::EventObject const ev(static_cast<SwXTextDocumentBaseClass &>(*this));
    m_pImpl->m_RefreshListeners.disposeAndClear(ev);
}

void SwXTextDocument::Reactivate(SwDocShell* pNewDocShell)
{
    if(pDocShell && pDocShell != pNewDocShell)
        Invalidate();
    pDocShell = pNewDocShell;
    bObjectValid = true;
}

void    SwXTextDocument::InitNewDoc()
{
    // first invalidate all collections, then delete references and Set to zero
    if(mxXTextTables.is())
    {
        XNameAccess* pTables = mxXTextTables.get();
        static_cast<SwXTextTables*>(pTables)->Invalidate();
        mxXTextTables.clear();
    }

    if(mxXTextFrames.is())
    {
        XNameAccess* pFrames = mxXTextFrames.get();
        static_cast<SwXTextFrames*>(pFrames)->Invalidate();
        mxXTextFrames.clear();
    }

    if(mxXGraphicObjects.is())
    {
        XNameAccess* pFrames = mxXGraphicObjects.get();
        static_cast<SwXTextGraphicObjects*>(pFrames)->Invalidate();
        mxXGraphicObjects.clear();
    }

    if(mxXEmbeddedObjects.is())
    {
        XNameAccess* pOLE = mxXEmbeddedObjects.get();
        static_cast<SwXTextEmbeddedObjects*>(pOLE)->Invalidate();
        mxXEmbeddedObjects.clear();
    }

    if(xBodyText.is())
    {
        xBodyText = nullptr;
        pBodyText = nullptr;
    }

    if(xNumFormatAgg.is())
    {
        const uno::Type& rTunnelType = cppu::UnoType<XUnoTunnel>::get();
        Any aNumTunnel = xNumFormatAgg->queryAggregation(rTunnelType);
        SvNumberFormatsSupplierObj* pNumFormat = nullptr;
        Reference< XUnoTunnel > xNumTunnel;
        if(aNumTunnel >>= xNumTunnel)
        {
            pNumFormat = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));

        }
        OSL_ENSURE(pNumFormat, "No number formatter available");
        if (pNumFormat)
            pNumFormat->SetNumberFormatter(nullptr);
    }

    if(mxXTextFieldTypes.is())
    {
        XEnumerationAccess* pT = mxXTextFieldTypes.get();
        static_cast<SwXTextFieldTypes*>(pT)->Invalidate();
        mxXTextFieldTypes.clear();
    }

    if(mxXTextFieldMasters.is())
    {
        XNameAccess* pT = mxXTextFieldMasters.get();
        static_cast<SwXTextFieldMasters*>(pT)->Invalidate();
        mxXTextFieldMasters.clear();
    }

    if(mxXTextSections.is())
    {
        XNameAccess* pSect = mxXTextSections.get();
        static_cast<SwXTextSections*>(pSect)->Invalidate();
        mxXTextSections.clear();
    }

    if(mxXDrawPage.is())
    {
        // #i91798#, #i91895#
        // dispose XDrawPage here. We are the owner and know that it is no longer in a valid condition.
        uno::Reference<lang::XComponent> xComp( mxXDrawPage, uno::UNO_QUERY );
        xComp->dispose();
        pDrawPage->InvalidateSwDoc();
        mxXDrawPage.clear();
    }

    if ( mxXNumberingRules.is() )
    {
        XIndexAccess* pNum = mxXNumberingRules.get();
        static_cast<SwXNumberingRulesCollection*>(pNum)->Invalidate();
        mxXNumberingRules.clear();
    }

    if(mxXFootnotes.is())
    {
        XIndexAccess* pFootnote = mxXFootnotes.get();
        static_cast<SwXFootnotes*>(pFootnote)->Invalidate();
        mxXFootnotes.clear();
    }

    if(mxXEndnotes.is())
    {
        XIndexAccess* pFootnote = mxXEndnotes.get();
        static_cast<SwXFootnotes*>(pFootnote)->Invalidate();
        mxXEndnotes.clear();
    }

    if(mxXDocumentIndexes.is())
    {
        XIndexAccess* pIdxs = mxXDocumentIndexes.get();
        static_cast<SwXDocumentIndexes*>(pIdxs)->Invalidate();
        mxXDocumentIndexes.clear();
    }

    if(mxXStyleFamilies.is())
    {
        XNameAccess* pStyles = mxXStyleFamilies.get();
        static_cast<SwXStyleFamilies*>(pStyles)->Invalidate();
        mxXStyleFamilies.clear();
    }
    if(mxXAutoStyles.is())
    {
        XNameAccess* pStyles = mxXAutoStyles.get();
        static_cast<SwXAutoStyles*>(pStyles)->Invalidate();
        mxXAutoStyles.clear();
    }

    if(mxXBookmarks.is())
    {
        XNameAccess* pBm = mxXBookmarks.get();
        static_cast<SwXBookmarks*>(pBm)->Invalidate();
        mxXBookmarks.clear();
    }

    if(mxXChapterNumbering.is())
    {
        XIndexReplace* pCh = mxXChapterNumbering.get();
        static_cast<SwXChapterNumbering*>(pCh)->Invalidate();
        mxXChapterNumbering.clear();
    }

    if(mxXFootnoteSettings.is())
    {
        XPropertySet* pFntSet = mxXFootnoteSettings.get();
        static_cast<SwXFootnoteProperties*>(pFntSet)->Invalidate();
        mxXFootnoteSettings.clear();
    }

    if(mxXEndnoteSettings.is())
    {
        XPropertySet* pEndSet = mxXEndnoteSettings.get();
        static_cast<SwXEndnoteProperties*>(pEndSet)->Invalidate();
        mxXEndnoteSettings.clear();
    }

    if(mxXLineNumberingProperties.is())
    {
        XPropertySet* pLine = mxXLineNumberingProperties.get();
        static_cast<SwXLineNumberingProperties*>(pLine)->Invalidate();
        mxXLineNumberingProperties.clear();
    }
    if(mxXReferenceMarks.is())
    {
        XNameAccess* pMarks = mxXReferenceMarks.get();
        static_cast<SwXReferenceMarks*>(pMarks)->Invalidate();
        mxXReferenceMarks.clear();
    }
    if(mxLinkTargetSupplier.is())
    {
        XNameAccess* pAccess = mxLinkTargetSupplier.get();
        static_cast<SwXLinkTargetSupplier*>(pAccess)->Invalidate();
        mxLinkTargetSupplier.clear();
    }
    if(mxXRedlines.is())
    {
        XEnumerationAccess* pMarks = mxXRedlines.get();
        static_cast<SwXRedlines*>(pMarks)->Invalidate();
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
    if (!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SwServiceType nType = SwXServiceProvider::GetProviderType(rServiceName);
    if (nType != SwServiceType::Invalid)
    {
        return SwXServiceProvider::MakeInstance(nType, *pDocShell->GetDoc());
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
        return static_cast<cppu::OWeakObject *>(
            new SvXMLEmbeddedObjectHelper(
                *pDocShell, SvXMLEmbeddedObjectHelperMode::Read));
    }
    if (rServiceName == "com.sun.star.text.DocumentSettings")
    {
        return Reference<XInterface>(*new SwXDocumentSettings(this));
    }
    if (rServiceName == "com.sun.star.chart2.data.DataProvider")
    {
        return Reference<XInterface>(
            dynamic_cast<chart2::data::XDataProvider *>(
                pDocShell->getIDocumentChartDataProviderAccess().
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
        return *new SwXGroupShape(xTmp, pDocShell->GetDoc());
    }
    if (rServiceName.startsWith("com.sun.star.drawing."))
    {
        return *new SwXShape(xTmp, pDocShell->GetDoc());
    }
    return xTmp;
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
    if ( aServices.getLength() == 0 )
    {
        Sequence< OUString > aRet =  SvxFmMSFactory::getAvailableServiceNames();
        OUString* pRet = aRet.getArray();
        for ( sal_Int32 i = 0; i < aRet.getLength(); ++i )
        {
            if ( pRet[i] == "com.sun.star.drawing.OLE2Shape" )
            {
                pRet[i] = pRet[aRet.getLength() - 1];
                aRet.realloc( aRet.getLength() - 1 ); // <pRet> no longer valid.
                break;
            }
        }
        Sequence< OUString > aOwn = SwXServiceProvider::GetAllServiceNames();
        aServices = SvxFmMSFactory::concatServiceNames(aRet, aOwn);
    }

    return aServices;
}

OUString SwXTextDocument::getImplementationName()
{
    return OUString("SwXTextDocument");
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
    bool bWebDoc    = (dynamic_cast<SwWebDocShell*>(    pDocShell) !=  nullptr );
    bool bGlobalDoc = (dynamic_cast<SwGlobalDocShell*>( pDocShell) !=  nullptr );
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
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    if(!mxXDocumentIndexes.is())
    {
        mxXDocumentIndexes = new SwXDocumentIndexes(pDocShell->GetDoc());
    }
    return mxXDocumentIndexes;
}

Reference< XPropertySetInfo >  SwXTextDocument::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXTextDocument::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw UnknownPropertyException();
    if(pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    switch(pEntry->nWID)
    {
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
            throw RuntimeException(
                "bad WID",
                static_cast< cppu::OWeakObject * >(
                    static_cast< SwXTextDocumentBaseClass * >(this)));
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
            bool bSet = *o3tl::doAccess<bool>(aValue);
            RedlineFlags eMode = pDocShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
            if(WID_DOC_CHANGES_SHOW == pEntry->nWID)
            {
                eMode &= ~RedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete);
                eMode |= RedlineFlags::ShowInsert;
                if( bSet )
                    eMode |= RedlineFlags::ShowDelete;
            }
            else if(WID_DOC_CHANGES_RECORD == pEntry->nWID)
            {
                eMode = bSet ? eMode|RedlineFlags::On : eMode&~RedlineFlags::On;
            }
            pDocShell->GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eMode );
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(aValue >>= aNew)
            {
                SwDoc* pDoc = pDocShell->GetDoc();
                pDoc->getIDocumentRedlineAccess().SetRedlinePassword(aNew);
                if(aNew.getLength())
                {
                    RedlineFlags eMode = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
                    eMode |= RedlineFlags::On;
                    pDoc->getIDocumentRedlineAccess().SetRedlineFlags( eMode );
                }
            }
        }
        break;
        case WID_DOC_AUTO_MARK_URL :
        {
            OUString sURL;
            aValue >>= sURL;
            pDocShell->GetDoc()->SetTOIAutoMarkURL(sURL);
        }
        break;
        case WID_DOC_HIDE_TIPS :
            SW_MOD()->GetModuleConfig()->SetHideFieldTips(*o3tl::doAccess<bool>(aValue));
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            RedlineFlags eRedMode = pDocShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
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
            pDocShell->GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags(eRedMode);
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            sal_Int16 nYear = 0;
            aValue >>= nYear;
            SfxRequest aRequest ( SID_ATTR_YEAR2000, SfxCallMode::SLOT, pDocShell->GetDoc()->GetAttrPool());
            aRequest.AppendItem(SfxUInt16Item( SID_ATTR_YEAR2000, static_cast < sal_uInt16 > ( nYear ) ) );
            pDocShell->Execute ( aRequest );
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawModel * pDrawDoc;
            bool bAuto = *o3tl::doAccess<bool>(aValue);

            if ( nullptr != ( pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel() ) )
                pDrawDoc->SetAutoControlFocus( bAuto );
            else if (bAuto)
            {
                // if setting to true, and we don't have an
                // SdrModel, then we are changing the default and
                // must thus create an SdrModel, if we don't have an
                // SdrModel and we are leaving the default at false,
                // we don't need to make an SdrModel and can do nothing
                // #i52858# - method name changed
                pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
                pDrawDoc->SetAutoControlFocus ( bAuto );
            }
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawModel * pDrawDoc;
            bool bMode = *o3tl::doAccess<bool>(aValue);

            if ( nullptr != ( pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel() ) )
                pDrawDoc->SetOpenInDesignMode( bMode );
            else if (!bMode)
            {
                // if setting to false, and we don't have an
                // SdrModel, then we are changing the default and
                // must thus create an SdrModel, if we don't have an
                // SdrModel and we are leaving the default at true,
                // we don't need to make an SdrModel and can do
                // nothing
                // #i52858# - method name changed
                pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
                pDrawDoc->SetOpenInDesignMode ( bMode );
            }
        }
        break;
        // #i42634# New property to set the bInReading
        // flag at the document, used during binary import
        case WID_DOC_LOCK_UPDATES :
        {
            SwDoc* pDoc = pDocShell->GetDoc();
            bool bBool (false);
            if( aValue >>= bBool )
              pDoc->SetInReading( bBool );
        }
        break;
        case WID_DOC_BUILDID:
            aValue >>= maBuildId;
        break;

        case WID_DOC_DEFAULT_PAGE_MODE:
        {
            bool bDefaultPageMode( false );
            aValue >>= bDefaultPageMode;
            pDocShell->GetDoc()->SetDefaultPageMode( bDefaultPageMode );
        }
        break;
        case WID_DOC_INTEROP_GRAB_BAG:
             setGrabBagItem(aValue);
        break;

        default:
        {
            const SfxPoolItem& rItem = pDocShell->GetDoc()->GetDefault(pEntry->nWID);
            std::unique_ptr<SfxPoolItem> pNewItem(rItem.Clone());
            pNewItem->PutValue(aValue, pEntry->nMemberId);
            pDocShell->GetDoc()->SetDefault(*pNewItem);
        }
    }
}

Any SwXTextDocument::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw UnknownPropertyException();
    Any aAny;
    switch(pEntry->nWID)
    {
        case WID_DOC_ISTEMPLATEID    :
            aAny <<= pDocShell->IsTemplate();
            break;
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
        {
            const SwDocStat& rStat(pDocShell->GetDoc()->getIDocumentStatistics().GetUpdatedDocStat( false, true ));
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
            const RedlineFlags eMode = pDocShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
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
        {
            SwDoc* pDoc = pDocShell->GetDoc();
            aAny <<= pDoc->getIDocumentRedlineAccess().GetRedlinePassword();
        }
        break;
        case WID_DOC_AUTO_MARK_URL :
            aAny <<= pDocShell->GetDoc()->GetTOIAutoMarkURL();
        break;
        case WID_DOC_HIDE_TIPS :
            aAny <<= SW_MOD()->GetModuleConfig()->IsHideFieldTips();
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            RedlineFlags eRedMode = pDocShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
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
            Reference<XForbiddenCharacters> xRet(static_cast<cppu::OWeakObject*>(mxPropertyHelper.get()), UNO_QUERY);
            aAny <<= xRet;
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            aAny <<= static_cast < sal_Int16 > (pDocShell->GetDoc()->GetNumberFormatter ()->GetYear2000());
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawModel * pDrawDoc;
            bool bAuto;
            if ( nullptr != ( pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel() ) )
                bAuto = pDrawDoc->GetAutoControlFocus();
            else
                bAuto = false;
            aAny <<= bAuto;
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawModel * pDrawDoc;
            bool bMode;
            if ( nullptr != ( pDrawDoc = pDocShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel() ) )
                bMode = pDrawDoc->GetOpenInDesignMode();
            else
                bMode = true;
            aAny <<= bMode;
        }
        break;
        case WID_DOC_BASIC_LIBRARIES:
            aAny <<= pDocShell->GetBasicContainer();
        break;
        case WID_DOC_DIALOG_LIBRARIES:
            aAny <<= pDocShell->GetDialogContainer();
        break;
        case WID_DOC_VBA_DOCOBJ:
        {
            /* #i111553# This property provides the name of the constant that
               will be used to store this model in the global Basic manager.
               That constant will be equivalent to 'ThisComponent' but for
               each application, so e.g. a 'ThisExcelDoc' and a 'ThisWordDoc'
               constant can co-exist, as required by VBA. */
            aAny <<= OUString( "ThisWordDoc" );
        }
        break;
        case WID_DOC_RUNTIME_UID:
            aAny <<= getRuntimeUID();
        break;
        case WID_DOC_LOCK_UPDATES :
            aAny <<= pDocShell->GetDoc()->IsInReading();
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

        default:
        {
            const SfxPoolItem& rItem = pDocShell->GetDoc()->GetDefault(pEntry->nWID);
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
        mxXRedlines = new SwXRedlines(pDocShell->GetDoc());
    }
    return mxXRedlines;
}

void SwXTextDocument::NotifyRefreshListeners()
{
    // why does SwBaseShell not just call refresh? maybe because it's rSh is
    // (sometimes) a different shell than GetWrtShell()?
    lang::EventObject const ev(static_cast<SwXTextDocumentBaseClass &>(*this));
    m_pImpl->m_RefreshListeners.notifyEach(
            & util::XRefreshListener::refreshed, ev);
}

void SwXTextDocument::refresh()
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    SwViewShell *pViewShell = pDocShell->GetWrtShell();
    NotifyRefreshListeners();
    if(pViewShell)
        pViewShell->CalcLayout();
}

void SAL_CALL SwXTextDocument::addRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.addInterface(xListener);
}

void SAL_CALL SwXTextDocument::removeRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.removeInterface(xListener);
}

void SwXTextDocument::updateLinks(  )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    SwDoc* pDoc = pDocShell->GetDoc();
      sfx2::LinkManager& rLnkMan = pDoc->getIDocumentLinksAdministration().GetLinkManager();
    if( !rLnkMan.GetLinks().empty() )
    {
        UnoActionContext aAction(pDoc);
        rLnkMan.UpdateAllLinks( false, true, nullptr );
    }
}

//XPropertyState
PropertyState SAL_CALL SwXTextDocument::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException();
    return PropertyState_DIRECT_VALUE;
}

Sequence< PropertyState > SAL_CALL SwXTextDocument::getPropertyStates( const Sequence< OUString >& rPropertyNames )
{
    const sal_Int32 nCount = rPropertyNames.getLength();
    const OUString * pNames = rPropertyNames.getConstArray();
    Sequence < PropertyState > aRet ( nCount );
    PropertyState *pState = aRet.getArray();

    for ( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++)
        pState[nIndex] = getPropertyState( pNames[nIndex] );

    return aRet;
}

void SAL_CALL SwXTextDocument::setPropertyToDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException();
    switch(pEntry->nWID)
    {
        case 0:default:break;
    }
}

Any SAL_CALL SwXTextDocument::getPropertyDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException();
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

    uno::Any aAny( rPrintUIOptions.getValue( "RenderDevice" ));
    uno::Reference< awt::XDevice >  xRenderDevice;
    aAny >>= xRenderDevice;
    if (xRenderDevice.is())
    {
        VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        pOut = pDevice ? pDevice->GetOutputDevice() : VclPtr< OutputDevice >();
    }

    return pOut;
}

static bool lcl_SeqHasProperty(
    const uno::Sequence< beans::PropertyValue >& rOptions,
    const sal_Char *pPropName )
{
    bool bRes = false;
    const sal_Int32 nLen = rOptions.getLength();
    const beans::PropertyValue *pProps = rOptions.getConstArray();
    for (sal_Int32 i = 0;  i < nLen && !bRes;  ++i)
    {
        if (pProps[i].Name.equalsAscii( pPropName ))
            bRes = true;
    }
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
        const sal_Int32 nLen = rOptions.getLength();
        const beans::PropertyValue *pProps = rOptions.getConstArray();
        for (sal_Int32 i = 0; i < nLen; ++i)
        {
            if ( pProps[i].Name == "View" )
            {
                aTmp = pProps[i].Value;
                break;
            }
        }

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
    if (xModel == pDocShell->GetModel())
        pDoc = pDocShell->GetDoc();
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
            if (rpView  &&  dynamic_cast< const SwView *>( rpView ) !=  nullptr)
            {
                if (!m_pRenderData)
                {
                    OSL_FAIL("GetRenderDoc: no renderdata");
                    return nullptr;
                }
                SwView *const pSwView(static_cast<SwView *>(rpView));
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
    aDocPrintData.SetPrintTable( true ); // for now it was decided that tables should always be printed
    aDocPrintData.SetPrintDraw( rPrintUIOptions.IsPrintDrawings() );
    aDocPrintData.SetPrintControl( rPrintUIOptions.IsPrintFormControls() );
    aDocPrintData.SetPrintLeftPage( rPrintUIOptions.IsPrintLeftPages() );
    aDocPrintData.SetPrintRightPage( rPrintUIOptions.IsPrintRightPages() );
    aDocPrintData.SetPrintReverse( false ); /*handled by print dialog now*/
    aDocPrintData.SetPaperFromSetup( rPrintUIOptions.IsPaperFromSetup() );
    aDocPrintData.SetPrintEmptyPages( rPrintUIOptions.IsPrintEmptyPages( bIsPDFEXport ) );
    aDocPrintData.SetPrintPostIts( rPrintUIOptions.GetPrintPostItsType() );
    aDocPrintData.SetPrintProspect( rPrintUIOptions.IsPrintProspect() );
    aDocPrintData.SetPrintProspect_RTL( rPrintUIOptions.IsPrintProspectRTL() );
    aDocPrintData.SetPrintPageBackground( rPrintUIOptions.IsPrintPageBackground() );
    aDocPrintData.SetPrintBlackFont( rPrintUIOptions.IsPrintWithBlackTextColor() );
    // aDocPrintData.SetPrintSingleJobs( b ); handled by File/Print dialog itself
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
    if(!IsValid())
    {
        throw DisposedException( OUString(),
                static_cast< XTextDocument* >(this) );
    }

    const bool bIsPDFExport = !lcl_SeqHasProperty( rxOptions, "IsPrinter" );
    bool bIsSwSrcView = false;
    SfxViewShell *pView = GetRenderView( bIsSwSrcView, rxOptions, bIsPDFExport );

    if (!bIsSwSrcView && !m_pRenderData)
        m_pRenderData = new SwRenderData;
    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = lcl_GetPrintUIOptions( pDocShell, pView );
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
                    m_pPrintUIOptions, bIsPDFExport );

            if (pSwView)
            {
                // PDF export should not make use of the SwPrtOptions
                const SwPrintData *pPrtOptions = bIsPDFExport
                    ? nullptr : m_pRenderData->GetSwPrtOptions();
                bool setShowPlaceHoldersInPDF = false;
                if(bIsPDFExport)
                {
                    const sal_Int32 nLen = rxOptions.getLength();
                    const beans::PropertyValue *pProps = rxOptions.getConstArray();
                    for (sal_Int32 i = 0;  i < nLen;  ++i)
                    {
                        if (pProps[i].Name == "ExportPlaceholders")
                        {
                            pProps[i].Value >>= setShowPlaceHoldersInPDF;
                            break;
                        }
                    }
                }
                m_pRenderData->ViewOptionAdjust( pPrtOptions, setShowPlaceHoldersInPDF );
            }

            // since printing now also use the API for PDF export this option
            // should be set for printing as well ...
            pViewShell->SetPDFExportOption( true );
            bool bOrigStatus = pRenderDocShell->IsEnableSetModified();
            // check configuration: shall update of printing information in DocInfo set the document to "modified"?
            bool bStateChanged = false;
            if ( bOrigStatus && !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
            {
                pRenderDocShell->EnableSetModified( false );
                bStateChanged = true;
            }

            // #122919# Force field update before PDF export
            pViewShell->SwViewShell::UpdateFields(true);
            if( bStateChanged )
                pRenderDocShell->EnableSetModified();

            // there is some redundancy between those two function calls, but right now
            // there is no time to sort this out.
            //TODO: check what exactly needs to be done and make just one function for that
            pViewShell->CalcLayout();
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
                m_pRenderData->CreatePostItData( pDoc, pViewShell->GetViewOptions(), pOutDev );
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

    return nRet;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SwXTextDocument::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
    {
        throw DisposedException("", static_cast< XTextDocument* >(this));
    }

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
        m_pPrintUIOptions = lcl_GetPrintUIOptions( pDocShell, pView );
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

    // TODO/mba: we really need a generic way to get the SwViewShell!
    SwViewShell* pVwSh = nullptr;
    SwView* pSwView = dynamic_cast<SwView*>( pView );
    if ( pSwView )
        pVwSh = pSwView->GetWrtShellPtr();
    else
        pVwSh = static_cast<SwPagePreview*>(pView)->GetViewShell();

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
            // for the correct PageSisze right now...
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
            aTmpSize = pVwSh->GetPageSize( nPage, bIsSkipEmptyPages );
            aPageSize = awt::Size ( convertTwipToMm100( aTmpSize.Width() ),
                                    convertTwipToMm100( aTmpSize.Height() ));
        }

        sal_Int32 nLen = 2;
        aRenderer.realloc(2);
        aRenderer[0].Name  = "PageSize";
        aRenderer[0].Value <<= aPageSize;
        aRenderer[1].Name  = "PageIncludesNonprintableArea";
        aRenderer[1].Value <<= true;
        if (aPreferredPageSize.Width && aPreferredPageSize.Height)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            aRenderer[ nLen - 1 ].Name  = "PreferredPageSize";
            aRenderer[ nLen - 1 ].Value <<= aPreferredPageSize;
        }
        if (nPrinterPaperTray >= 0)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            aRenderer[ nLen - 1 ].Name  = "PrinterPaperTray";
            aRenderer[ nLen - 1 ].Value <<= nPrinterPaperTray;
        }
    }

    // #i117783#
    if ( bApplyPagePrintSettingsFromXPagePrintable )
    {
        const SwPagePreviewPrtData* pPagePrintSettings =
                                        pDocShell->GetDoc()->GetPreviewPrtData();
        if ( pPagePrintSettings &&
             ( pPagePrintSettings->GetRow() > 1 ||
               pPagePrintSettings->GetCol() > 1 ) )
        {
            // extend render data by page print settings attributes
            sal_Int32 nLen = aRenderer.getLength();
            const sal_Int32 nRenderDataIdxStart = nLen;
            nLen += 9;
            aRenderer.realloc( nLen );
            // put page print settings attribute into render data
            const sal_Int32 nRow = pPagePrintSettings->GetRow();
            aRenderer[ nRenderDataIdxStart + 0 ].Name  = "NUpRows";
            aRenderer[ nRenderDataIdxStart + 0 ].Value <<= ( nRow > 1 ? nRow : 1 );
            const sal_Int32 nCol = pPagePrintSettings->GetCol();
            aRenderer[ nRenderDataIdxStart + 1 ].Name  = "NUpColumns";
            aRenderer[ nRenderDataIdxStart + 1 ].Value <<= ( nCol > 1 ? nCol : 1 );
            aRenderer[ nRenderDataIdxStart + 2 ].Name  = "NUpPageMarginLeft";
            aRenderer[ nRenderDataIdxStart + 2 ].Value <<= pPagePrintSettings->GetLeftSpace();
            aRenderer[ nRenderDataIdxStart + 3 ].Name  = "NUpPageMarginRight";
            aRenderer[ nRenderDataIdxStart + 3 ].Value <<= pPagePrintSettings->GetRightSpace();
            aRenderer[ nRenderDataIdxStart + 4 ].Name  = "NUpPageMarginTop";
            aRenderer[ nRenderDataIdxStart + 4 ].Value <<= pPagePrintSettings->GetTopSpace();
            aRenderer[ nRenderDataIdxStart + 5 ].Name  = "NUpPageMarginBottom";
            aRenderer[ nRenderDataIdxStart + 5 ].Value <<= pPagePrintSettings->GetBottomSpace();
            aRenderer[ nRenderDataIdxStart + 6 ].Name  = "NUpHorizontalSpacing";
            aRenderer[ nRenderDataIdxStart + 6 ].Value <<= pPagePrintSettings->GetHorzSpace();
            aRenderer[ nRenderDataIdxStart + 7 ].Name  = "NUpVerticalSpacing";
            aRenderer[ nRenderDataIdxStart + 7 ].Value <<= pPagePrintSettings->GetVertSpace();
            {
                Printer* pPrinter = pDocShell->GetDoc()->getIDocumentDeviceAccess().getPrinter( false );
                if ( pPrinter )
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
                    aRenderer[ nRenderDataIdxStart + 8 ].Name  = "NUpPaperSize";
                    aRenderer[ nRenderDataIdxStart + 8 ].Value <<= aNewPageSize;
                }
            }
        }

        bApplyPagePrintSettingsFromXPagePrintable = false;
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
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( pDocShell, false );

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
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell,  false );
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
    if(!IsValid())
    {
        throw DisposedException( OUString(),
                static_cast< XTextDocument* >(this) );
    }

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (0 > nRenderer)
        throw IllegalArgumentException();

    const bool bIsPDFExport = !lcl_SeqHasProperty( rxOptions, "IsPrinter" );
    bool bIsSwSrcView = false;
    SfxViewShell *pView = GetRenderView( bIsSwSrcView, rxOptions, bIsPDFExport );

    OSL_ENSURE( m_pRenderData, "data should have been created already in getRendererCount..." );
    OSL_ENSURE( m_pPrintUIOptions, "data should have been created already in getRendererCount..." );
    if (!bIsSwSrcView && !m_pRenderData)
        m_pRenderData = new SwRenderData;
    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = lcl_GetPrintUIOptions( pDocShell, pView );
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
                //!! (check for SwView first as in GuessViewShell) !!
                OSL_ENSURE( pView, "!! view missing !!" );
                SwViewShell* pVwSh = nullptr;
                if (pView)
                {
                    // TODO/mba: we really need a generic way to get the SwViewShell!
                    SwView* pSwView = dynamic_cast<SwView*>( pView );
                    if ( pSwView )
                        pVwSh = pSwView->GetWrtShellPtr();
                    else
                        pVwSh = static_cast<SwPagePreview*>(pView)->GetViewShell();
                }

                // get output device to use
                VclPtr< OutputDevice > pOut = lcl_GetOutputDevice( *m_pPrintUIOptions );

                if(pVwSh && pOut && m_pRenderData->HasSwPrtOptions())
                {
                    const OUString aPageRange  = m_pPrintUIOptions->getStringValue( "PageRange" );
                    const bool bFirstPage           = m_pPrintUIOptions->getBoolValue( "IsFirstPage" );
                    bool bIsSkipEmptyPages          = !m_pPrintUIOptions->IsPrintEmptyPages( bIsPDFExport );

                    OSL_ENSURE(( dynamic_cast< const SwView *>( pView ) !=  nullptr &&  m_pRenderData->IsViewOptionAdjust())
                            || (dynamic_cast< const SwView *>( pView ) ==  nullptr && !m_pRenderData->IsViewOptionAdjust()),
                            "SwView / SwViewOptionAdjust_Impl availability mismatch" );

                    // since printing now also use the API for PDF export this option
                    // should be set for printing as well ...
                    pVwSh->SetPDFExportOption( true );

                    // #i12836# enhanced pdf export

                    // First, we have to export hyperlinks, notes, and outline to pdf.
                    // During this process, additional information required for tagging
                    // the pdf file are collected, which are evaulated during painting.

                    SwWrtShell* pWrtShell = dynamic_cast< const SwView *>( pView ) !=  nullptr ?
                                            static_cast<SwView*>(pView)->GetWrtShellPtr() :
                                            nullptr;

                    SwPrintData const& rSwPrtOptions =
                        *m_pRenderData->GetSwPrtOptions();

                    if (bIsPDFExport && bFirstPage && pWrtShell)
                    {
                        SwEnhancedPDFExportHelper aHelper( *pWrtShell, *pOut, aPageRange, bIsSkipEmptyPages, false, rSwPrtOptions );
                    }

                    if (bPrintProspect)
                        pVwSh->PrintProspect( pOut, rSwPrtOptions, nRenderer );
                    else    // normal printing and PDF export
                        pVwSh->PrintOrPDFExport( pOut, rSwPrtOptions, nRenderer );

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
                            lcl_DisposeView( m_pHiddenViewFrame, pDocShell );
                            m_pHiddenViewFrame = nullptr;

                            // prevent crash described in #i108805
                            SwDocShell *pRenderDocShell = pDoc->GetDocShell();
                            SfxItemSet *pSet = pRenderDocShell->GetMedium()->GetItemSet();
                            pSet->Put( SfxBoolItem( SID_HIDDEN, false ) );

                        }
                    }
                }
            }
        }
    }
    if( bLastPage )
    {
        delete m_pRenderData;       m_pRenderData     = nullptr;
        delete m_pPrintUIOptions;   m_pPrintUIOptions = nullptr;
    }
}

// xforms::XFormsSupplier
Reference<XNameContainer> SAL_CALL SwXTextDocument::getXForms()
{
    SolarMutexGuard aGuard;
    if ( !pDocShell )
        throw DisposedException( OUString(), static_cast< XTextDocument* >( this ) );
    SwDoc* pDoc = pDocShell->GetDoc();
    return pDoc->getXForms();
}

uno::Reference< text::XFlatParagraphIterator > SAL_CALL SwXTextDocument::getFlatParagraphIterator(::sal_Int32 nTextMarkupType, sal_Bool bAutomatic)
{
    SolarMutexGuard aGuard;
    if (!IsValid())
    {
        throw DisposedException("SwXTextDocument not valid",
                static_cast<XTextDocument*>(this));
    }

    return SwUnoCursorHelper::CreateFlatParagraphIterator(
            *pDocShell->GetDoc(), nTextMarkupType, bAutomatic);
}

uno::Reference< util::XCloneable > SwXTextDocument::createClone(  )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw DisposedException("", static_cast< XTextDocument* >(this));

    // create a new document - hidden - copy the storage and return it
    // SfxObjectShellRef is used here, since the model should control object lifetime after creation
    // and thus SfxObjectShellLock is not allowed here
    // the model holds reference to the shell, so the shell will not destructed at the end of method
    SfxObjectShellRef pShell = pDocShell->GetDoc()->CreateCopy(false, false);
    uno::Reference< frame::XModel > xNewModel = pShell->GetModel();
    uno::Reference< embed::XStorage > xNewStorage = ::comphelper::OStorageHelper::GetTemporaryStorage( );
    uno::Sequence< beans::PropertyValue > aTempMediaDescriptor;
    storeToStorage( xNewStorage, aTempMediaDescriptor );
    uno::Reference< document::XStorageBasedDocument > xStorageDoc( xNewModel, uno::UNO_QUERY );
    xStorageDoc->loadFromStorage( xNewStorage, aTempMediaDescriptor );
    return uno::Reference< util::XCloneable >( xNewModel, UNO_QUERY );
}

void SwXTextDocument::paintTile( VirtualDevice &rDevice,
                                 int nOutputWidth, int nOutputHeight,
                                 int nTilePosX, int nTilePosY,
                                 long nTileWidth, long nTileHeight )
{
    SwViewShell* pViewShell = pDocShell->GetWrtShell();
    pViewShell->PaintTile(rDevice, nOutputWidth, nOutputHeight,
                          nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    LokChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

Size SwXTextDocument::getDocumentSize()
{
    SwViewShell* pViewShell = pDocShell->GetWrtShell();
    Size aDocSize = pViewShell->GetDocSize();

    return Size(aDocSize.Width()  + 2 * DOCUMENTBORDER,
                aDocSize.Height() + 2 * DOCUMENTBORDER);
}

void SwXTextDocument::setPart(int nPart)
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    if (!pWrtShell)
        return;

    pWrtShell->GotoPage(nPart + 1, true);
}

int SwXTextDocument::getParts()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    if (!pWrtShell)
        return 0;

    return pWrtShell->GetPageCnt();
}

OUString SwXTextDocument::getPartPageRectangles()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    if (!pWrtShell)
        return OUString();

    return pWrtShell->getPageRectangles();
}

void SwXTextDocument::setClipboard(const uno::Reference<datatransfer::clipboard::XClipboard>& xClipboard)
{
    SolarMutexGuard aGuard;

    pDocShell->GetView()->GetEditWin().SetClipboard(xClipboard);
}

bool SwXTextDocument::isMimeTypeSupported()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
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
    SwView* pView = pDocShell->GetView();
    if (!pView)
        return;

    // set the PgUp/PgDown offset
    pView->ForcePageUpDownOffset(2 * rRectangle.GetHeight() / 3);
}

void SwXTextDocument::setClientZoom(int nTilePixelWidth_, int /*nTilePixelHeight_*/,
                                    int nTileTwipWidth_, int /*nTileTwipHeight_*/)
{
    // Here we set the zoom value as it has been set by the user in the client.
    // This value is used in postMouseEvent and setGraphicSelection methods
    // for in place chart editing. We assume that x and y scale is roughly
    // the same.
    // Indeed we could set mnTilePixelWidth, mnTilePixelHeight, mnTileTwipWidth,
    // mnTileTwipHeight data members of this class but they are not very useful
    // since we need to be able to retrieve the zoom value for each view shell.
    SfxInPlaceClient* pIPClient = pDocShell->GetView()->GetIPClient();
    if (pIPClient)
    {
        SwViewShell* pWrtViewShell = pDocShell->GetWrtShell();
        double fScale = nTilePixelWidth_ * TWIPS_PER_PIXEL / (nTileTwipWidth_ * 1.0);
        SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
        if (aOption.GetZoom() != fScale * 100)
        {
            aOption.SetZoom(fScale * 100);
            pWrtViewShell->ApplyViewOptions(aOption);

            // Changing the zoom value doesn't always trigger the updating of
            // the client ole object area, so we call it directly.
            pIPClient->VisAreaChanged();
        }
    }
}

Pointer SwXTextDocument::getPointer()
{
    SolarMutexGuard aGuard;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    if (!pWrtShell)
        return Pointer();

    return pWrtShell->GetView().GetEditWin().GetPointer();
}

OUString SwXTextDocument::getTrackedChanges()
{
    const SwRedlineTable& rRedlineTable = pDocShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    boost::property_tree::ptree aTrackedChanges;
    for (SwRedlineTable::size_type i = 0; i < rRedlineTable.size(); ++i)
    {
        boost::property_tree::ptree aTrackedChange;
        aTrackedChange.put("index", rRedlineTable[i]->GetId());
        aTrackedChange.put("author", rRedlineTable[i]->GetAuthorString(1).toUtf8().getStr());
        aTrackedChange.put("type", nsRedlineType_t::SwRedlineTypeToOUString(rRedlineTable[i]->GetRedlineData().GetType()).toUtf8().getStr());
        aTrackedChange.put("comment", rRedlineTable[i]->GetRedlineData().GetComment().toUtf8().getStr());
        aTrackedChange.put("description", rRedlineTable[i]->GetDescr().toUtf8().getStr());
        OUString sDateTime = utl::toISO8601(rRedlineTable[i]->GetRedlineData().GetTimeStamp().GetUNODateTime());
        aTrackedChange.put("dateTime", sDateTime.toUtf8().getStr());

        SwContentNode* pContentNd = rRedlineTable[i]->GetContentNode();
        SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
        if (pView && pContentNd)
        {
            SwShellCursor aCursor(pView->GetWrtShell(), *(rRedlineTable[i]->Start()));
            aCursor.SetMark();
            aCursor.GetMark()->nNode = *pContentNd;
            aCursor.GetMark()->nContent.Assign(pContentNd, rRedlineTable[i]->End()->nContent.GetIndex());

            aCursor.FillRects();

            SwRects* pRects(&aCursor);
            std::vector<OString> aRects;
            for(SwRect& rNextRect : *pRects)
                aRects.push_back(rNextRect.SVRect().toString());

            const OString sRects = comphelper::string::join("; ", aRects);
            aTrackedChange.put("textRange", sRects.getStr());
        }

        aTrackedChanges.push_back(std::make_pair("", aTrackedChange));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("redlines", aTrackedChanges);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return OUString::fromUtf8(aStream.str().c_str());
}

OUString SwXTextDocument::getTrackedChangeAuthors()
{
    return SW_MOD()->GetRedlineAuthorInfo();
}

OUString SwXTextDocument::getRulerState()
{
    SwView* pView = pDocShell->GetView();
    return OUString::fromUtf8(dynamic_cast<SwCommentRuler&>(pView->GetHRuler()).CreateJsonNotification().c_str());
}

OUString SwXTextDocument::getPostIts()
{
    SolarMutexGuard aGuard;
    boost::property_tree::ptree aAnnotations;
    for (std::list<SwSidebarItem*>::const_iterator i = pDocShell->GetView()->GetPostItMgr()->begin();
         i != pDocShell->GetView()->GetPostItMgr()->end(); ++i )
    {
        sw::annotation::SwAnnotationWin* pWin = (*i)->pPostIt.get();

        const SwPostItField* pField = pWin->GetPostItField();
        const SwRect& aRect = pWin->GetAnchorRect();
        const tools::Rectangle aSVRect(aRect.Pos().getX(),
                                aRect.Pos().getY(),
                                aRect.Pos().getX() + aRect.SSize().Width(),
                                aRect.Pos().getY() + aRect.SSize().Height());
        std::vector<OString> aRects;
        for (const basegfx::B2DRange& aRange : pWin->GetAnnotationTextRanges())
        {
            const SwRect rect(aRange.getMinX(), aRange.getMinY(), aRange.getWidth(), aRange.getHeight());
            aRects.push_back(rect.SVRect().toString());
        }
        const OString sRects = comphelper::string::join("; ", aRects);

        boost::property_tree::ptree aAnnotation;
        aAnnotation.put("id", pField->GetPostItId());
        aAnnotation.put("parent", pWin->CalcParent());
        aAnnotation.put("author", pField->GetPar1().toUtf8().getStr());
        aAnnotation.put("text", pField->GetPar2().toUtf8().getStr());
        aAnnotation.put("dateTime", utl::toISO8601(pField->GetDateTime().GetUNODateTime()));
        aAnnotation.put("anchorPos", aSVRect.toString());
        aAnnotation.put("textRange", sRects.getStr());

        aAnnotations.push_back(std::make_pair("", aAnnotation));
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("comments", aAnnotations);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    return OUString::fromUtf8(aStream.str().c_str());
}

int SwXTextDocument::getPart()
{
    SolarMutexGuard aGuard;

    SwView* pView = pDocShell->GetView();
    if (!pView)
        return 0;

    return pView->getPart();
}

OUString SwXTextDocument::getPartName(int nPart)
{
    SolarMutexGuard aGuard;

    return SwResId(STR_PAGE) + OUString::number(nPart + 1);
}

OUString SwXTextDocument::getPartHash(int nPart)
{
    SolarMutexGuard aGuard;
    OUString sPart(SwResId(STR_PAGE) + OUString::number(nPart + 1));

    return OUString::number(sPart.hashCode());
}

void SwXTextDocument::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    SolarMutexGuard aGuard;

    SwViewShell* pViewShell = pDocShell->GetWrtShell();

    SwView* pView = pDocShell->GetView();
    if (!pView)
        return;

    pView->SetViewLayout(1/*nColumns*/, false/*bBookMode*/, true);

    // Tiled rendering defaults.
    SwViewOption aViewOption(*pViewShell->GetViewOptions());
    aViewOption.SetHardBlank(false);
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        const beans::PropertyValue& rValue = rArguments[i];
        if (rValue.Name == ".uno:HideWhitespace" && rValue.Value.has<bool>())
            aViewOption.SetHideWhitespaceMode(rValue.Value.get<bool>());
        else if (rValue.Name == ".uno:ShowBorderShadow" && rValue.Value.has<bool>())
            SwViewOption::SetAppearanceFlag(ViewOptFlags::Shadow , rValue.Value.get<bool>());
        else if (rValue.Name == ".uno:Author" && rValue.Value.has<OUString>())
        {
            // Store the author name in the view.
            pView->SetRedlineAuthor(rValue.Value.get<OUString>());
            // Let the actual author name pick up the value from the current
            // view, which would normally happen only during the next view
            // switch.
            pDocShell->SetView(pView);
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
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    rEditWin.EnableMapMode(false);

    // when the "This document may contain formatting or content that cannot
    // be saved..." dialog appears, it is auto-cancelled with tiled rendering,
    // causing 'Save' being disabled; so let's always save to the original
    // format
    SvtSaveOptions().SetWarnAlienFormat(false);

    // disable word auto-completion suggestions, the tooltips are not visible,
    // and the editeng-like auto-completion is annoying
    SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags().bAutoCompleteWords = false;

    // don't change the whitespace at the beginning of paragraphs, this is
    // annoying when taking minutes without further formatting
    SwEditShell::GetAutoFormatFlags()->bAFormatByInpDelSpacesAtSttEnd = false;
}

void SwXTextDocument::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = &(pDocShell->GetView()->GetEditWin());

    SfxViewShell* pViewShell = pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    vcl::Window* pChartWindow = aChartHelper.GetWindow();
    if (pChartWindow)
    {
        pWindow = pChartWindow;
    }

    KeyEvent aEvent(nCharCode, nKeyCode, 0);

    switch (nType)
    {
    case LOK_KEYEVENT_KEYINPUT:
        pWindow->KeyInput(aEvent);
        break;
    case LOK_KEYEVENT_KEYUP:
        pWindow->KeyUp(aEvent);
        break;
    default:
        assert(false);
        break;
    }
}

void SwXTextDocument::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    SwViewShell* pWrtViewShell = pDocShell->GetWrtShell();
    SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
    double fScale = aOption.GetZoom() / (TWIPS_PER_PIXEL * 100.0);

    // check if user hit a chart which is being edited by him
    SfxViewShell* pViewShell = pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.postMouseEvent(nType, nX, nY,
                                    nCount, nButtons, nModifier,
                                    fScale, fScale))
        return;

    // check if the user hit a chart which is being edited by someone else
    // and, if so, skip current mouse event
    if (nType != LOK_MOUSEEVENT_MOUSEMOVE)
    {
        if (LokChartHelper::HitAny(Point(nX, nY)))
            return;
    }

    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    Point aPos(nX , nY);
    MouseEvent aEvent(aPos, nCount, MouseEventModifiers::SIMPLECLICK, nButtons, nModifier);

    switch (nType)
    {
    case LOK_MOUSEEVENT_MOUSEBUTTONDOWN:
        rEditWin.LogicMouseButtonDown(aEvent);

        if (nButtons & MOUSE_RIGHT)
        {
            const CommandEvent aCEvt(aPos, CommandEventId::ContextMenu, true, nullptr);
            rEditWin.Command(aCEvt);
        }
        break;
    case LOK_MOUSEEVENT_MOUSEBUTTONUP:
        rEditWin.LogicMouseButtonUp(aEvent);
        break;
    case LOK_MOUSEEVENT_MOUSEMOVE:
        rEditWin.LogicMouseMove(aEvent);
        break;
    default:
        assert(false);
        break;
    }
}

void SwXTextDocument::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    SfxViewShell* pViewShell = pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setTextSelection(nType, nX, nY))
        return;

    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
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

OString SwXTextDocument::getTextSelection(const char* pMimeType, OString& rUsedMimeType)
{
    SolarMutexGuard aGuard;

    uno::Reference<datatransfer::XTransferable> xTransferable;

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    if (SdrView* pSdrView = pWrtShell->GetDrawView())
    {
        if (pSdrView->GetTextEditObject())
        {
            // Editing shape text
            EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
            xTransferable = rEditView.GetEditEngine()->CreateTransferable(rEditView.GetSelection());
        }
    }

    if (SwPostItMgr* pPostItMgr = pDocShell->GetView()->GetPostItMgr())
    {
        if (sw::annotation::SwAnnotationWin* pWin = pPostItMgr->GetActiveSidebarWin())
        {
            // Editing postit text.
            EditView& rEditView = pWin->GetOutlinerView()->GetEditView();
            xTransferable = rEditView.GetEditEngine()->CreateTransferable(rEditView.GetSelection());
        }
    }

    if (!xTransferable.is())
        xTransferable = new SwTransferable(*pWrtShell);

    // Take care of UTF-8 text here.
    OString aMimeType(pMimeType);
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    if (aMimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (aMimeType.getToken(0, ';', nIndex) == "charset=utf-8")
        {
            aMimeType = "text/plain;charset=utf-16";
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(aMimeType.getStr());
    if (aMimeType == "text/plain;charset=utf-16")
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        aFlavor.DataType = cppu::UnoType< uno::Sequence<sal_Int8> >::get();

    if (!xTransferable->isDataFlavorSupported(aFlavor))
        return OString();

    uno::Any aAny(xTransferable->getTransferData(aFlavor));

    OString aRet;
    if (aFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        OUString aString;
        aAny >>= aString;
        if (bConvert)
            aRet = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
        else
            aRet = OString(reinterpret_cast<const sal_Char *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), aSequence.getLength());
    }

    rUsedMimeType = pMimeType;
    return aRet;
}

void SwXTextDocument::setGraphicSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    SwViewShell* pWrtViewShell = pDocShell->GetWrtShell();
    SwViewOption aOption(*(pWrtViewShell->GetViewOptions()));
    double fScale = aOption.GetZoom() / (TWIPS_PER_PIXEL * 100.0);

    SfxViewShell* pViewShell = pDocShell->GetView();
    LokChartHelper aChartHelper(pViewShell);
    if (aChartHelper.setGraphicSelection(nType, nX, nY, fScale, fScale))
        return;

    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
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

    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
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
    ScopedVclPtrInstance<VirtualDevice> xDevice(&aData, Size(1, 1), DeviceFormat::DEFAULT);
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

void * SAL_CALL SwXTextDocument::operator new( size_t t) throw()
{
    return SwXTextDocumentBaseClass::operator new(t);
}

void SAL_CALL SwXTextDocument::operator delete( void * p) throw()
{
    SwXTextDocumentBaseClass::operator delete(p);
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
        throw IllegalArgumentException("nScriptTypes ranges from 1 to 7!", Reference< XInterface >(), 1);
    if (!pDocShell)
        throw DisposedException();
    SwDoc* pDoc = pDocShell->GetDoc();

    // avoid duplicate values
    std::set< LanguageType > aAllLangs;

    //USER STYLES

    const SwCharFormats *pFormats = pDoc->GetCharFormats();
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

    const SwTextFormatColls *pColls = pDoc->GetTextFormatColls();
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
        pDoc->GetIStyleAccess().getAllStyles(rStyles, i);
        while (!rStyles.empty())
        {
            std::shared_ptr<SfxItemSet> pStyle = rStyles.back();
            rStyles.pop_back();
            const SfxItemSet *pSet = dynamic_cast< const SfxItemSet * >(pStyle.get());

            LanguageType nLang = LANGUAGE_DONTKNOW;
            if (bLatin)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_LANGUAGE, false )).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bAsian)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_CJK_LANGUAGE, false )).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bComplex)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_CTL_LANGUAGE, false )).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
        }
    }

    //TODO/mba: it's a strange concept that a view is needed to retrieve core data
    SwWrtShell *pWrtSh = pDocShell->GetWrtShell();
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
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE, false )).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bAsian)
                    {
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE_CJK, false )).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bComplex)
                    {
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE_CTL, false )).GetLanguage();
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
        for (std::set< LanguageType >::const_iterator it = aAllLangs.begin(); it != aAllLangs.end(); ++it)
        {
            if (nCount >= nMaxCount)
                break;
            if (LANGUAGE_NONE != *it)
            {
                pLanguage[nCount] = LanguageTag::convertToLocale( *it );
                pLanguage[nCount].Language = SvtLanguageTable::GetLanguageString( *it );
                nCount += 1;
            }
        }
    }

    return aLanguages;
}

SwXLinkTargetSupplier::SwXLinkTargetSupplier(SwXTextDocument& rxDoc) :
    pxDoc(&rxDoc)
{
    sTables     = SwResId(STR_CONTENT_TYPE_TABLE);
    sFrames     = SwResId(STR_CONTENT_TYPE_FRAME);
    sGraphics   = SwResId(STR_CONTENT_TYPE_GRAPHIC);
    sOLEs       = SwResId(STR_CONTENT_TYPE_OLE);
    sSections   = SwResId(STR_CONTENT_TYPE_REGION);
    sOutlines   = SwResId(STR_CONTENT_TYPE_OUTLINE);
    sBookmarks  = SwResId(STR_CONTENT_TYPE_BOOKMARK);
}

SwXLinkTargetSupplier::~SwXLinkTargetSupplier()
{
}

Any SwXLinkTargetSupplier::getByName(const OUString& rName)
{
    Any aRet;
    if(!pxDoc)
        throw RuntimeException("No document available");
    OUString sSuffix("|");
    if(rName == sTables)
    {
        sSuffix += "table";

        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextTables(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sFrames)
    {
        sSuffix += "frame";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextFrames(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sSections)
    {
        sSuffix += "region";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextSections(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sGraphics)
    {
        sSuffix += "graphic";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        pxDoc->getGraphicObjects(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sOLEs)
    {
        sSuffix += "ole";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        pxDoc->getEmbeddedObjects(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sOutlines)
    {
        sSuffix += "outline";
        Reference< XNameAccess >  xTables = new SwXLinkNameAccessWrapper(
                                        *pxDoc, rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xTables, UNO_QUERY);
    }
    else if(rName == sBookmarks)
    {
        sSuffix.clear();
        Reference< XNameAccess >  xBkms = new SwXLinkNameAccessWrapper(
                                        pxDoc->getBookmarks(), rName, sSuffix );
        aRet <<= Reference< XPropertySet >(xBkms, UNO_QUERY);
    }
    else
        throw NoSuchElementException();
    return aRet;
}

Sequence< OUString > SwXLinkTargetSupplier::getElementNames()
{
    Sequence< OUString > aRet(7);
    OUString* pNames = aRet.getArray();
    pNames[0] = sTables;
    pNames[1] = sFrames  ;
    pNames[2] = sGraphics;
    pNames[3] = sOLEs   ;
    pNames[4] = sSections;
    pNames[5] = sOutlines;
    pNames[6] = sBookmarks;
    return aRet;
}

sal_Bool SwXLinkTargetSupplier::hasByName(const OUString& rName)
{
    if( rName == sTables  ||
        rName == sFrames  ||
        rName == sGraphics||
        rName == sOLEs   ||
        rName == sSections ||
        rName == sOutlines ||
        rName == sBookmarks    )
        return true;
    return false;
}

uno::Type  SwXLinkTargetSupplier::getElementType()
{
    return cppu::UnoType<XPropertySet>::get();

}

sal_Bool SwXLinkTargetSupplier::hasElements()
{
    return nullptr != pxDoc;
}

OUString SwXLinkTargetSupplier::getImplementationName()
{
    return OUString("SwXLinkTargetSupplier");
}

sal_Bool SwXLinkTargetSupplier::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXLinkTargetSupplier::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.document.LinkTargets" };
    return aRet;
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(
            Reference< XNameAccess > const & xAccess, const OUString& rLinkDisplayName, const OUString& sSuffix ) :
    xRealAccess(xAccess),
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    sLinkSuffix(sSuffix),
    sLinkDisplayName(rLinkDisplayName),
    pxDoc(nullptr)
{
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const OUString& rLinkDisplayName, const OUString& sSuffix) :
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    sLinkSuffix(sSuffix),
    sLinkDisplayName(rLinkDisplayName),
    pxDoc(&rxDoc)
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
    OUString sSuffix(sLinkSuffix);
    if(sParam.getLength() > sSuffix.getLength() )
    {
        OUString sCmp = sParam.copy(sParam.getLength() - sSuffix.getLength(),
                                                    sSuffix.getLength());
        if(sCmp == sSuffix)
        {
            if(pxDoc)
            {
                sParam = sParam.copy(0, sParam.getLength() - sSuffix.getLength());
                if(!pxDoc->GetDocShell())
                    throw RuntimeException("No document shell available");
                SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
                const size_t nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                for (size_t i = 0; i < nOutlineCount && !bFound; ++i)
                {
                    const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
                    const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
                    if(sParam == lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule))
                    {
                        Reference< XPropertySet >  xOutline = new SwXOutlineTarget(sParam);
                        aRet <<= xOutline;
                        bFound = true;
                    }
                }
            }
            else
            {
                aRet = xRealAccess->getByName(sParam.copy(0, sParam.getLength() - sSuffix.getLength()));
                Reference< XInterface > xInt;
                if(!(aRet >>= xInt))
                    throw RuntimeException("Could not retrieve property");
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
    if(pxDoc)
    {
        if(!pxDoc->GetDocShell())
            throw RuntimeException("No document shell available");

        SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
        const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
        const size_t nOutlineCount = rOutlineNodes.size();
        aRet.realloc(nOutlineCount);
        OUString* pResArr = aRet.getArray();
        const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
        for (size_t i = 0; i < nOutlineCount; ++i)
        {
            OUString sEntry = lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule);
            sEntry += "|outline";
            pResArr[i] = sEntry;
        }
    }
    else
    {
        Sequence< OUString > aOrg = xRealAccess->getElementNames();
        const OUString* pOrgArr = aOrg.getConstArray();
        aRet.realloc(aOrg.getLength());
        OUString* pResArr = aRet.getArray();
        for(long i = 0; i < aOrg.getLength(); i++)
        {
            pResArr[i] = pOrgArr[i] + sLinkSuffix;
        }
    }
    return aRet;
}

sal_Bool SwXLinkNameAccessWrapper::hasByName(const OUString& rName)
{
    bool bRet = false;
    OUString sParam(rName);
    if(sParam.getLength() > sLinkSuffix.getLength() )
    {
        OUString sCmp = sParam.copy(sParam.getLength() - sLinkSuffix.getLength(),
                                                    sLinkSuffix.getLength());
        if(sCmp == sLinkSuffix)
        {
            sParam = sParam.copy(0, sParam.getLength() - sLinkSuffix.getLength());
            if(pxDoc)
            {
                if(!pxDoc->GetDocShell())
                    throw RuntimeException("No document shell available");
                SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
                const size_t nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                for (size_t i = 0; i < nOutlineCount && !bRet; ++i)
                {
                    const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
                    const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
                    if(sParam ==
                        lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule))
                    {
                        bRet = true;
                    }
                }
            }
            else
            {
                bRet = xRealAccess->hasByName(sParam);
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
    if(pxDoc)
    {
        OSL_FAIL("not implemented");
    }
    else
    {
        bRet = xRealAccess->hasElements();
    }
    return bRet;
}

Reference< XPropertySetInfo >  SwXLinkNameAccessWrapper::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXLinkNameAccessWrapper::setPropertyValue(
    const OUString& , const Any& )
{
    throw UnknownPropertyException();
}

static Any lcl_GetDisplayBitmap(const OUString& _sLinkSuffix)
{
    Any aRet;
    OUString sLinkSuffix = _sLinkSuffix;
    if(!sLinkSuffix.isEmpty())
        sLinkSuffix = sLinkSuffix.copy(1);
    OUString sImgId;

    if(sLinkSuffix == "outline")
        sImgId = RID_BMP_NAVI_OUTLINE;
    else if(sLinkSuffix == "table")
        sImgId = RID_BMP_NAVI_TABLE;
    else if(sLinkSuffix == "frame")
        sImgId = RID_BMP_NAVI_FRAME;
    else if(sLinkSuffix == "graphic")
        sImgId = RID_BMP_NAVI_GRAPHIC;
    else if(sLinkSuffix == "ole")
        sImgId = RID_BMP_NAVI_OLE;
    else if(sLinkSuffix.isEmpty())
        sImgId = RID_BMP_NAVI_BOOKMARK;
    else if(sLinkSuffix == "region")
        sImgId = RID_BMP_NAVI_REGION;

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
        aRet <<= sLinkDisplayName;
    }
    else if( rPropertyName == UNO_LINK_DISPLAY_BITMAP )
    {
        aRet = lcl_GetDisplayBitmap(sLinkSuffix);
    }
    else
        throw UnknownPropertyException();
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
    return OUString("SwXLinkNameAccessWrapper");
}

sal_Bool SwXLinkNameAccessWrapper::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXLinkNameAccessWrapper::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.document.LinkTargets" };
    return aRet;
}

SwXOutlineTarget::SwXOutlineTarget(const OUString& rOutlineText) :
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    sOutlineText(rOutlineText)
{
}

SwXOutlineTarget::~SwXOutlineTarget()
{
}

Reference< XPropertySetInfo >  SwXOutlineTarget::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXOutlineTarget::setPropertyValue(
    const OUString& /*PropertyName*/, const Any& /*aValue*/)
{
    throw UnknownPropertyException();
}

Any SwXOutlineTarget::getPropertyValue(const OUString& rPropertyName)
{
    if(rPropertyName != UNO_LINK_DISPLAY_NAME)
        throw UnknownPropertyException();

    return Any(sOutlineText);
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
    return OUString("SwXOutlineTarget");
}

sal_Bool SwXOutlineTarget::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SwXOutlineTarget::getSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.document.LinkTarget" };

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
                if(!xDashTable.is())
                    xDashTable = SvxUnoDashTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xDashTable;
            break;
            case SwCreateDrawTable::Gradient     :
                if(!xGradientTable.is())
                    xGradientTable = SvxUnoGradientTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xGradientTable;
            break;
            case SwCreateDrawTable::Hatch        :
                if(!xHatchTable.is())
                    xHatchTable = SvxUnoHatchTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xHatchTable;
            break;
            case SwCreateDrawTable::Bitmap       :
                if(!xBitmapTable.is())
                    xBitmapTable = SvxUnoBitmapTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xBitmapTable;
            break;
            case SwCreateDrawTable::TransGradient:
                if(!xTransGradientTable.is())
                    xTransGradientTable = SvxUnoTransGradientTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xTransGradientTable;
            break;
            case SwCreateDrawTable::Marker       :
                if(!xMarkerTable.is())
                    xMarkerTable = SvxUnoMarkerTable_createInstance( m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
                xRet = xMarkerTable;
            break;
            case  SwCreateDrawTable::Defaults:
                if(!xDrawDefaults.is())
                    xDrawDefaults = static_cast<cppu::OWeakObject*>(new SwSvxUnoDrawPool(m_pDoc));
                xRet = xDrawDefaults;
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
    xDashTable = nullptr;
    xGradientTable = nullptr;
    xHatchTable = nullptr;
    xBitmapTable = nullptr;
    xTransGradientTable = nullptr;
    xMarkerTable = nullptr;
    xDrawDefaults = nullptr;
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
