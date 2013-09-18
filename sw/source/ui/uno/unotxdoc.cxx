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


#include <osl/mutex.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/print.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <cmdid.h>
#include <swtypes.hxx>
#include <wdocsh.hxx>
#include <wrtsh.hxx>
#include <pview.hxx>
#include <viewsh.hxx>
#include <pvprtdat.hxx>
#include <printdata.hxx>
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
#include <sfx2/objsh.hxx>   // SfxObjectShellRef <-> SV_DECL_REF(SfxObjectShell)
#include <unoprnms.hxx>
#include <unostyle.hxx>
#include <unodraw.hxx>
#include <svl/eitem.hxx>
#include <pagedesc.hxx>
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
#include <com/sun/star/util/SearchOptions.hpp>
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
#include <utlui.hrc>
#include <swcont.hxx>
#include <unodefaults.hxx>
#include <SwXDocumentSettings.hxx>
#include <doc.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svl/zforlist.hxx>
#include <drawdoc.hxx>
#include <SwStyleNameMapper.hxx>
#include <osl/file.hxx>
#include <comphelper/storagehelper.hxx>

// #i12836# enhanced pdf export
#include <EnhancedPDFExportHelper.hxx>
#include <numrule.hxx>

#include <editeng/langitem.hxx>
#include <docary.hxx>      //SwCharFmts
#include <i18nlangtag/languagetag.hxx>

#include <format.hxx>
#include <charfmt.hxx>    //SwCharFmt
#include <fmtcol.hxx>     //SwTxtFmtColl
#include <unostyle.hxx>   //SwAutoStyleFamily
#include <istyleaccess.hxx> // handling of automatic styles

#include <svl/stylepool.hxx>
#include <swatrset.hxx>
#include <view.hxx>
#include <srcview.hxx>

#include <svtools/langtab.hxx>
#include <map>
#include <set>
#include <vector>

#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdoutl.hxx>
#include <svl/languageoptions.hxx>
#include <svx/svdview.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using ::osl::FileBase;

#define SW_CREATE_DASH_TABLE            0x01
#define SW_CREATE_GRADIENT_TABLE        0x02
#define SW_CREATE_HATCH_TABLE           0x03
#define SW_CREATE_BITMAP_TABLE          0x04
#define SW_CREATE_TRANSGRADIENT_TABLE   0x05
#define SW_CREATE_MARKER_TABLE          0x06
#define SW_CREATE_DRAW_DEFAULTS         0x07

extern bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );

static SwPrintUIOptions * lcl_GetPrintUIOptions(
    SwDocShell * pDocShell,
    const SfxViewShell * pView )
{
    if (!pDocShell)
        return NULL;

    const sal_Bool bWebDoc      = NULL != dynamic_cast< const SwWebDocShell * >(pDocShell);
    const bool bSwSrcView   = NULL != dynamic_cast< const SwSrcView * >(pView);
    const SwView * pSwView = dynamic_cast< const SwView * >(pView);
    const bool bHasSelection    = pSwView ? pSwView->HasSelection( sal_False ) : false;  // check for any selection, not just text selection
    const bool bHasPostIts      = sw_GetPostIts( pDocShell->GetDoc(), 0 );

    // get default values to use in dialog from documents SwPrintData
    const SwPrintData &rPrintData = pDocShell->GetDoc()->getPrintData();

    // Get current page number
    sal_uInt16 nCurrentPage = 1;
    SwWrtShell* pSh = pDocShell->GetWrtShell();
    if (pSh)
    {
        SwPaM* pShellCrsr = pSh->GetCrsr();
        nCurrentPage = pShellCrsr->GetPageNum(true, 0);
    }
    else if (!bSwSrcView)
    {
        const SwPagePreView* pPreView = dynamic_cast< const SwPagePreView* >(pView);
        OSL_ENSURE(pPreView, "Unexpected type of the view shell");
        if (pPreView)
            nCurrentPage = pPreView->GetSelectedPage();
    }
    return new SwPrintUIOptions( nCurrentPage, bWebDoc, bSwSrcView, bHasSelection, bHasPostIts, rPrintData );
}

static SwTxtFmtColl *lcl_GetParaStyle(const String& rCollName, SwDoc* pDoc)
{
    SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName( rCollName );
    if( !pColl )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCollName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId )
            pColl = pDoc->GetTxtCollFromPool( nId );
    }
    return pColl;
}

static void lcl_DisposeView( SfxViewFrame* pToClose, SwDocShell* pDocShell )
{
    // check if the view frame still exists
    SfxViewFrame* pFound = SfxViewFrame::GetFirst( pDocShell,
                                sal_False );
    while(pFound)
    {
        if( pFound == pToClose)
        {
            pToClose->DoClose();
            break;
        }
        pFound = SfxViewFrame::GetNext( *pFound,
                                pDocShell,
                                sal_False );
    }
}

class SwXTextDocument::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    ::cppu::OInterfaceContainerHelper m_RefreshListeners;

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
    throw(RuntimeException)
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
    if ( nRet )
        return nRet;
    else
    {
        GetNumberFormatter();
        Any aNumTunnel = xNumFmtAgg->queryAggregation(::getCppuType((Reference<XUnoTunnel>*)0));
        Reference<XUnoTunnel> xNumTunnel;
        aNumTunnel >>= xNumTunnel;
        if(xNumTunnel.is())
            return xNumTunnel->getSomething(rId);
    }

    return SfxBaseModel::getSomething( rId );
}

Any SAL_CALL SwXTextDocument::queryInterface( const uno::Type& rType ) throw(RuntimeException)
{
    Any aRet = SwXTextDocumentBaseClass::queryInterface(rType);
    if ( !aRet.hasValue() )
        aRet = SfxBaseModel::queryInterface(rType);
    if ( !aRet.hasValue() &&
        rType == ::getCppuType((Reference<lang::XMultiServiceFactory>*)0))
    {
        Reference<lang::XMultiServiceFactory> xTmp = this;
        aRet <<= xTmp;
    }

    if ( !aRet.hasValue()
        && rType != ::getCppuType((Reference< com::sun::star::document::XDocumentEventBroadcaster>*)0)
        && rType != ::getCppuType((Reference< com::sun::star::frame::XController>*)0)
        && rType != ::getCppuType((Reference< com::sun::star::frame::XFrame>*)0)
        && rType != ::getCppuType((Reference< com::sun::star::script::XInvocation>*)0)
        && rType != ::getCppuType((Reference< com::sun::star::beans::XFastPropertySet>*)0)
        && rType != ::getCppuType((Reference< com::sun::star::awt::XWindow>*)0))
    {
        GetNumberFormatter();
        if(xNumFmtAgg.is())
            aRet = xNumFmtAgg->queryAggregation(rType);
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

Reference< XAdapter > SwXTextDocument::queryAdapter(  ) throw(RuntimeException)
{
    return SfxBaseModel::queryAdapter();
}

Sequence< uno::Type > SAL_CALL SwXTextDocument::getTypes() throw(RuntimeException)
{
    Sequence< uno::Type > aBaseTypes = SfxBaseModel::getTypes();
    Sequence< uno::Type > aTextTypes = SwXTextDocumentBaseClass::getTypes();

    Sequence< uno::Type > aNumTypes;
    GetNumberFormatter();
    if(xNumFmtAgg.is())
    {
        const uno::Type& rProvType = ::getCppuType((Reference <XTypeProvider>*)0);
        Any aNumProv = xNumFmtAgg->queryAggregation(rProvType);
        Reference<XTypeProvider> xNumProv;
        if(aNumProv >>= xNumProv)
        {
            aNumTypes = xNumProv->getTypes();
        }
    }
    long nIndex = aBaseTypes.getLength();
    // don't forget the lang::XMultiServiceFactory
    aBaseTypes.realloc(aBaseTypes.getLength() + aTextTypes.getLength() + aNumTypes.getLength() + 1);
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
    pBaseTypes[nIndex++] = ::getCppuType((Reference<lang::XMultiServiceFactory>*)0);
    return aBaseTypes;
}

SwXTextDocument::SwXTextDocument(SwDocShell* pShell)
    : SfxBaseModel(pShell)
    , m_pImpl(new Impl)
    ,
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_DOCUMENT)),

    pDocShell(pShell),

    bObjectValid(pShell != 0),

    pDrawPage(0),
    pxXDrawPage(0),

    pxXNumberingRules(0),
    pxXFootnotes(0),
    pxXFootnoteSettings(0),
    pxXEndnotes(0),
    pxXEndnoteSettings(0),
    pxXReferenceMarks(0),
    pxXTextFieldTypes(0),
    pxXTextFieldMasters(0),
    pxXTextSections(0),
    pxXBookmarks(0),
    pxXTextTables(0),
    pxXTextFrames(0),
    pxXGraphicObjects(0),
    pxXEmbeddedObjects(0),
    pxXStyleFamilies(0),
    pxXAutoStyles(0),
    pxXChapterNumbering(0),
    pxXDocumentIndexes(0),

    pxXLineNumberingProperties(0),
    pxLinkTargetSupplier(0),
    pxXRedlines(0),
    m_pHiddenViewFrame(0),
    m_pPrintUIOptions( NULL ),
    m_pRenderData( NULL ),
    // #i117783#
    bApplyPagePrintSettingsFromXPagePrintable( sal_False )
{
}

SwXTextDocument::~SwXTextDocument()
{
    InitNewDoc();
    if(xNumFmtAgg.is())
    {
        Reference< XInterface >  x0;
        xNumFmtAgg->setDelegator(x0);
        xNumFmtAgg = 0;
    }
    delete m_pPrintUIOptions;
    if (m_pRenderData && m_pRenderData->IsViewOptionAdjust())
    {   // rhbz#827695: this can happen if the last page is not printed
        // the ViewShell has been deleted already by SwView::~SwView
        // FIXME: replace this awful implementation of XRenderable with
        // something less insane that has its own view
        m_pRenderData->ViewOptionAdjustCrashPreventionKludge();
    }
    delete m_pRenderData;
}

SwXDocumentPropertyHelper * SwXTextDocument::GetPropertyHelper ()
{
    if(!xPropertyHelper.is())
    {
        pPropertyHelper = new SwXDocumentPropertyHelper(*pDocShell->GetDoc());
        xPropertyHelper = (cppu::OWeakObject*)pPropertyHelper;
    }
    return pPropertyHelper;
}

void SwXTextDocument::GetNumberFormatter()
{
    if(IsValid())
    {
        if(!xNumFmtAgg.is())
        {
            if ( pDocShell->GetDoc() )
            {
                SvNumberFormatsSupplierObj* pNumFmt = new SvNumberFormatsSupplierObj(
                                    pDocShell->GetDoc()->GetNumberFormatter( sal_True ));
                Reference< util::XNumberFormatsSupplier >  xTmp = pNumFmt;
                xNumFmtAgg = Reference< XAggregation >(xTmp, UNO_QUERY);
            }
            if(xNumFmtAgg.is())
                xNumFmtAgg->setDelegator((cppu::OWeakObject*)(SwXTextDocumentBaseClass*)this);
        }
        else
        {
            const uno::Type& rTunnelType = ::getCppuType((Reference <XUnoTunnel>*)0);
            Any aNumTunnel = xNumFmtAgg->queryAggregation(rTunnelType);
            SvNumberFormatsSupplierObj* pNumFmt = 0;
            Reference< XUnoTunnel > xNumTunnel;
            if(aNumTunnel >>= xNumTunnel)
            {
                pNumFmt = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                        xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));

            }
            OSL_ENSURE(pNumFmt, "No number formatter available");
            if(!pNumFmt->GetNumberFormatter())
                pNumFmt->SetNumberFormatter(pDocShell->GetDoc()->GetNumberFormatter( sal_True ));
        }
    }
}

Reference< XText >  SwXTextDocument::getText(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!xBodyText.is())
    {
        pBodyText = new SwXBodyText(pDocShell->GetDoc());
        xBodyText = pBodyText;
    }
    return xBodyText;
}

void SwXTextDocument::reformat(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
}

void SwXTextDocument::lockControllers(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        UnoActionContext* pContext = new UnoActionContext(pDocShell->GetDoc());
        aActionArr.push_front(pContext);
    }
    else
        throw RuntimeException();
}

void SwXTextDocument::unlockControllers(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!aActionArr.empty())
    {
        UnoActionContext* pContext = aActionArr.front();
        aActionArr.pop_front();
        delete pContext;
    }
    else
        throw RuntimeException();
}

sal_Bool SwXTextDocument::hasControllersLocked(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return !aActionArr.empty();
}

Reference< frame::XController >  SwXTextDocument::getCurrentController(void) throw( RuntimeException )
{
    return SfxBaseModel::getCurrentController();
}

void SwXTextDocument::setCurrentController(const Reference< frame::XController > & xController)
    throw( NoSuchElementException, RuntimeException )
{
    SfxBaseModel::setCurrentController(xController);
}

Reference< XInterface >  SwXTextDocument::getCurrentSelection() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xRef;
    if(IsValid())
    {

        const TypeId aTypeId = TYPE(SwView);
        SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
        while(pView && pView->GetObjectShell() != pDocShell)
        {
            pView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId);
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
        throw( RuntimeException )
{
    return SfxBaseModel::attachResource(aURL, aArgs);
}

OUString SwXTextDocument::getURL(void) throw( RuntimeException )
{
    return SfxBaseModel::getURL();
}

Sequence< beans::PropertyValue > SwXTextDocument::getArgs(void) throw( RuntimeException )
{
    return SfxBaseModel::getArgs();
}

void SwXTextDocument::connectController(const Reference< frame::XController > & xController) throw( RuntimeException )
{
    SfxBaseModel::connectController(xController);
}

void SwXTextDocument::disconnectController(const Reference< frame::XController > & xController) throw( RuntimeException )
{
    SfxBaseModel::disconnectController(xController);
}

void SwXTextDocument::dispose(void) throw( RuntimeException )
{
    SfxBaseModel::dispose();
}

void SwXTextDocument::close( sal_Bool bDeliverOwnership ) throw( util::CloseVetoException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid() && m_pHiddenViewFrame)
        lcl_DisposeView( m_pHiddenViewFrame, pDocShell);
    SfxBaseModel::close(bDeliverOwnership);
}

void SwXTextDocument::addEventListener(const Reference< lang::XEventListener > & aListener) throw( RuntimeException )
{
    SfxBaseModel::addEventListener(aListener);
}

void SwXTextDocument::removeEventListener(const Reference< lang::XEventListener > & aListener) throw( RuntimeException )
{
    SfxBaseModel::removeEventListener(aListener);
}

Reference< XPropertySet > SwXTextDocument::getLineNumberingProperties(void)
            throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        if(!pxXLineNumberingProperties)
        {
            pxXLineNumberingProperties = new Reference<XPropertySet>;
            (*pxXLineNumberingProperties) = new SwXLineNumberingProperties(pDocShell->GetDoc());
        }
    }
    else
        throw RuntimeException();
    return *pxXLineNumberingProperties;
}

Reference< XIndexReplace >  SwXTextDocument::getChapterNumberingRules(void)
                                    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXChapterNumbering)
    {
        pxXChapterNumbering = new Reference< XIndexReplace > ;
        *pxXChapterNumbering = new SwXChapterNumbering(*pDocShell);
    }
    return *pxXChapterNumbering;
}

Reference< XIndexAccess >  SwXTextDocument::getNumberingRules(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXNumberingRules )
    {
        ((SwXTextDocument*)this)->pxXNumberingRules = new Reference< XIndexAccess > ;
        *pxXNumberingRules = new SwXNumberingRulesCollection( pDocShell->GetDoc() );
    }
    return *pxXNumberingRules;
}

Reference< XIndexAccess >  SwXTextDocument::getFootnotes(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXFootnotes)
    {
        ((SwXTextDocument*)this)->pxXFootnotes = new Reference< XIndexAccess > ;
        *pxXFootnotes = new SwXFootnotes(sal_False, pDocShell->GetDoc());
    }
    return *pxXFootnotes;
}

Reference< XPropertySet >  SAL_CALL
        SwXTextDocument::getFootnoteSettings(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXFootnoteSettings)
    {
        ((SwXTextDocument*)this)->pxXFootnoteSettings = new Reference< XPropertySet > ;
        *pxXFootnoteSettings = new SwXFootnoteProperties(pDocShell->GetDoc());
    }
    return *pxXFootnoteSettings;
}

Reference< XIndexAccess >  SwXTextDocument::getEndnotes(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXEndnotes)
    {
        ((SwXTextDocument*)this)->pxXEndnotes = new Reference< XIndexAccess > ;
        *pxXEndnotes = new SwXFootnotes(sal_True, pDocShell->GetDoc());
    }
    return *pxXEndnotes;
}

Reference< XPropertySet >  SwXTextDocument::getEndnoteSettings(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXEndnoteSettings)
    {
        ((SwXTextDocument*)this)->pxXEndnoteSettings = new Reference< XPropertySet > ;
        *pxXEndnoteSettings = new SwXEndnoteProperties(pDocShell->GetDoc());
    }
    return *pxXEndnoteSettings;
}

Reference< util::XReplaceDescriptor >  SwXTextDocument::createReplaceDescriptor(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< util::XReplaceDescriptor >  xRet = new SwXTextSearch;
    return xRet;
}

SwUnoCrsr*  SwXTextDocument::CreateCursorForSearch(Reference< XTextCursor > & xCrsr)
{
    getText();
    XText *const pText = xBodyText.get();
    SwXBodyText* pBText = (SwXBodyText*)pText;
    SwXTextCursor *const pXTextCursor = pBText->CreateTextCursor(true);
    xCrsr.set( static_cast<text::XWordCursor*>(pXTextCursor) );

    SwUnoCrsr *const pUnoCrsr = pXTextCursor->GetCursor();
    pUnoCrsr->SetRemainInSection(sal_False);
    return pUnoCrsr;
}

sal_Int32 SwXTextDocument::replaceAll(const Reference< util::XSearchDescriptor > & xDesc)
                                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY);
    if(!IsValid() || !xDescTunnel.is() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        throw RuntimeException();

    Reference< XTextCursor >  xCrsr;
    SwUnoCrsr*  pUnoCrsr = CreateCursorForSearch(xCrsr);

    const SwXTextSearch* pSearch = reinterpret_cast<const SwXTextSearch*>(
            xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()));

    int eRanges(FND_IN_BODY|FND_IN_SELALL);

    util::SearchOptions aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

    SwDocPositions eStart = pSearch->bBack ? DOCPOS_END : DOCPOS_START;
    SwDocPositions eEnd = pSearch->bBack ? DOCPOS_START : DOCPOS_END;

    // Search should take place anywhere
    pUnoCrsr->SetRemainInSection(sal_False);
    sal_uInt32 nResult;
    UnoActionContext aContext(pDocShell->GetDoc());
    //try attribute search first
    if(pSearch->HasSearchAttributes()||pSearch->HasReplaceAttributes())
    {
        SfxItemSet aSearch(pDocShell->GetDoc()->GetAttrPool(),
                            RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                            0);
        SfxItemSet aReplace(pDocShell->GetDoc()->GetAttrPool(),
                            RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                            RES_PARATR_BEGIN, RES_PARATR_END-1,
                            RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                            0);
        pSearch->FillSearchItemSet(aSearch);
        pSearch->FillReplaceItemSet(aReplace);
        sal_Bool bCancel;
        nResult = (sal_Int32)pUnoCrsr->Find( aSearch, !pSearch->bStyles,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges,
                    !pSearch->sSearchText.isEmpty() ? &aSearchOpt : 0,
                    &aReplace );
    }
    else if(pSearch->bStyles)
    {
        SwTxtFmtColl *pSearchColl = lcl_GetParaStyle(pSearch->sSearchText, pUnoCrsr->GetDoc());
        SwTxtFmtColl *pReplaceColl = lcl_GetParaStyle(pSearch->sReplaceText, pUnoCrsr->GetDoc());

        sal_Bool bCancel;
        nResult = pUnoCrsr->Find( *pSearchColl,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges, pReplaceColl );

    }
    else
    {
        //todo/mba: assuming that notes should be omitted
        sal_Bool bSearchInNotes = sal_False;
        sal_Bool bCancel;
        nResult = pUnoCrsr->Find( aSearchOpt, bSearchInNotes,
            eStart, eEnd, bCancel,
            (FindRanges)eRanges,
            sal_True );
    }
    return (sal_Int32)nResult;

}

Reference< util::XSearchDescriptor >  SwXTextDocument::createSearchDescriptor(void)
                                                    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< util::XSearchDescriptor >  xRet = new SwXTextSearch;
    return xRet;

}

// Used for findAll/First/Next

SwUnoCrsr*  SwXTextDocument::FindAny(const Reference< util::XSearchDescriptor > & xDesc,
                                        Reference< XTextCursor > & xCrsr, sal_Bool bAll,
                                                sal_Int32& nResult,
                                                Reference< XInterface >  xLastResult)
{
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY);
    if(!IsValid() || !xDescTunnel.is() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        return 0;

    SwUnoCrsr*  pUnoCrsr = CreateCursorForSearch(xCrsr);
    const SwXTextSearch* pSearch = reinterpret_cast<const SwXTextSearch*>(
        xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()));

    sal_Bool bParentInExtra = sal_False;
    if(xLastResult.is())
    {
        Reference<XUnoTunnel> xCursorTunnel( xLastResult, UNO_QUERY);
        OTextCursorHelper* pPosCrsr = 0;
        if(xCursorTunnel.is())
        {
            pPosCrsr = reinterpret_cast<OTextCursorHelper*>(xCursorTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
        }
        SwPaM* pCrsr = pPosCrsr ? pPosCrsr->GetPaM() : 0;
        if(pCrsr)
        {
            *pUnoCrsr->GetPoint() = *pCrsr->End();
            pUnoCrsr->DeleteMark();
        }
        else
        {
            SwXTextRange* pRange = 0;
            if(xCursorTunnel.is())
            {
                pRange = reinterpret_cast<SwXTextRange*>(xCursorTunnel->getSomething(
                                        SwXTextRange::getUnoTunnelId()));
            }
            if(!pRange)
                return 0;
            pRange->GetPositions(*pUnoCrsr);
            if(pUnoCrsr->HasMark())
            {
                if(*pUnoCrsr->GetPoint() < *pUnoCrsr->GetMark())
                    pUnoCrsr->Exchange();
                pUnoCrsr->DeleteMark();
            }
        }
        const SwNode* pRangeNode = pUnoCrsr->GetNode();
        bParentInExtra = pRangeNode->FindFlyStartNode() ||
                            pRangeNode->FindFootnoteStartNode() ||
                            pRangeNode->FindHeaderStartNode() ||
                            pRangeNode->FindFooterStartNode() ;
    }

    util::SearchOptions aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

/**
 * The following combinations are allowed:
 *  - Search in the body:                   -> FND_IN_BODY
 *  - Search all in the body:               -> FND_IN_BODYONLY | FND_IN_SELALL
 *  - Search in selections: one / all       -> FND_IN_SEL  [ | FND_IN_SELALL ]
 *  - Search outside the body: one / all    -> FND_IN_OTHER [ | FND_IN_SELALL ]
 *  - Search everywhere all:                -> FND_IN_SELALL
 */
    int eRanges(FND_IN_BODY);
    if(bParentInExtra)
        eRanges = FND_IN_OTHER;
    if(bAll) //always - everywhere?
        eRanges = FND_IN_SELALL;
    SwDocPositions eStart = !bAll ? DOCPOS_CURR : pSearch->bBack ? DOCPOS_END : DOCPOS_START;
    SwDocPositions eEnd = pSearch->bBack ? DOCPOS_START : DOCPOS_END;

    nResult = 0;
    sal_uInt16 nSearchProc = 0;
    while(nSearchProc < 2)
    {
        //try attribute search first
        if(pSearch->HasSearchAttributes())
        {
            SfxItemSet aSearch(pDocShell->GetDoc()->GetAttrPool(),
                                RES_CHRATR_BEGIN, RES_CHRATR_END-1,
                                RES_PARATR_BEGIN, RES_PARATR_END-1,
                                RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                                RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                                0);
            pSearch->FillSearchItemSet(aSearch);
            sal_Bool bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( aSearch, !pSearch->bStyles,
                        eStart, eEnd, bCancel,
                        (FindRanges)eRanges,
                        !pSearch->sSearchText.isEmpty() ? &aSearchOpt : 0,
                        0 );
        }
        else if(pSearch->bStyles)
        {
            SwTxtFmtColl *pSearchColl = lcl_GetParaStyle(pSearch->sSearchText, pUnoCrsr->GetDoc());
            //pSearch->sReplaceText
            SwTxtFmtColl *pReplaceColl = 0;
            sal_Bool bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( *pSearchColl,
                        eStart, eEnd, bCancel,
                        (FindRanges)eRanges, pReplaceColl );
        }
        else
        {
            //todo/mba: assuming that notes should be omitted
            sal_Bool bSearchInNotes = sal_False;
            sal_Bool bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( aSearchOpt, bSearchInNotes,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges,
                    /*int bReplace =*/sal_False );
        }
        nSearchProc++;
        if(nResult || (eRanges&(FND_IN_SELALL|FND_IN_OTHER)))
            break;
        //second step - find in other
        eRanges = FND_IN_OTHER;
    }
    return pUnoCrsr;
}

Reference< XIndexAccess >
    SwXTextDocument::findAll(const Reference< util::XSearchDescriptor > & xDesc)
                                                throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xTmp;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCrsr;
    SwUnoCrsr* pResultCrsr = FindAny(xDesc, xCrsr, sal_True, nResult, xTmp);
    if(!pResultCrsr)
        throw RuntimeException();
    Reference< XIndexAccess >  xRet;
    xRet = new SwXTextRanges( (nResult) ? pResultCrsr : 0 );
    delete pResultCrsr;
    return xRet;
}

Reference< XInterface >  SwXTextDocument::findFirst(const Reference< util::XSearchDescriptor > & xDesc)
                                            throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xTmp;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCrsr;
    SwUnoCrsr* pResultCrsr = FindAny(xDesc, xCrsr, sal_False, nResult, xTmp);
    if(!pResultCrsr)
        throw RuntimeException();
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(*pDocShell->GetDoc(),
                    *pResultCrsr->GetPoint());
        xRet = *new SwXTextCursor(xParent, *pResultCrsr);
        delete pResultCrsr;
    }
    return xRet;
}

Reference< XInterface >  SwXTextDocument::findNext(const Reference< XInterface > & xStartAt,
            const Reference< util::XSearchDescriptor > & xDesc)
            throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xTmp;
    sal_Int32 nResult = 0;
    Reference< XTextCursor >  xCrsr;
    if(!xStartAt.is())
        throw RuntimeException();
    SwUnoCrsr* pResultCrsr = FindAny(xDesc, xCrsr, sal_False, nResult, xStartAt);
    if(!pResultCrsr)
        throw RuntimeException();
    Reference< XInterface >  xRet;
    if(nResult)
    {
        const uno::Reference< text::XText >  xParent =
            ::sw::CreateParentXText(*pDocShell->GetDoc(),
                    *pResultCrsr->GetPoint());

        xRet = *new SwXTextCursor(xParent, *pResultCrsr);
        delete pResultCrsr;
    }
    return xRet;
}

Sequence< beans::PropertyValue > SwXTextDocument::getPagePrintSettings(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence< beans::PropertyValue > aSeq(9);
    if(IsValid())
    {
        beans::PropertyValue* pArray = aSeq.getArray();
        SwPagePreViewPrtData aData;
        const SwPagePreViewPrtData* pData = pDocShell->GetDoc()->GetPreViewPrtData();
        if(pData)
            aData = *pData;
        Any aVal;
        aVal <<= (sal_Int16)aData.GetRow();
        pArray[0] = beans::PropertyValue("PageRows", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int16)aData.GetCol();
        pArray[1] = beans::PropertyValue("PageColumns", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetLeftSpace());
        pArray[2] = beans::PropertyValue("LeftMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetRightSpace());
        pArray[3] = beans::PropertyValue("RightMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetTopSpace());
        pArray[4] = beans::PropertyValue("TopMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetBottomSpace());
        pArray[5] = beans::PropertyValue("BottomMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetHorzSpace());
        pArray[6] = beans::PropertyValue("HoriMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(aData.GetVertSpace());
        pArray[7] = beans::PropertyValue("VertMargin", -1, aVal, PropertyState_DIRECT_VALUE);
        sal_Bool bTemp = aData.GetLandscape();
        aVal.setValue(&bTemp, ::getCppuBooleanType());
        pArray[8] = beans::PropertyValue("IsLandscape", -1, aVal, PropertyState_DIRECT_VALUE);
    }
    else
        throw RuntimeException();
    return aSeq;
}

static sal_uInt32 lcl_Any_To_ULONG(const Any& rValue, sal_Bool& bException)
{
    bException = sal_False;
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

static String lcl_CreateOutlineString( sal_uInt16 nIndex,
            const SwOutlineNodes& rOutlineNodes, const SwNumRule* pOutlRule)
{
    String sEntry;
    const SwTxtNode * pTxtNd = rOutlineNodes[ nIndex ]->GetTxtNode();
    SwNumberTree::tNumberVector aNumVector = pTxtNd->GetNumberVector();
    if( pOutlRule && pTxtNd->GetNumRule())
        for( sal_Int8 nLevel = 0;
             nLevel <= pTxtNd->GetActualListLevel();
             nLevel++ )
        {
            long nVal = aNumVector[nLevel];
            nVal ++;
            nVal -= pOutlRule->Get(nLevel).GetStart();
            sEntry += OUString::number( nVal );
            sEntry += '.';
        }
    sEntry += rOutlineNodes[ nIndex ]->
                    GetTxtNode()->GetExpandTxt( 0, STRING_LEN, sal_False );
    return sEntry;
}

void SwXTextDocument::setPagePrintSettings(const Sequence< beans::PropertyValue >& aSettings)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        SwPagePreViewPrtData aData;
        //if only a few properties are coming, then use the current settings
        const SwPagePreViewPrtData* pData = pDocShell->GetDoc()->GetPreViewPrtData();
        if(pData)
            aData = *pData;
        const beans::PropertyValue* pProperties = aSettings.getConstArray();
        int nCount = aSettings.getLength();
        for(int i = 0; i < nCount; i++)
        {
            String sName = pProperties[i].Name;
            const Any& rVal = pProperties[i].Value;
            sal_Bool bException;
            sal_uInt32 nVal = lcl_Any_To_ULONG(rVal, bException);
            if( COMPARE_EQUAL == sName.CompareToAscii("PageRows" ) )
            {
                if(!nVal || nVal > 0xff)
                    throw RuntimeException();
                aData.SetRow((sal_uInt8)nVal);
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("PageColumns"))
            {
                if(!nVal  || nVal > 0xff)
                    throw RuntimeException();
                aData.SetCol((sal_uInt8)nVal);
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("LeftMargin"))
            {
                aData.SetLeftSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("RightMargin"))
            {
                aData.SetRightSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("TopMargin"))
            {
                aData.SetTopSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("BottomMargin"))
            {
                aData.SetBottomSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("HoriMargin"))
            {
                aData.SetHorzSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("VertMargin"))
            {
                aData.SetVertSpace(MM100_TO_TWIP_UNSIGNED(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("IsLandscape"))
            {
                bException =  (::getBooleanCppuType() != rVal.getValueType());
                aData.SetLandscape(*(sal_Bool*)rVal.getValue());
            }
            else
                bException = sal_True;
            if(bException)
                throw RuntimeException();
        }
        pDocShell->GetDoc()->SetPreViewPrtData(&aData);
    }
    else
        throw RuntimeException();
}

void SwXTextDocument::printPages(const Sequence< beans::PropertyValue >& xOptions)
    throw( IllegalArgumentException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        SfxViewFrame* pFrame = SfxViewFrame::LoadHiddenDocument( *pDocShell, 7 );
        SfxRequest aReq(FN_PRINT_PAGEPREVIEW, SFX_CALLMODE_SYNCHRON,
                                    pDocShell->GetDoc()->GetAttrPool());
            aReq.AppendItem(SfxBoolItem(FN_PRINT_PAGEPREVIEW, sal_True));

        OUString sFileName(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_FILE_NAME)));
        OUString sCopyCount(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_COPY_COUNT)));
        OUString sCollate(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_COLLATE)));
        OUString sSort(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SORT)));
        OUString sPages(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_PAGES)));

        for ( int n = 0; n < xOptions.getLength(); ++n )
        {
            // get Property-Value from options
            const beans::PropertyValue &rProp = xOptions.getConstArray()[n];
            Any aValue( rProp.Value );

            // FileName-Property?
            if ( rProp.Name == sFileName )
            {
                OUString sFileURL;
                if ( (rProp.Value >>= sFileURL ) )
                {
                    // Convert the File URL into a system dependant path, as the SalPrinter expects
                    OUString sSystemPath;
                    FileBase::getSystemPathFromFileURL ( sFileURL, sSystemPath );
                    aReq.AppendItem(SfxStringItem( SID_FILE_NAME, sSystemPath ) );
                }
                else if ( rProp.Value.getValueType() != ::getVoidCppuType() )
                    throw IllegalArgumentException();
            }

            // CopyCount-Property
            else if ( rProp.Name == sCopyCount )
            {
                sal_Int32 nCopies = 0;
                aValue >>= nCopies;
                aReq.AppendItem(SfxInt16Item( SID_PRINT_COPIES, (sal_Int16)nCopies ) );
            }

            // Collate-Property
            else if ( rProp.Name == sCollate )
            {
                if ( rProp.Value.getValueType() == ::getBooleanCppuType())

                    aReq.AppendItem(SfxBoolItem( SID_PRINT_COLLATE, *(sal_Bool*)rProp.Value.getValue() ) );
                else
                    throw IllegalArgumentException();
            }

            // Sort-Property
            else if ( rProp.Name == sSort )
            {
                if ( rProp.Value.getValueType() == ::getBooleanCppuType() )
                    aReq.AppendItem(SfxBoolItem( SID_PRINT_SORT, *(sal_Bool*)rProp.Value.getValue() ) );
                else
                    throw IllegalArgumentException();
            }

            // Pages-Property
            else if ( rProp.Name == sPages )
            {
                OUString sTmp;
                if ( rProp.Value >>= sTmp )
                    aReq.AppendItem( SfxStringItem( SID_PRINT_PAGES, sTmp ) );
                else
                    throw IllegalArgumentException();
            }
        }

        // #i117783#
        bApplyPagePrintSettingsFromXPagePrintable = sal_True;
        pFrame->GetViewShell()->ExecuteSlot(aReq);
        // Frame close
        pFrame->DoClose();

    }
    else
        throw RuntimeException();
}

Reference< XNameAccess >  SwXTextDocument::getReferenceMarks(void)
                                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXReferenceMarks)
    {
        ((SwXTextDocument*)this)->pxXReferenceMarks = new Reference< XNameAccess > ;
        *pxXReferenceMarks = new SwXReferenceMarks(pDocShell->GetDoc());
    }
    return *pxXReferenceMarks;
}

Reference< XEnumerationAccess >  SwXTextDocument::getTextFields(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXTextFieldTypes)
    {
        ((SwXTextDocument*)this)->pxXTextFieldTypes = new Reference< XEnumerationAccess > ;
        *pxXTextFieldTypes = new SwXTextFieldTypes(pDocShell->GetDoc());
    }
    return *pxXTextFieldTypes;
}

Reference< XNameAccess >  SwXTextDocument::getTextFieldMasters(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXTextFieldMasters)
    {
        ((SwXTextDocument*)this)->pxXTextFieldMasters = new Reference< XNameAccess > ;
        *pxXTextFieldMasters = new SwXTextFieldMasters(pDocShell->GetDoc());
    }
    return *pxXTextFieldMasters;
}

Reference< XNameAccess >  SwXTextDocument::getEmbeddedObjects(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXEmbeddedObjects)
    {
        ((SwXTextDocument*)this)->pxXEmbeddedObjects = new Reference< XNameAccess > ;
        *pxXEmbeddedObjects = new SwXTextEmbeddedObjects(pDocShell->GetDoc());
    }
    return *pxXEmbeddedObjects;
}

Reference< XNameAccess >  SwXTextDocument::getBookmarks(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXBookmarks)
    {
        ((SwXTextDocument*)this)->pxXBookmarks = new Reference< XNameAccess > ;
        *pxXBookmarks = new SwXBookmarks(pDocShell->GetDoc());
    }
    return *pxXBookmarks;
}

Reference< XNameAccess >  SwXTextDocument::getTextSections(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXTextSections)
    {
        ((SwXTextDocument*)this)->pxXTextSections = new Reference< XNameAccess > ;
        *pxXTextSections = new SwXTextSections(pDocShell->GetDoc());
    }
    return *pxXTextSections;
}

Reference< XNameAccess >  SwXTextDocument::getTextTables(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXTextTables)
    {
        ((SwXTextDocument*)this)->pxXTextTables = new Reference< XNameAccess > ;
        *pxXTextTables = new SwXTextTables(pDocShell->GetDoc());
    }
    return *pxXTextTables;
}

Reference< XNameAccess >  SwXTextDocument::getGraphicObjects(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXGraphicObjects)
    {
        ((SwXTextDocument*)this)->pxXGraphicObjects = new Reference< XNameAccess > ;
        *pxXGraphicObjects = new SwXTextGraphicObjects(pDocShell->GetDoc());
    }
    return *pxXGraphicObjects;
}

Reference< XNameAccess >  SwXTextDocument::getTextFrames(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXTextFrames)
    {
        ((SwXTextDocument*)this)->pxXTextFrames = new Reference< XNameAccess > ;
        *pxXTextFrames = new SwXTextFrames(pDocShell->GetDoc());
    }
    return *pxXTextFrames;
}

Reference< XNameAccess >  SwXTextDocument::getStyleFamilies(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXStyleFamilies)
    {
        ((SwXTextDocument*)this)->pxXStyleFamilies = new Reference< XNameAccess > ;
        *pxXStyleFamilies = new SwXStyleFamilies(*pDocShell);
    }
    return *pxXStyleFamilies;
}

uno::Reference< style::XAutoStyles > SwXTextDocument::getAutoStyles(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXAutoStyles)
    {
        pxXAutoStyles = new Reference< style::XAutoStyles > ;
        *pxXAutoStyles = new SwXAutoStyles(*pDocShell);
    }
    return *pxXAutoStyles;

}

Reference< drawing::XDrawPage >  SwXTextDocument::getDrawPage(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXDrawPage)
    {
        ((SwXTextDocument*)this)->pDrawPage = new SwXDrawPage(pDocShell->GetDoc());
        ((SwXTextDocument*)this)->pxXDrawPage = new Reference< drawing::XDrawPage >(pDrawPage);
        // Create a Reference to trigger the complete initialization of the
        // object. Otherwise in some corner cases it would get initialized
        // at ::InitNewDoc -> which would get called during
        // close() or dispose() -> n#681746
        uno::Reference<lang::XComponent> xComp( *pxXDrawPage, uno::UNO_QUERY );
    }
    return *pxXDrawPage;
}

SwXDrawPage* SwXTextDocument::GetDrawPage()
{
    if(!IsValid())
        return 0;
    if(!pDrawPage)
        getDrawPage();
    return pDrawPage;
}

void SwXTextDocument::Invalidate()
{
    bObjectValid = sal_False;
    if(xNumFmtAgg.is())
    {
        const uno::Type& rTunnelType = ::getCppuType((Reference <XUnoTunnel>*)0);
        Any aNumTunnel = xNumFmtAgg->queryAggregation(rTunnelType);
        SvNumberFormatsSupplierObj* pNumFmt = 0;
        Reference< XUnoTunnel > xNumTunnel;
        if(aNumTunnel >>= xNumTunnel)
        {
            pNumFmt = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));
            pNumFmt->SetNumberFormatter(0);
        }
        OSL_ENSURE(pNumFmt, "No number formatter available");
    }
    InitNewDoc();
    pDocShell = 0;
    lang::EventObject const ev(static_cast<SwXTextDocumentBaseClass &>(*this));
    m_pImpl->m_RefreshListeners.disposeAndClear(ev);
}

void SwXTextDocument::Reactivate(SwDocShell* pNewDocShell)
{
    if(pDocShell && pDocShell != pNewDocShell)
        Invalidate();
    pDocShell = pNewDocShell;
    bObjectValid = sal_True;
}

void    SwXTextDocument::InitNewDoc()
{
    // first invalidate all collections, then delete references and Set to zero
    if(pxXTextTables)
    {
         XNameAccess* pTbls = pxXTextTables->get();
        ((SwXTextTables*)pTbls)->Invalidate();
        delete pxXTextTables;
        pxXTextTables = 0;
    }

    if(pxXTextFrames)
    {
         XNameAccess* pFrms = pxXTextFrames->get();
        ((SwXTextFrames*)pFrms)->Invalidate();
        delete pxXTextFrames;
        pxXTextFrames = 0;
    }

    if(pxXGraphicObjects)
    {
         XNameAccess* pFrms = pxXGraphicObjects->get();
        ((SwXTextGraphicObjects*)pFrms)->Invalidate();
        delete pxXGraphicObjects;
        pxXGraphicObjects = 0;
    }

    if(pxXEmbeddedObjects)
    {
     XNameAccess* pOLE = pxXEmbeddedObjects->get();
        ((SwXTextEmbeddedObjects*)pOLE)->Invalidate();
        delete pxXEmbeddedObjects;
        pxXEmbeddedObjects = 0;
    }

    if(xBodyText.is())
    {
        xBodyText = 0;
        pBodyText = 0;
    }

    if(xNumFmtAgg.is())
    {
        const uno::Type& rTunnelType = ::getCppuType((Reference <XUnoTunnel>*)0);
        Any aNumTunnel = xNumFmtAgg->queryAggregation(rTunnelType);
        SvNumberFormatsSupplierObj* pNumFmt = 0;
        Reference< XUnoTunnel > xNumTunnel;
        if(aNumTunnel >>= xNumTunnel)
        {
            pNumFmt = reinterpret_cast<SvNumberFormatsSupplierObj*>(
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));

        }
        OSL_ENSURE(pNumFmt, "No number formatter available");
        pNumFmt->SetNumberFormatter(0);
    }

    if(pxXTextFieldTypes)
    {
         XEnumerationAccess* pT = pxXTextFieldTypes->get();
        ((SwXTextFieldTypes*)pT)->Invalidate();
        delete pxXTextFieldTypes;
        pxXTextFieldTypes = 0;
    }

    if(pxXTextFieldMasters)
    {
         XNameAccess* pT = pxXTextFieldMasters->get();
        ((SwXTextFieldMasters*)pT)->Invalidate();
        delete pxXTextFieldMasters;
        pxXTextFieldMasters = 0;
    }

    if(pxXTextSections)
    {
         XNameAccess* pSect = pxXTextSections->get();
        ((SwXTextSections*)pSect)->Invalidate();
        delete pxXTextSections;
        pxXTextSections = 0;
    }

    if(pxXDrawPage)
    {
        // #i91798#, #i91895#
        // dispose XDrawPage here. We are the owner and know that it is no longer in a valid condition.
        uno::Reference<lang::XComponent> xComp( *pxXDrawPage, uno::UNO_QUERY );
        xComp->dispose();
        pDrawPage->InvalidateSwDoc();
        delete pxXDrawPage;
        pxXDrawPage = 0;
    }

    if ( pxXNumberingRules )
    {
        XIndexAccess* pNum = pxXNumberingRules->get();
        ((SwXNumberingRulesCollection*)pNum)->Invalidate();
        delete pxXNumberingRules;
        pxXNumberingRules = 0;
    }

    if(pxXFootnotes)
    {
         XIndexAccess* pFtn = pxXFootnotes->get();
        ((SwXFootnotes*)pFtn)->Invalidate();
        delete pxXFootnotes;
        pxXFootnotes = 0;
    }

    if(pxXEndnotes)
    {
         XIndexAccess* pFtn = pxXEndnotes->get();
        ((SwXFootnotes*)pFtn)->Invalidate();
        delete pxXEndnotes;
        pxXEndnotes = 0;
    }

    if(pxXDocumentIndexes)
    {
         XIndexAccess* pIdxs = pxXDocumentIndexes->get();
        ((SwXDocumentIndexes*)pIdxs)->Invalidate();
        delete pxXDocumentIndexes;
        pxXDocumentIndexes = 0;
    }

    if(pxXStyleFamilies)
    {
         XNameAccess* pStyles = pxXStyleFamilies->get();
        ((SwXStyleFamilies*)pStyles)->Invalidate();
        delete pxXStyleFamilies;
        pxXStyleFamilies = 0;
    }
    if(pxXAutoStyles)
    {
         XNameAccess* pStyles = pxXAutoStyles->get();
        ((SwXAutoStyles*)pStyles)->Invalidate();
        delete pxXAutoStyles;
        pxXAutoStyles = 0;
    }

    if(pxXBookmarks)
    {
         XNameAccess* pBm = pxXBookmarks->get();
        ((SwXBookmarks*)pBm)->Invalidate();
        delete pxXBookmarks;
        pxXBookmarks = 0;
    }

    if(pxXChapterNumbering)
    {
         XIndexReplace* pCh = pxXChapterNumbering->get();
        ((SwXChapterNumbering*)pCh)->Invalidate();
        delete pxXChapterNumbering;
        pxXChapterNumbering = 0;
    }

    if(pxXFootnoteSettings)
    {
         XPropertySet* pFntSet = pxXFootnoteSettings->get();
        ((SwXFootnoteProperties*)pFntSet)->Invalidate();
        delete pxXFootnoteSettings;
        pxXFootnoteSettings = 0;
    }

    if(pxXEndnoteSettings)
    {
         XPropertySet* pEndSet = pxXEndnoteSettings->get();
        ((SwXEndnoteProperties*)pEndSet)->Invalidate();
        delete pxXEndnoteSettings;
        pxXEndnoteSettings = 0;
    }

    if(pxXLineNumberingProperties)
    {
         XPropertySet* pLine = pxXLineNumberingProperties->get();
        ((SwXLineNumberingProperties*)pLine)->Invalidate();
        delete pxXLineNumberingProperties;
        pxXLineNumberingProperties = 0;
    }
    if(pxXReferenceMarks)
    {
         XNameAccess* pMarks = pxXReferenceMarks->get();
        ((SwXReferenceMarks*)pMarks)->Invalidate();
        delete pxXReferenceMarks;
        pxXReferenceMarks = 0;
    }
    if(pxLinkTargetSupplier)
    {
         XNameAccess* pAccess = (*pxLinkTargetSupplier).get();
        ((SwXLinkTargetSupplier*)pAccess)->Invalidate();
        delete pxLinkTargetSupplier;
        pxLinkTargetSupplier = 0;
    }
    if(pxXRedlines)
    {
        XEnumerationAccess* pMarks = pxXRedlines->get();
        ((SwXRedlines*)pMarks)->Invalidate();
        delete pxXRedlines;
        pxXRedlines = 0;
    }
    if(xPropertyHelper.is())
    {
        pPropertyHelper->Invalidate();
        xPropertyHelper = 0;
        pPropertyHelper = 0;
    }
}

#define COM_SUN_STAR__DRAWING_LENGTH 13
Reference< XInterface >  SwXTextDocument::createInstance(const OUString& rServiceName)
                                        throw( Exception, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    Reference< XInterface >  xRet;
    sal_uInt16 nType = SwXServiceProvider::GetProviderType(rServiceName);
    if(nType != SW_SERVICE_INVALID)
    {
        xRet = SwXServiceProvider::MakeInstance(nType, pDocShell->GetDoc());
    }
    else
    {
        if( rServiceName.startsWith("com.sun.star.") )
        {
            sal_Int32 nIndex = COM_SUN_STAR__DRAWING_LENGTH;
            OUString sCategory = rServiceName.getToken( 0, '.', nIndex );
            bool bShape = sCategory == "drawing";
            if( bShape || sCategory == "form")
            {
                if(bShape)
                {
                    short nTable = 0;
                    if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.DashTable") ) )
                        nTable = SW_CREATE_DASH_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GradientTable") ) )
                        nTable = SW_CREATE_GRADIENT_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.HatchTable") ) )
                        nTable = SW_CREATE_HATCH_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.BitmapTable") ) )
                        nTable = SW_CREATE_BITMAP_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TransparencyGradientTable") ) )
                        nTable = SW_CREATE_TRANSGRADIENT_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MarkerTable") ) )
                        nTable = SW_CREATE_MARKER_TABLE;
                    else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Defaults") ) )
                        nTable = SW_CREATE_DRAW_DEFAULTS;
                    if(nTable)
                    {
                        xRet = GetPropertyHelper()->GetDrawTable(nTable);
                    }
                }
            }
            else if (sCategory == "document" )
            {
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.Settings") ) )
                    xRet = Reference < XInterface > ( *new SwXDocumentSettings ( this ) );
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.ImportEmbeddedObjectResolver") ) )
                {
                    xRet = (::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pDocShell, EMBEDDEDOBJECTHELPER_MODE_READ );
                }
            }
            else if (sCategory == "text" )
            {
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.DocumentSettings") ) )
                    xRet = Reference < XInterface > ( *new SwXDocumentSettings ( this ) );
            }
            else if (sCategory == "chart2" )
            {
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.chart2.data.DataProvider") ) )
                    xRet = Reference < XInterface > ( dynamic_cast< chart2::data::XDataProvider * >(pDocShell->getIDocumentChartDataProviderAccess()->GetChartDataProvider()) );
            }

            if(!xRet.is())
            {
                //! we don't want to insert OLE2 Shapes (e.g. "com.sun.star.drawing.OLE2Shape", ...)
                //! like this (by creating them with the documents factory and
                //! adding the shapes to the draw page).
                //! For inserting OLE objects the proper way is to use
                //! "com.sun.star.text.TextEmbeddedObject"!
                if (rServiceName.lastIndexOf( ".OLE2Shape" ) == rServiceName.getLength() - 10)
                    throw ServiceNotRegisteredException();  // declare service to be not registered with this factory

                //
                // the XML import is allowed to create instances of com.sun.star.drawing.OLE2Shape.
                // Thus, a temporary service name is introduced to make this possible.
                OUString aTmpServiceName( rServiceName );
                if ( bShape &&
                     rServiceName.compareToAscii( "com.sun.star.drawing.temporaryForXMLImportOLE2Shape" ) == 0 )
                {
                    aTmpServiceName = OUString("com.sun.star.drawing.OLE2Shape");
                }
                //here search for the draw service
                Reference< XInterface >  xTmp = SvxFmMSFactory::createInstance(aTmpServiceName);
                if(bShape)
                {
                    nIndex = COM_SUN_STAR__DRAWING_LENGTH;
                    if( 0 == rServiceName.reverseCompareToAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.drawing.GroupShape" ) ) ||
                        0 == rServiceName.reverseCompareToAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.drawing.Shape3DSceneObject" ) ) )
                        xRet = *new SwXGroupShape( xTmp );
                    else
                        xRet = *new SwXShape( xTmp );
                }
                else
                    xRet = xTmp;
            }
        }
        else
            throw ServiceNotRegisteredException();
    }
    return xRet;
}

Reference< XInterface >  SwXTextDocument::createInstanceWithArguments(
        const OUString& ServiceSpecifier,
        const Sequence< Any >& /*Arguments*/)
        throw( Exception, RuntimeException )
{
    Reference< XInterface >  xInt = createInstance(ServiceSpecifier);
    // The Any-Sequence is for initializing objects that are dependent
    // on parameters necessarily - so far we have not.
    return xInt;
}

Sequence< OUString > SwXTextDocument::getAvailableServiceNames(void)
                                        throw( RuntimeException )
{
    static Sequence< OUString > aServices;
    if ( aServices.getLength() == 0 )
    {
        Sequence< OUString > aRet =  SvxFmMSFactory::getAvailableServiceNames();
        OUString* pRet = aRet.getArray();
        for ( sal_Int32 i = 0; i < aRet.getLength(); ++i )
        {
            if ( pRet[i].compareToAscii( "com.sun.star.drawing.OLE2Shape" ) == 0 )
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

OUString SwXTextDocument::getImplementationName(void) throw( RuntimeException )
{
    return OUString("SwXTextDocument");
}

sal_Bool SwXTextDocument::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    if ( rServiceName == "com.sun.star.document.OfficeDocument" || rServiceName == "com.sun.star.text.GenericTextDocument" )
        return sal_True;

    bool bWebDoc    = (0 != PTR_CAST(SwWebDocShell,    pDocShell));
    bool bGlobalDoc = (0 != PTR_CAST(SwGlobalDocShell, pDocShell));
    bool bTextDoc   = (!bWebDoc && !bGlobalDoc);

    return ( (bWebDoc    && rServiceName == "com.sun.star.text.WebDocument")
          || (bGlobalDoc && rServiceName == "com.sun.star.text.GlobalDocument")
          || (bTextDoc   && rServiceName == "com.sun.star.text.TextDocument") );
}

Sequence< OUString > SwXTextDocument::getSupportedServiceNames(void) throw( RuntimeException )
{
    bool bWebDoc    = (0 != PTR_CAST(SwWebDocShell,    pDocShell));
    bool bGlobalDoc = (0 != PTR_CAST(SwGlobalDocShell, pDocShell));
    bool bTextDoc   = (!bWebDoc && !bGlobalDoc);

    Sequence< OUString > aRet (3);
    OUString* pArray = aRet.getArray();

    pArray[0] = OUString ( ( "com.sun.star.document.OfficeDocument"  ) );
    pArray[1] = OUString ( ( "com.sun.star.text.GenericTextDocument" ) );

    if (bTextDoc)
        pArray[2] = OUString ( ( "com.sun.star.text.TextDocument" ) );
    else if (bWebDoc)
        pArray[2] = OUString ( ( "com.sun.star.text.WebDocument" ) );
    else if (bGlobalDoc)
        pArray[2] = OUString ( ( "com.sun.star.text.GlobalDocument" ) );

    return aRet;
}

Reference< XIndexAccess >  SwXTextDocument::getDocumentIndexes(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXDocumentIndexes)
    {
        ((SwXTextDocument*)this)->pxXDocumentIndexes = new Reference< XIndexAccess > ;
        *pxXDocumentIndexes = new SwXDocumentIndexes(pDocShell->GetDoc());
    }
    return *pxXDocumentIndexes;
}

Reference< XPropertySetInfo >  SwXTextDocument::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXTextDocument::setPropertyValue(const OUString& rPropertyName,
    const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
                                         WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException(
            "invalid SwXTextDocument",
            static_cast< cppu::OWeakObject * >(
                static_cast< SwXTextDocumentBaseClass * >(this)));
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
            sal_Bool bSet = *(sal_Bool*)aValue.getValue();
            sal_uInt16 eMode = pDocShell->GetDoc()->GetRedlineMode();
            if(WID_DOC_CHANGES_SHOW == pEntry->nWID)
            {
                eMode &= ~(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE);
                eMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT;
                if( bSet )
                    eMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;
            }
            else if(WID_DOC_CHANGES_RECORD == pEntry->nWID)
            {
                eMode = bSet ? eMode|nsRedlineMode_t::REDLINE_ON : eMode&~nsRedlineMode_t::REDLINE_ON;
            }
            pDocShell->GetDoc()->SetRedlineMode( (RedlineMode_t)(eMode ));
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(aValue >>= aNew)
            {
                SwDoc* pDoc = pDocShell->GetDoc();
                pDoc->SetRedlinePassword(aNew);
                if(aNew.getLength())
                {
                    sal_uInt16 eMode = pDoc->GetRedlineMode();
                    eMode = eMode|nsRedlineMode_t::REDLINE_ON;
                    pDoc->SetRedlineMode( (RedlineMode_t)(eMode ));
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
            SW_MOD()->GetModuleConfig()->SetHideFieldTips(*(sal_Bool*)aValue.getValue());
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            sal_Int16 eRedMode = pDocShell->GetDoc()->GetRedlineMode();
            eRedMode = eRedMode & (~nsRedlineMode_t::REDLINE_SHOW_MASK);
            sal_Int16 nSet = 0;
            aValue >>= nSet;
            switch(nSet)
            {
                case RedlineDisplayType::NONE: break;
                case RedlineDisplayType::INSERTED: eRedMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT; break;
                case RedlineDisplayType::REMOVED: eRedMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;  break;
                case RedlineDisplayType::
                        INSERTED_AND_REMOVED: eRedMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT|nsRedlineMode_t::REDLINE_SHOW_DELETE;
                break;
                default: throw IllegalArgumentException();
            }
            pDocShell->GetDoc()->SetRedlineMode(eRedMode);
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            sal_Int16 nYear = 0;
            aValue >>= nYear;
            SfxRequest aRequest ( SID_ATTR_YEAR2000, SFX_CALLMODE_SLOT, pDocShell->GetDoc()->GetAttrPool());
            aRequest.AppendItem(SfxUInt16Item( SID_ATTR_YEAR2000, static_cast < sal_uInt16 > ( nYear ) ) );
            pDocShell->Execute ( aRequest );
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bAuto = *(sal_Bool*) aValue.getValue();

            if ( 0 != ( pDrawDoc = dynamic_cast< SwDrawDocument * >( pDocShell->GetDoc()->GetDrawModel() ) ) )
                pDrawDoc->SetAutoControlFocus( bAuto );
            else if (bAuto)
            {
                // if setting to true, and we don't have an
                // SdrModel, then we are changing the default and
                // must thus create an SdrModel, if we don't have an
                // SdrModel and we are leaving the default at false,
                // we don't need to make an SdrModel and can do nothing
                // #i52858# - method name changed
                pDrawDoc = dynamic_cast< SwDrawDocument * > (pDocShell->GetDoc()->GetOrCreateDrawModel() );
                pDrawDoc->SetAutoControlFocus ( bAuto );
            }
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bMode = *(sal_Bool*)aValue.getValue();

            if ( 0 != ( pDrawDoc = dynamic_cast< SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
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
                pDrawDoc = dynamic_cast< SwDrawDocument * > (pDocShell->GetDoc()->GetOrCreateDrawModel() );
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

        default:
        {
            const SfxPoolItem& rItem = pDocShell->GetDoc()->GetDefault(pEntry->nWID);
            SfxPoolItem* pNewItem = rItem.Clone();
            pNewItem->PutValue(aValue, pEntry->nMemberId);
            pDocShell->GetDoc()->SetDefault(*pNewItem);
            delete pNewItem;
        }
    }
}

Any SwXTextDocument::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
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
            const SwDocStat& rStat(pDocShell->GetDoc()->GetUpdatedDocStat());
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
            sal_uInt16 eMode = pDocShell->GetDoc()->GetRedlineMode();
            sal_Bool bSet = sal_False;
            if(WID_DOC_CHANGES_SHOW == pEntry->nWID)
            {
                sal_uInt16 nMask = nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE;
                bSet = (eMode & nMask) == nMask;
            }
            else if(WID_DOC_CHANGES_RECORD == pEntry->nWID)
            {
                bSet = (eMode& nsRedlineMode_t::REDLINE_ON)  != 0;
            }
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            SwDoc* pDoc = pDocShell->GetDoc();
            aAny <<= pDoc->GetRedlinePassword();
        }
        break;
        case WID_DOC_AUTO_MARK_URL :
            aAny <<= pDocShell->GetDoc()->GetTOIAutoMarkURL();
        break;
        case WID_DOC_HIDE_TIPS :
        {
            sal_Bool bTemp = SW_MOD()->GetModuleConfig()->IsHideFieldTips();
            aAny.setValue(&bTemp, ::getBooleanCppuType());
        }
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            sal_Int16 eRedMode = pDocShell->GetDoc()->GetRedlineMode();
            eRedMode = eRedMode & nsRedlineMode_t::REDLINE_SHOW_MASK;
            sal_Int16 nRet = RedlineDisplayType::NONE;
            if(nsRedlineMode_t::REDLINE_SHOW_INSERT == eRedMode)
                nRet = RedlineDisplayType::INSERTED;
            else if(nsRedlineMode_t::REDLINE_SHOW_DELETE == eRedMode)
                nRet = RedlineDisplayType::REMOVED;
            else if(nsRedlineMode_t::REDLINE_SHOW_MASK == eRedMode)
                nRet = RedlineDisplayType::INSERTED_AND_REMOVED;
            aAny <<= nRet;
        }
        break;
        case WID_DOC_FORBIDDEN_CHARS:
        {
            GetPropertyHelper();
            Reference<XForbiddenCharacters> xRet(xPropertyHelper, UNO_QUERY);
            aAny <<= xRet;
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            aAny <<= static_cast < sal_Int16 > (pDocShell->GetDoc()->GetNumberFormatter ( sal_True )->GetYear2000());
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bAuto;
            if ( 0 != ( pDrawDoc = dynamic_cast< SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
                bAuto = pDrawDoc->GetAutoControlFocus();
            else
                bAuto = sal_False;
            aAny.setValue(&bAuto, ::getBooleanCppuType());
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bMode;
            if ( 0 != ( pDrawDoc = dynamic_cast< SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
                bMode = pDrawDoc->GetOpenInDesignMode();
            else
                bMode = sal_True;
            aAny.setValue(&bMode, ::getBooleanCppuType());
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
            beans::PropertyValue aProp;
            aProp.Name = OUString("ThisWordDoc");
            aProp.Value <<= pDocShell->GetModel();
            aAny <<= aProp;
        }
        break;
        case WID_DOC_RUNTIME_UID:
            aAny <<= getRuntimeUID();
        break;
        case WID_DOC_LOCK_UPDATES :
            aAny <<= static_cast<bool>( pDocShell->GetDoc()->IsInReading() );
        break;
        case WID_DOC_BUILDID:
            aAny <<= maBuildId;
        break;
        case WID_DOC_HAS_VALID_SIGNATURES:
            aAny <<= hasValidSignatures();
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
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextDocument::removeVetoableChangeListener(const OUString& /*PropertyName*/,
                        const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    OSL_FAIL("not implemented");
}

Reference< XNameAccess >  SwXTextDocument::getLinks(void) throw( RuntimeException )
{
    if(!pxLinkTargetSupplier)
    {
        pxLinkTargetSupplier = new Reference< XNameAccess > ;
        (*pxLinkTargetSupplier) = new SwXLinkTargetSupplier(*(SwXTextDocument*)this);
    }
    return (*pxLinkTargetSupplier);
}

Reference< XEnumerationAccess > SwXTextDocument::getRedlines(  ) throw(RuntimeException)
{
    if(!pxXRedlines)
    {
        pxXRedlines = new Reference< XEnumerationAccess > ;
        (*pxXRedlines) = new SwXRedlines(pDocShell->GetDoc());
    }
    return *pxXRedlines;
}

void SwXTextDocument::NotifyRefreshListeners()
{
    // why does SwBaseShell not just call refresh? maybe because it's rSh is
    // (sometimes) a different shell than GetWrtShell()?
    lang::EventObject const ev(static_cast<SwXTextDocumentBaseClass &>(*this));
    m_pImpl->m_RefreshListeners.notifyEach(
            & util::XRefreshListener::refreshed, ev);
}

void SwXTextDocument::refresh(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    ViewShell *pViewShell = pDocShell->GetWrtShell();
    NotifyRefreshListeners();
    if(pViewShell)
        pViewShell->CalcLayout();
}

void SAL_CALL SwXTextDocument::addRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
throw (RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.addInterface(xListener);
}

void SAL_CALL SwXTextDocument::removeRefreshListener(
        const Reference<util::XRefreshListener> & xListener)
throw (RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_RefreshListeners.removeInterface(xListener);
}

void SwXTextDocument::updateLinks(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    SwDoc* pDoc = pDocShell->GetDoc();
      sfx2::LinkManager& rLnkMan = pDoc->GetLinkManager();
    if( !rLnkMan.GetLinks().empty() )
    {
        UnoActionContext aAction(pDoc);
        rLnkMan.UpdateAllLinks( false, false, true );
    }
}

//XPropertyState
PropertyState SAL_CALL SwXTextDocument::getPropertyState( const OUString& rPropertyName )
    throw (UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    PropertyState eRet = PropertyState_DIRECT_VALUE;
    if(!IsValid())
        throw RuntimeException();
    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);

    if(!pEntry)
        throw UnknownPropertyException();
    Any aAny;
    switch(pEntry->nWID)
    {
        case 0:default:break;
    }
    return eRet;
}

Sequence< PropertyState > SAL_CALL SwXTextDocument::getPropertyStates( const Sequence< OUString >& rPropertyNames )
    throw (UnknownPropertyException, RuntimeException)
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
    throw (UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    const SfxItemPropertySimpleEntry*  pEntry = pPropSet->getPropertyMap().getByName( rPropertyName);
    if(!pEntry)
        throw UnknownPropertyException();
    switch(pEntry->nWID)
    {
        case 0:default:break;
    }
}

Any SAL_CALL SwXTextDocument::getPropertyDefault( const OUString& rPropertyName )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
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

static OutputDevice * lcl_GetOutputDevice( const SwPrintUIOptions &rPrintUIOptions )
{
    OutputDevice *pOut = 0;

    uno::Any aAny( rPrintUIOptions.getValue( "RenderDevice" ));
    uno::Reference< awt::XDevice >  xRenderDevice;
    aAny >>= xRenderDevice;
    if (xRenderDevice.is())
    {
        VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        pOut = pDevice ? pDevice->GetOutputDevice() : 0;
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
    SfxViewShell *pView = 0;
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
    SwDoc *pDoc = 0;

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

            bool bIsSwSrcView = false;
            if (!rpView)
            {
                (void) bIsPDFExport;
                // aside from maybe PDF export the view should always have been provided!
                OSL_ENSURE( bIsPDFExport, "view is missing, guessing one..." );

                rpView = GuessViewShell( bIsSwSrcView );
            }
            OSL_ENSURE( rpView, "ViewShell missing" );
            // the view shell should be SwView for documents PDF export.
            // for the page preview no selection should be possible
            // (the export dialog does not allow for this option)
            const TypeId aSwViewTypeId = TYPE(SwView);
            if (rpView  &&  rpView->IsA(aSwViewTypeId))
            {
                if (!m_pRenderData)
                {
                    OSL_FAIL("GetRenderDoc: no renderdata");
                    return 0;
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
                    pDoc = ((SwDocShell*)&xDocSh)->GetDoc();
                    rpView = pDoc->GetDocShell()->GetView();
                }
            }
            else
            {
                OSL_FAIL("unexpected ViewShell" );
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
    SwPrintData aDocPrintData( rDoc.getPrintData() );

    aDocPrintData.SetPrintGraphic( rPrintUIOptions.IsPrintGraphics() );
    aDocPrintData.SetPrintTable( rPrintUIOptions.IsPrintTables() );
    aDocPrintData.SetPrintDraw( rPrintUIOptions.IsPrintDrawings() );
    aDocPrintData.SetPrintControl( rPrintUIOptions.IsPrintFormControls() );
    aDocPrintData.SetPrintLeftPage( rPrintUIOptions.IsPrintLeftPages() );
    aDocPrintData.SetPrintRightPage( rPrintUIOptions.IsPrintRightPages() );
    aDocPrintData.SetPrintReverse( rPrintUIOptions.IsPrintReverse() );
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

    rDoc.setPrintData( aDocPrintData );
}

sal_Int32 SAL_CALL SwXTextDocument::getRendererCount(
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
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
        SwSrcView *pSwSrcView = dynamic_cast< SwSrcView * >(pView);
        OutputDevice *pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
        nRet = pSwSrcView->PrintSource( pOutDev, 1 /* dummy */, true /* get page count only */ );
    }
    else
    {
        SwDocShell *pRenderDocShell = pDoc->GetDocShell();

        // TODO/mba: we really need a generic way to get the ViewShell!
        ViewShell* pViewShell = 0;
        SwView* pSwView = PTR_CAST(SwView, pView);
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
                m_pHiddenViewFrame = SfxViewFrame::LoadHiddenDocument( *pRenderDocShell, 2 );
                pViewShell = ((SwView*)m_pHiddenViewFrame->GetViewShell())->GetWrtShellPtr();
            }
            else
                pViewShell = ((SwPagePreView*)pView)->GetViewShell();
        }

        if (!pViewShell || !pViewShell->GetLayout())
            return 0;

        if (bFormat)
        {
            // #i38289
            if( pViewShell->GetViewOptions()->getBrowseMode() )
            {
                SwViewOption aOpt( *pViewShell->GetViewOptions() );
                aOpt.setBrowseMode( false );
                pViewShell->ApplyViewOptions( aOpt );
                if (pSwView)
                {
                    pSwView->RecheckBrowseMode();
                }
            }

            // reformating the document for printing will show the changes in the view
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
                const SwPrintData *pPrtOptions = (bIsPDFExport)
                    ? NULL : m_pRenderData->GetSwPrtOptions();
                m_pRenderData->ViewOptionAdjust( pPrtOptions );
            }

            // since printing now also use the API for PDF export this option
            // should be set for printing as well ...
            pViewShell->SetPDFExportOption( sal_True );
            bool bOrigStatus = pRenderDocShell->IsEnableSetModified();
            // check configuration: shall update of printing information in DocInfo set the document to "modified"?
            bool bStateChanged = false;
            if ( bOrigStatus && !SvtPrintWarningOptions().IsModifyDocumentOnPrintingAllowed() )
            {
                pRenderDocShell->EnableSetModified( sal_False );
                bStateChanged = true;
            }

            // #122919# Force field update before PDF export
            pViewShell->ViewShell::UpdateFlds(sal_True);
            if( bStateChanged )
                pRenderDocShell->EnableSetModified( sal_True );

            // there is some redundancy between those two function calls, but right now
            // there is no time to sort this out.
            //TODO: check what exatly needs to be done and make just one function for that
            pViewShell->CalcLayout();
            pViewShell->CalcPagesForPrint( pViewShell->GetPageCount() );

            pViewShell->SetPDFExportOption( sal_False );

            // enable view again
            pViewShell->EndAction();
        }

        const sal_Int32 nPageCount = pViewShell->GetPageCount();

        //
        // get number of pages to be rendered
        //
        const bool bPrintProspect = m_pPrintUIOptions->getBoolValue( "PrintProspect", false );
        if (bPrintProspect)
        {
            pDoc->CalculatePagePairsForProspectPrinting( *pViewShell->GetLayout(), *m_pRenderData, *m_pPrintUIOptions, nPageCount );
            nRet = m_pRenderData->GetPagePairsForProspectPrinting().size();
        }
        else
        {
            const sal_Int16 nPostItMode = (sal_Int16) m_pPrintUIOptions->getIntValue( "PrintAnnotationMode", 0 );
            if (nPostItMode != POSTITS_NONE)
            {
                OutputDevice *pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
                m_pRenderData->CreatePostItData( pDoc, pViewShell->GetViewOptions(), pOutDev );
            }

            // get set of valid document pages (according to the current settings)
            // and their start frames
            pDoc->CalculatePagesForPrinting( *pViewShell->GetLayout(), *m_pRenderData, *m_pPrintUIOptions, bIsPDFExport, nPageCount );

            if (nPostItMode != POSTITS_NONE)
            {
                pDoc->UpdatePagesForPrintingWithPostItData( *m_pRenderData,
                        *m_pPrintUIOptions, bIsPDFExport, nPageCount );
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
    throw (IllegalArgumentException, RuntimeException)
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
    const bool bPrintProspect    = m_pPrintUIOptions->getBoolValue( "PrintProspect", false );
    const bool bIsSkipEmptyPages = !m_pPrintUIOptions->IsPrintEmptyPages( bIsPDFExport );
    const bool bPrintPaperFromSetup = m_pPrintUIOptions->getBoolValue( "PrintPaperFromSetup", false );

    SwDoc *pDoc = GetRenderDoc( pView, rSelection, bIsPDFExport );
    OSL_ENSURE( pDoc && pView, "doc or view shell missing!" );
    if (!pDoc || !pView)
        return uno::Sequence< beans::PropertyValue >();

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (0 > nRenderer)
        throw IllegalArgumentException();

    // TODO/mba: we really need a generic way to get the ViewShell!
    ViewShell* pVwSh = 0;
    SwView* pSwView = PTR_CAST(SwView, pView);
    if ( pSwView )
        pVwSh = pSwView->GetWrtShellPtr();
    else
        pVwSh = ((SwPagePreView*)pView)->GetViewShell();

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
        const sal_uInt16 nPage = bPrintProspect
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
            Printer *pPrinter = dynamic_cast< Printer * >(lcl_GetOutputDevice( *m_pPrintUIOptions ));
            if (pPrinter)
            {
                // HTML source view and prospect adapt to the printer's paper size
                aTmpSize = pPrinter->GetPaperSize();
                aTmpSize = pPrinter->LogicToLogic( aTmpSize,
                            pPrinter->GetMapMode(), MapMode( MAP_100TH_MM ));
                aPageSize = awt::Size( aTmpSize.Width(), aTmpSize.Height() );
                #if 0
                // #i115048# it seems users didn't like getting double the formatted page size
                // revert to "old" behavior scaling to the current paper size of the printer
                if (bPrintProspect)
                {
                    // we just state what output size we would need
                    // which may cause vcl to set that page size on the printer
                    // (if available and not overriden by the user)
                    aTmpSize = pVwSh->GetPageSize( nPage, bIsSkipEmptyPages );
                    aPreferredPageSize = awt::Size ( TWIP_TO_MM100( 2 * aTmpSize.Width() ),
                                                     TWIP_TO_MM100( aTmpSize.Height() ));
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
            aPageSize = awt::Size ( TWIP_TO_MM100( aTmpSize.Width() ),
                                    TWIP_TO_MM100( aTmpSize.Height() ));
        }

        sal_Int32 nLen = 2;
        aRenderer.realloc(2);
        aRenderer[0].Name  = OUString( "PageSize" );
        aRenderer[0].Value <<= aPageSize;
        aRenderer[1].Name  = OUString( "PageIncludesNonprintableArea" );
        aRenderer[1].Value <<= sal_True;
        if (aPreferredPageSize.Width && aPreferredPageSize.Height)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            aRenderer[ nLen - 1 ].Name  = OUString( "PreferredPageSize" );
            aRenderer[ nLen - 1 ].Value <<= aPreferredPageSize;
        }
        if (nPrinterPaperTray >= 0)
        {
            ++nLen;
            aRenderer.realloc( nLen );
            aRenderer[ nLen - 1 ].Name  = OUString( "PrinterPaperTray" );
            aRenderer[ nLen - 1 ].Value <<= nPrinterPaperTray;
        }
    }

    // #i117783#
    if ( bApplyPagePrintSettingsFromXPagePrintable )
    {
        const SwPagePreViewPrtData* pPagePrintSettings =
                                        pDocShell->GetDoc()->GetPreViewPrtData();
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
            aRenderer[ nRenderDataIdxStart + 0 ].Name  = OUString( "NUpRows" );
            aRenderer[ nRenderDataIdxStart + 0 ].Value <<= ( nRow > 1 ? nRow : 1 );
            const sal_Int32 nCol = pPagePrintSettings->GetCol();
            aRenderer[ nRenderDataIdxStart + 1 ].Name  = OUString( "NUpColumns" );
            aRenderer[ nRenderDataIdxStart + 1 ].Value <<= ( nCol > 1 ? nCol : 1 );
            aRenderer[ nRenderDataIdxStart + 2 ].Name  = OUString( "NUpPageMarginLeft" );
            aRenderer[ nRenderDataIdxStart + 2 ].Value <<= pPagePrintSettings->GetLeftSpace();
            aRenderer[ nRenderDataIdxStart + 3 ].Name  = OUString( "NUpPageMarginRight" );
            aRenderer[ nRenderDataIdxStart + 3 ].Value <<= pPagePrintSettings->GetRightSpace();
            aRenderer[ nRenderDataIdxStart + 4 ].Name  = OUString( "NUpPageMarginTop" );
            aRenderer[ nRenderDataIdxStart + 4 ].Value <<= pPagePrintSettings->GetTopSpace();
            aRenderer[ nRenderDataIdxStart + 5 ].Name  = OUString( "NUpPageMarginBottom" );
            aRenderer[ nRenderDataIdxStart + 5 ].Value <<= pPagePrintSettings->GetBottomSpace();
            aRenderer[ nRenderDataIdxStart + 6 ].Name  = OUString( "NUpHorizontalSpacing" );
            aRenderer[ nRenderDataIdxStart + 6 ].Value <<= pPagePrintSettings->GetHorzSpace();
            aRenderer[ nRenderDataIdxStart + 7 ].Name  = OUString( "NUpVerticalSpacing" );
            aRenderer[ nRenderDataIdxStart + 7 ].Value <<= pPagePrintSettings->GetVertSpace();
            {
                Printer* pPrinter = pDocShell->GetDoc()->getPrinter( false );
                if ( pPrinter )
                {
                    awt::Size aNewPageSize;
                    const Size aPageSize = pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) );
                    aNewPageSize = awt::Size( aPageSize.Width(), aPageSize.Height() );
                    if ( ( pPagePrintSettings->GetLandscape() &&
                           aPageSize.Width() < aPageSize.Height() ) ||
                         ( !pPagePrintSettings->GetLandscape() &&
                           aPageSize.Width() > aPageSize.Height() ) )
                    {
                        aNewPageSize = awt::Size( aPageSize.Height(), aPageSize.Width() );
                    }
                    aRenderer[ nRenderDataIdxStart + 8 ].Name  = OUString( "NUpPaperSize" );
                    aRenderer[ nRenderDataIdxStart + 8 ].Value <<= aNewPageSize;
                }
            }
        }

        bApplyPagePrintSettingsFromXPagePrintable = sal_False;
    }

    m_pPrintUIOptions->appendPrintUIOptions( aRenderer );

    return aRenderer;
}

SfxViewShell * SwXTextDocument::GuessViewShell(
    /* out */ bool &rbIsSwSrcView,
    const uno::Reference< css::frame::XController > xController )
{
    // #130810# SfxViewShell::Current() / SfxViewShell::GetObjectShell()
    // must not be used (see comment from MBA)
    //
    SfxViewShell    *pView = 0;
    SwView          *pSwView = 0;
    SwPagePreView   *pSwPagePreView = 0;
    SwSrcView       *pSwSrcView = 0;
    SfxViewFrame    *pFrame = SfxViewFrame::GetFirst( pDocShell, sal_False );

    // look for the view shell with the same controller in use,
    // otherwise look for a suitable view, preferably a SwView,
    // if that one is not found use a SwPagePreView if found.
    while (pFrame)
    {
        pView = pFrame->GetViewShell();
        pSwView = dynamic_cast< SwView * >(pView);
        pSwSrcView = dynamic_cast< SwSrcView * >(pView);
        if (!pSwPagePreView)
            pSwPagePreView = dynamic_cast< SwPagePreView * >(pView);
        if (xController.is())
        {
            if (pView && pView->GetController() == xController)
                break;
        }
        else if (pSwView || pSwSrcView)
            break;
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell,  sal_False );
    }

    OSL_ENSURE( pSwView || pSwPagePreView || pSwSrcView, "failed to get view shell" );
    if (pView)
        rbIsSwSrcView = pSwSrcView != 0;
    return pView;
}

void SAL_CALL SwXTextDocument::render(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
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
    const bool bPrintProspect   = m_pPrintUIOptions->getBoolValue( "PrintProspect", false );
    const bool bLastPage        = m_pPrintUIOptions->getBoolValue( "IsLastPage", sal_False );

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
                SwSrcView *pSwSrcView = dynamic_cast< SwSrcView * >(pView);
                OutputDevice *pOutDev = lcl_GetOutputDevice( *m_pPrintUIOptions );
                pSwSrcView->PrintSource( pOutDev, nRenderer + 1, false );
            }
            else
            {
                // the view shell should be SwView for documents PDF export
                // or SwPagePreView for PDF export of the page preview
                //!! (check for SwView first as in GuessViewShell) !!
                OSL_ENSURE( pView, "!! view missing !!" );
                const TypeId aSwViewTypeId = TYPE(SwView);
                ViewShell* pVwSh = 0;
                if (pView)
                {
                    // TODO/mba: we really need a generic way to get the ViewShell!
                    SwView* pSwView = PTR_CAST(SwView, pView);
                    if ( pSwView )
                        pVwSh = pSwView->GetWrtShellPtr();
                    else
                        pVwSh = ((SwPagePreView*)pView)->GetViewShell();
                }

                // get output device to use
                OutputDevice * pOut = lcl_GetOutputDevice( *m_pPrintUIOptions );

                if(pVwSh && pOut && m_pRenderData->HasSwPrtOptions())
                {
                    const OUString aPageRange  = m_pPrintUIOptions->getStringValue( "PageRange", OUString() );
                    const bool bFirstPage           = m_pPrintUIOptions->getBoolValue( "IsFirstPage", sal_False );
                    bool bIsSkipEmptyPages          = !m_pPrintUIOptions->IsPrintEmptyPages( bIsPDFExport );

                    OSL_ENSURE(( pView->IsA(aSwViewTypeId) &&  m_pRenderData->IsViewOptionAdjust())
                            || (!pView->IsA(aSwViewTypeId) && !m_pRenderData->IsViewOptionAdjust()),
                            "SwView / SwViewOptionAdjust_Impl availability mismatch" );

                    // since printing now also use the API for PDF export this option
                    // should be set for printing as well ...
                    pVwSh->SetPDFExportOption( sal_True );

                    // #i12836# enhanced pdf export
                    //
                    // First, we have to export hyperlinks, notes, and outline to pdf.
                    // During this process, additional information required for tagging
                    // the pdf file are collected, which are evaulated during painting.
                    //
                    SwWrtShell* pWrtShell = pView->IsA(aSwViewTypeId) ?
                                            ((SwView*)pView)->GetWrtShellPtr() :
                                            0;

                    if (bIsPDFExport && bFirstPage && pWrtShell)
                    {
                        SwEnhancedPDFExportHelper aHelper( *pWrtShell, *pOut, aPageRange, bIsSkipEmptyPages, sal_False );
                    }

                    SwPrintData const& rSwPrtOptions =
                        *m_pRenderData->GetSwPrtOptions();
                    if (bPrintProspect)
                        pVwSh->PrintProspect( pOut, rSwPrtOptions, nRenderer );
                    else    // normal printing and PDF export
                        pVwSh->PrintOrPDFExport( pOut, rSwPrtOptions, nRenderer );

                    // #i35176#
                    //
                    // After printing the last page, we take care for the links coming
                    // from the EditEngine. The links are generated during the painting
                    // process, but the destinations are still missing.
                    //
                    if (bIsPDFExport && bLastPage && pWrtShell)
                    {
                        SwEnhancedPDFExportHelper aHelper( *pWrtShell, *pOut, aPageRange, bIsSkipEmptyPages,  sal_True );
                    }

                    pVwSh->SetPDFExportOption( sal_False );

                    // last page to be rendered? (not necessarily the last page of the document)
                    // -> do clean-up of data
                    if (bLastPage)
                    {
                        // #i96167# haggai: delete ViewOptionsAdjust here because it makes use
                        // of the shell, which might get destroyed in lcl_DisposeView!
                        if (m_pRenderData && m_pRenderData->IsViewOptionAdjust())
                            m_pRenderData->ViewOptionAdjustStop();

                        if (m_pRenderData && m_pRenderData->HasPostItData())
                            m_pRenderData->DeletePostItData();
                        if (m_pHiddenViewFrame)
                        {
                            lcl_DisposeView( m_pHiddenViewFrame, pDocShell );
                            m_pHiddenViewFrame = 0;

                            // prevent crash described in #i108805
                            SwDocShell *pRenderDocShell = pDoc->GetDocShell();
                            SfxItemSet *pSet = pRenderDocShell->GetMedium()->GetItemSet();
                            pSet->Put( SfxBoolItem( SID_HIDDEN, sal_False ) );

                        }
                    }
                }
            }
        }
    }
    if( bLastPage )
    {
        delete m_pRenderData;       m_pRenderData     = NULL;
        delete m_pPrintUIOptions;   m_pPrintUIOptions = NULL;
    }
}

// xforms::XFormsSupplier
Reference<XNameContainer> SAL_CALL SwXTextDocument::getXForms()
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !pDocShell )
        throw DisposedException( OUString(), static_cast< XTextDocument* >( this ) );
    SwDoc* pDoc = pDocShell->GetDoc();
    return pDoc->getXForms();
}

uno::Reference< text::XFlatParagraphIterator > SAL_CALL SwXTextDocument::getFlatParagraphIterator(::sal_Int32 nTextMarkupType, sal_Bool bAutomatic)
    throw ( uno::RuntimeException )
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

uno::Reference< util::XCloneable > SwXTextDocument::createClone(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    // create a new document - hidden - copy the storage and return it
    // SfxObjectShellRef is used here, since the model should control object lifetime after creation
    // and thus SfxObjectShellLock is not allowed here
    // the model holds reference to the shell, so the shell will not destructed at the end of method
    SfxObjectShellRef pShell = pDocShell->GetDoc()->CreateCopy(false);
    uno::Reference< frame::XModel > xNewModel = pShell->GetModel();
    uno::Reference< embed::XStorage > xNewStorage = ::comphelper::OStorageHelper::GetTemporaryStorage( );
    uno::Sequence< beans::PropertyValue > aTempMediaDescriptor;
    storeToStorage( xNewStorage, aTempMediaDescriptor );
    uno::Reference< document::XStorageBasedDocument > xStorageDoc( xNewModel, uno::UNO_QUERY );
    xStorageDoc->loadFromStorage( xNewStorage, aTempMediaDescriptor );
    return uno::Reference< util::XCloneable >( xNewModel, UNO_QUERY );
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
    throw (lang::IllegalArgumentException, uno::RuntimeException)
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

    const SwCharFmts *pFmts = pDoc->GetCharFmts();
    for(sal_uInt16 i = 0; i < pFmts->size(); ++i)
    {
        const SwAttrSet &rAttrSet = (*pFmts)[i]->GetAttrSet();
        LanguageType nLang = LANGUAGE_DONTKNOW;
        if (bLatin)
        {
            nLang = rAttrSet.GetLanguage( sal_False ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bAsian)
        {
            nLang = rAttrSet.GetCJKLanguage( sal_False ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bComplex)
        {
            nLang = rAttrSet.GetCTLLanguage( sal_False ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
    }

    const SwTxtFmtColls *pColls = pDoc->GetTxtFmtColls();
    for (sal_uInt16 i = 0; i < pColls->size(); ++i)
    {
        const SwAttrSet &rAttrSet = (*pColls)[i]->GetAttrSet();
        LanguageType nLang = LANGUAGE_DONTKNOW;
        if (bLatin)
        {
            nLang = rAttrSet.GetLanguage( sal_False ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bAsian)
        {
            nLang = rAttrSet.GetCJKLanguage( sal_False ).GetLanguage();
            if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                aAllLangs.insert( nLang );
        }
        if (bComplex)
        {
            nLang = rAttrSet.GetCTLLanguage( sal_False ).GetLanguage();
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
    for (sal_uInt16 i = 0; i < 2; ++i)
    {
        std::vector< SfxItemSet_Pointer_t > rStyles;
        pDoc->GetIStyleAccess().getAllStyles(rStyles, aFam[i]);
        while (!rStyles.empty())
        {
            SfxItemSet_Pointer_t pStyle = rStyles.back();
            rStyles.pop_back();
            const SfxItemSet *pSet = dynamic_cast< const SfxItemSet * >(pStyle.get());

            LanguageType nLang = LANGUAGE_DONTKNOW;
            if (bLatin)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_LANGUAGE, sal_False )).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bAsian)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_CJK_LANGUAGE, sal_False )).GetLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aAllLangs.insert( nLang );
            }
            if (bComplex)
            {
                nLang = dynamic_cast< const SvxLanguageItem & >(pSet->Get( RES_CHRATR_CTL_LANGUAGE, sal_False )).GetLanguage();
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
            EditEngine& rEditEng = (EditEngine&)pOutliner->GetEditEngine();
            sal_Int32 nParCount = pOutliner->GetParagraphCount();
            for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
            {
                //every paragraph
                std::vector<sal_uInt16> aPortions;
                rEditEng.GetPortions( nPar, aPortions );

                for ( size_t nPos = aPortions.size(); nPos; )
                {
                    //every position
                    --nPos;
                    sal_uInt16 nEnd = aPortions[ nPos ];
                    sal_uInt16 nStart = nPos ? aPortions[ nPos - 1 ] : 0;
                    ESelection aSelection( nPar, nStart, nPar, nEnd );
                    SfxItemSet aAttr = rEditEng.GetAttribs( aSelection );

                    LanguageType nLang = LANGUAGE_DONTKNOW;
                    if (bLatin)
                    {
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE, sal_False )).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bAsian)
                    {
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE_CJK, sal_False )).GetLanguage();
                        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                            aAllLangs.insert( nLang );
                    }
                    if (bComplex)
                    {
                        nLang = dynamic_cast< const SvxLanguageItem & >(aAttr.Get( EE_CHAR_LANGUAGE_CTL, sal_False )).GetLanguage();
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
        const SvtLanguageTable aLangTab;
        for (std::set< LanguageType >::const_iterator it = aAllLangs.begin(); it != aAllLangs.end(); ++it)
        {
            if (nCount >= nMaxCount)
                break;
            if (LANGUAGE_NONE != *it)
            {
                pLanguage[nCount] = LanguageTag::convertToLocale( *it );
                pLanguage[nCount].Language = aLangTab.GetString( *it );
                nCount += 1;
            }
        }
    }

    return aLanguages;
}

SwXLinkTargetSupplier::SwXLinkTargetSupplier(SwXTextDocument& rxDoc) :
    pxDoc(&rxDoc)
{
    sTables     = String(SW_RES(STR_CONTENT_TYPE_TABLE));
    sFrames     = String(SW_RES(STR_CONTENT_TYPE_FRAME));
    sGraphics   = String(SW_RES(STR_CONTENT_TYPE_GRAPHIC));
    sOLEs       = String(SW_RES(STR_CONTENT_TYPE_OLE));
    sSections   = String(SW_RES(STR_CONTENT_TYPE_REGION));
    sOutlines   = String(SW_RES(STR_CONTENT_TYPE_OUTLINE));
    sBookmarks  = String(SW_RES(STR_CONTENT_TYPE_BOOKMARK));
}

SwXLinkTargetSupplier::~SwXLinkTargetSupplier()
{
}

Any SwXLinkTargetSupplier::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    if(!pxDoc)
        throw RuntimeException();
    String sToCompare(rName);
    String sSuffix = OUString('|');
    if(sToCompare == sTables)
    {
        sSuffix += "table";

        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextTables(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference<XPropertySet>*)0));
    }
    else if(sToCompare == sFrames)
    {
        sSuffix += "frame";
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextFrames(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sSections)
    {
        sSuffix += "region";
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextSections(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sGraphics)
    {
        sSuffix += "graphic";
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getGraphicObjects(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sOLEs)
    {
        sSuffix += "ole";
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getEmbeddedObjects(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sOutlines)
    {
        sSuffix += "outline";
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        *pxDoc, sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sBookmarks)
    {
        sSuffix.Erase();
        Reference< XNameAccess >  xBkms = new SwXLinkNameAccessWrapper(
                                        pxDoc->getBookmarks(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xBkms, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else
        throw NoSuchElementException();
    return aRet;
}

Sequence< OUString > SwXLinkTargetSupplier::getElementNames(void)
                                        throw( RuntimeException )
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
                                    throw( RuntimeException )
{
    String sToCompare(rName);
    if( sToCompare == sTables  ||
        sToCompare == sFrames  ||
        sToCompare == sGraphics||
        sToCompare == sOLEs   ||
        sToCompare == sSections ||
        sToCompare == sOutlines ||
        sToCompare == sBookmarks    )
        return sal_True;
    return sal_False;
}

uno::Type  SwXLinkTargetSupplier::getElementType(void)
                                    throw( RuntimeException )
{
    return ::getCppuType((Reference< XPropertySet>*)0);

}

sal_Bool SwXLinkTargetSupplier::hasElements(void) throw( RuntimeException )
{
    return 0 != pxDoc;
}

OUString SwXLinkTargetSupplier::getImplementationName(void) throw( RuntimeException )
{
    return OUString("SwXLinkTargetSupplier");
}

sal_Bool SwXLinkTargetSupplier::supportsService(const OUString& rServiceName)
                                                throw( RuntimeException )
{
    return (rServiceName == "com.sun.star.document.LinkTargets");
}

Sequence< OUString > SwXLinkTargetSupplier::getSupportedServiceNames(void)
                                                throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pNames = aRet.getArray();
    pNames[0] = "com.sun.star.document.LinkTargets";
    return aRet;
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(
            Reference< XNameAccess >  xAccess, const OUString& rLinkDisplayName, OUString sSuffix ) :
    xRealAccess(xAccess),
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    sLinkSuffix(sSuffix),
    sLinkDisplayName(rLinkDisplayName),
    pxDoc(0)
{
}

SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const OUString& rLinkDisplayName, OUString sSuffix) :
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_LINK_TARGET)),
    sLinkSuffix(sSuffix),
    sLinkDisplayName(rLinkDisplayName),
    xDoc(&rxDoc),
    pxDoc(&rxDoc)
{
}

SwXLinkNameAccessWrapper::~SwXLinkNameAccessWrapper()
{
}

Any SwXLinkNameAccessWrapper::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    bool bFound = false;
    //cut link extension and call the real NameAccess
    String sParam = rName;
    String sSuffix(sLinkSuffix);
    if(sParam.Len() > sSuffix.Len() )
    {
        String sCmp = sParam.Copy(sParam.Len() - sSuffix.Len(),
                                                    sSuffix.Len());
        if(sCmp == sSuffix)
        {
            if(pxDoc)
            {
                sParam = sParam.Copy(0, sParam.Len() - sSuffix.Len());
                if(!pxDoc->GetDocShell())
                    throw RuntimeException();
                SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
                sal_uInt16 nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                for (sal_uInt16 i = 0; i < nOutlineCount && !bFound; ++i)
                {
                    const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
                    const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
                    if(sParam ==
                        lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule))
                    {
                        Reference< XPropertySet >  xOutline = new SwXOutlineTarget(sParam);
                        aRet.setValue(&xOutline, ::getCppuType((Reference<XPropertySet>*)0));
                        bFound = true;
                    }
                }
            }
            else
            {
                aRet = xRealAccess->getByName(sParam.Copy(0, sParam.Len() - sSuffix.Len()));
                Reference< XInterface > xInt;
                if(!(aRet >>= xInt))
                    throw RuntimeException();
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

Sequence< OUString > SwXLinkNameAccessWrapper::getElementNames(void)
                                    throw( RuntimeException )
{
    Sequence< OUString > aRet;
    if(pxDoc)
    {
        if(!pxDoc->GetDocShell())
            throw RuntimeException();

        SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
        const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
        sal_uInt16 nOutlineCount = rOutlineNodes.size();
        aRet.realloc(nOutlineCount);
        OUString* pResArr = aRet.getArray();
        String sSuffix = OUString('|');
        sSuffix += OUString::createFromAscii("outline");
        const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
        for (sal_uInt16 i = 0; i < nOutlineCount; ++i)
        {
            String sEntry = lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule);
            sEntry += sSuffix;
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
    throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
    String sParam(rName);
    if(sParam.Len() > sLinkSuffix.getLength() )
    {
        String sCmp = sParam.Copy(sParam.Len() - sLinkSuffix.getLength(),
                                                    sLinkSuffix.getLength());
        if(sCmp == sLinkSuffix)
        {
            sParam = sParam.Copy(0, sParam.Len() - sLinkSuffix.getLength());
            if(pxDoc)
            {
                if(!pxDoc->GetDocShell())
                    throw RuntimeException();
                SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
                sal_uInt16 nOutlineCount = pDoc->GetNodes().GetOutLineNds().size();

                for (sal_uInt16 i = 0; i < nOutlineCount && !bRet; ++i)
                {
                    const SwOutlineNodes& rOutlineNodes = pDoc->GetNodes().GetOutLineNds();
                    const SwNumRule* pOutlRule = pDoc->GetOutlineNumRule();
                    if(sParam ==
                        lcl_CreateOutlineString(i, rOutlineNodes, pOutlRule))
                    {
                        bRet = sal_True;
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

uno::Type  SwXLinkNameAccessWrapper::getElementType(void)
                                throw( RuntimeException )
{
    return ::getCppuType((Reference<XPropertySet>*)0);
}

sal_Bool SwXLinkNameAccessWrapper::hasElements(void) throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
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

Reference< XPropertySetInfo >  SwXLinkNameAccessWrapper::getPropertySetInfo(void)
                                        throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXLinkNameAccessWrapper::setPropertyValue(
    const OUString& , const Any& )
    throw( UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
             WrappedTargetException,
            RuntimeException)
{
    throw UnknownPropertyException();
}

static Any lcl_GetDisplayBitmap(OUString sLinkSuffix)
{
    Any aRet;
    if(!sLinkSuffix.isEmpty())
        sLinkSuffix = sLinkSuffix.copy(1);
    sal_uInt16 nImgId = USHRT_MAX;

    if(sLinkSuffix == "outline")
        nImgId = CONTENT_TYPE_OUTLINE;
    else if(sLinkSuffix == "table")
        nImgId = CONTENT_TYPE_TABLE;
    else if(sLinkSuffix == "frame")
        nImgId = CONTENT_TYPE_FRAME;
    else if(sLinkSuffix == "graphic")
        nImgId = CONTENT_TYPE_GRAPHIC;
    else if(sLinkSuffix == "region")
        nImgId = CONTENT_TYPE_REGION;
    else if(sLinkSuffix == "ole")
        nImgId = CONTENT_TYPE_OLE;
    else if(sLinkSuffix.isEmpty())
        nImgId = CONTENT_TYPE_BOOKMARK;
    if(USHRT_MAX != nImgId)
    {
        nImgId += 20000;
        ImageList aEntryImages( SW_RES(IMG_NAVI_ENTRYBMP) );
        const Image& rImage = aEntryImages.GetImage( nImgId );
        Bitmap aBitmap( rImage.GetBitmapEx().GetBitmap() );
        Reference<awt::XBitmap> xBmp = VCLUnoHelper::CreateBitmap( aBitmap );
        aRet.setValue( &xBmp, ::getCppuType((Reference<awt::XBitmap>*)0) );
    }
    return aRet;
}

Any SwXLinkNameAccessWrapper::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
    {
        aRet <<= OUString(sLinkDisplayName);
    }
    else if( rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_BITMAP)))
    {
        aRet = lcl_GetDisplayBitmap(sLinkSuffix);
    }
    else
        throw UnknownPropertyException();
    return aRet;
}

void SwXLinkNameAccessWrapper::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}

void SwXLinkNameAccessWrapper::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}

void SwXLinkNameAccessWrapper::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}

void SwXLinkNameAccessWrapper::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}

Reference< XNameAccess >  SwXLinkNameAccessWrapper::getLinks(void)
                                    throw( RuntimeException )
{
    return (SwXLinkNameAccessWrapper*)this;
}

OUString SwXLinkNameAccessWrapper::getImplementationName(void) throw( RuntimeException )
{
    return OUString("SwXLinkNameAccessWrapper");
}

sal_Bool SwXLinkNameAccessWrapper::supportsService(const OUString& rServiceName)
                                                    throw( RuntimeException )
{
    return (rServiceName == "com.sun.star.document.LinkTargets");
}

Sequence< OUString > SwXLinkNameAccessWrapper::getSupportedServiceNames(void)
                                                    throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pNames = aRet.getArray();
    pNames[0] = "com.sun.star.document.LinkTargets";
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

Reference< XPropertySetInfo >  SwXOutlineTarget::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
    return xRet;
}

void SwXOutlineTarget::setPropertyValue(
    const OUString& /*PropertyName*/, const Any& /*aValue*/)
    throw( UnknownPropertyException, PropertyVetoException,
         IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw UnknownPropertyException();
}

Any SwXOutlineTarget::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_LINK_DISPLAY_NAME)))
        aRet <<= OUString(sOutlineText);
    else
        throw UnknownPropertyException();
    return aRet;
}

void SwXOutlineTarget::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}

void SwXOutlineTarget::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const Reference< XPropertyChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}

void SwXOutlineTarget::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}

void SwXOutlineTarget::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const Reference< XVetoableChangeListener > & /*aListener*/)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}

OUString SwXOutlineTarget::getImplementationName(void) throw( RuntimeException )
{
    return OUString("SwXOutlineTarget");
}

sal_Bool SwXOutlineTarget::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    return ServiceName == "com.sun.star.document.LinkTarget";
}

Sequence< OUString > SwXOutlineTarget::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.LinkTarget";

    return aRet;
}

SwXDocumentPropertyHelper::SwXDocumentPropertyHelper(SwDoc& rDoc) :
SvxUnoForbiddenCharsTable ( rDoc.getForbiddenCharacterTable() )
,m_pDoc(&rDoc)
{
}

SwXDocumentPropertyHelper::~SwXDocumentPropertyHelper()
{
}

Reference<XInterface> SwXDocumentPropertyHelper::GetDrawTable(short nWhich)
{
    Reference<XInterface> xRet;
    if(m_pDoc)
    {
        switch(nWhich)
        {
            // #i52858#
            // assure that Draw model is created, if it doesn't exist.
            case SW_CREATE_DASH_TABLE         :
                if(!xDashTable.is())
                    xDashTable = SvxUnoDashTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xDashTable;
            break;
            case SW_CREATE_GRADIENT_TABLE     :
                if(!xGradientTable.is())
                    xGradientTable = SvxUnoGradientTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xGradientTable;
            break;
            case SW_CREATE_HATCH_TABLE        :
                if(!xHatchTable.is())
                    xHatchTable = SvxUnoHatchTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xHatchTable;
            break;
            case SW_CREATE_BITMAP_TABLE       :
                if(!xBitmapTable.is())
                    xBitmapTable = SvxUnoBitmapTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xBitmapTable;
            break;
            case SW_CREATE_TRANSGRADIENT_TABLE:
                if(!xTransGradientTable.is())
                    xTransGradientTable = SvxUnoTransGradientTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xTransGradientTable;
            break;
            case SW_CREATE_MARKER_TABLE       :
                if(!xMarkerTable.is())
                    xMarkerTable = SvxUnoMarkerTable_createInstance( m_pDoc->GetOrCreateDrawModel() );
                xRet = xMarkerTable;
            break;
            case  SW_CREATE_DRAW_DEFAULTS:
                if(!xDrawDefaults.is())
                    xDrawDefaults = (cppu::OWeakObject*)new SwSvxUnoDrawPool(m_pDoc);
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
    xDashTable = 0;
    xGradientTable = 0;
    xHatchTable = 0;
    xBitmapTable = 0;
    xTransGradientTable = 0;
    xMarkerTable = 0;
    xDrawDefaults = 0;
    m_pDoc = 0;
    SvxUnoForbiddenCharsTable::mxForbiddenChars.clear();
}

void SwXDocumentPropertyHelper::onChange()
{
    if(m_pDoc)
       m_pDoc->SetModified();
}

SwViewOptionAdjust_Impl::SwViewOptionAdjust_Impl(
            ViewShell& rSh, const SwViewOption &rViewOptions)
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
SwViewOptionAdjust_Impl::AdjustViewOptions(SwPrintData const*const pPrtOptions)
{
    // to avoid unnecessary reformatting the view options related to the content
    // below should only change if necessary, that is if respective content is present
    const bool bContainsHiddenChars         = m_pShell->GetDoc()->ContainsHiddenChars();
    const SwFieldType* pFldType = m_pShell->GetDoc()->GetSysFldType( RES_HIDDENTXTFLD );
    const bool bContainsHiddenFields        = pFldType && pFldType->GetDepends();
    pFldType = m_pShell->GetDoc()->GetSysFldType( RES_HIDDENPARAFLD );
    const bool bContainsHiddenParagraphs    = pFldType && pFldType->GetDepends();
    pFldType = m_pShell->GetDoc()->GetSysFldType( RES_JUMPEDITFLD );
    const bool bContainsPlaceHolders        = pFldType && pFldType->GetDepends();
    const bool bContainsFields              = m_pShell->IsAnyFieldInDoc();

    SwViewOption aRenderViewOptions( m_aOldViewOptions );

    // disable anything in the view that should not be printed (or exported to PDF) by default
    // (see also dialog "Tools/Options - StarOffice Writer - Formatting Aids"
    // in section "Display of ...")
    aRenderViewOptions.SetParagraph( sal_False );             // paragraph end
    aRenderViewOptions.SetSoftHyph( sal_False );              // aka custom hyphens
    aRenderViewOptions.SetBlank( sal_False );                 // spaces
    aRenderViewOptions.SetHardBlank( sal_False );             // non-breaking spaces
    aRenderViewOptions.SetTab( sal_False );                   // tabs
    aRenderViewOptions.SetLineBreak( sal_False );             // breaks (type 1)
    aRenderViewOptions.SetPageBreak( sal_False );             // breaks (type 2)
    aRenderViewOptions.SetColumnBreak( sal_False );           // breaks (type 3)
    sal_Bool bVal = pPrtOptions? pPrtOptions->bPrintHiddenText : sal_False;
    if (bContainsHiddenChars)
        aRenderViewOptions.SetShowHiddenChar( bVal );     // hidden text
    if (bContainsHiddenFields)
        aRenderViewOptions.SetShowHiddenField( bVal );
    if (bContainsHiddenParagraphs)
        aRenderViewOptions.SetShowHiddenPara( bVal );

    if (bContainsPlaceHolders)
    {
        // should always be printed in PDF export!
        bVal = pPrtOptions ? pPrtOptions->bPrintTextPlaceholder : sal_True;
        aRenderViewOptions.SetShowPlaceHolderFields( bVal );
    }

    if (bContainsFields)
        aRenderViewOptions.SetFldName( sal_False );

    // we need to set this flag in order to get to see the visible effect of
    // some of the above settings (needed for correct rendering)
    aRenderViewOptions.SetViewMetaChars( sal_True );

    if (m_aOldViewOptions != aRenderViewOptions)  // check if reformatting is necessary
    {
        aRenderViewOptions.SetPrinting( pPrtOptions != NULL );
        m_pShell->ApplyViewOptions( aRenderViewOptions );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
