/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <cmdid.h>
#include <swtypes.hxx>

#include <bf_sfx2/request.hxx>
#include <bf_svtools/intitem.hxx>

#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <wdocsh.hxx>

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <viewsh.hxx>
#include <pvprtdat.hxx>
#include <swprtopt.hxx>
#include <bf_svtools/stritem.hxx>
#include <unotxdoc.hxx>
#include <bf_svtools/numuno.hxx>
#include <unoobj.hxx>
#include <unosett.hxx>
#include <unocoll.hxx>
#include <unoredlines.hxx>
#include <unosrch.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_sfx2/objsh.hxx>   // SfxObjectShellRef <-> SV_DECL_REF(SfxObjectShell)
#include <unoprnms.hxx>
#include <unostyle.hxx>
#include <unodraw.hxx>
#include <bf_svtools/eitem.hxx>
#include <pagedesc.hxx>
#include <unoprnms.hxx>
#include <unoobj.hxx>
#include <bf_svtools/txtcmp.hxx>
#include <unocrsr.hxx>
#include <unofield.hxx>
#include <unoidx.hxx>
#include <poolfmt.hxx>
#include <globdoc.hxx>
#include <viewopt.hxx>

#include <bf_sfx2/app.hxx>

#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/RedlineDisplayType.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <cppuhelper/implbase1.hxx>

#include <bf_svx/linkmgr.hxx>
#include <bf_svx/unomid.hxx>
#include <bf_svx/unofill.hxx>
#include <bf_svx/unolingu.hxx>
#include <bf_sfx2/progress.hxx>
#include <osl/mutex.hxx>

#include <swtypes.hxx>
#include <swmodule.hxx>
#include <docstat.hxx>
#ifndef _MODOPT_HXX //
#include <modcfg.hxx>
 #endif
#include <ndtxt.hxx>
#include <utlui.hrc>
#include <swcont.hxx>
#include <unodefaults.hxx>
#include <SwXDocumentSettings.hxx>
#include <SwXPrintPreviewSettings.hxx>
#include <doc.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>
#include <bf_svtools/zforlist.hxx>
#include <drawdoc.hxx>
#include <SwStyleNameMapper.hxx>
#include <osl/file.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::i18n;
using ::rtl::OUString;
using ::osl::FileBase;

#if defined(MACOSX) && ( __GNUC__ < 3 )
static cppu::ClassData27 cppu::ImplHelper27::s_aCD;
static cppu::ClassData27 cppu::WeakImplHelper27::s_aCD;
#endif

/* -----------------------------17.01.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
#define SW_CREATE_DASH_TABLE            0x01
#define SW_CREATE_GRADIENT_TABLE        0x02
#define SW_CREATE_HATCH_TABLE           0x03
#define SW_CREATE_BITMAP_TABLE          0x04
#define SW_CREATE_TRANSGRADIENT_TABLE   0x05
#define SW_CREATE_MARKER_TABLE          0x06
#define SW_CREATE_DRAW_DEFAULTS         0x07


/******************************************************************************
 *
 ******************************************************************************/
SwTxtFmtColl *lcl_GetParaStyle(const String& rCollName, SwDoc* pDoc)
{
    SwTxtFmtColl* pColl = pDoc->FindTxtFmtCollByName( rCollName );
    if( !pColl )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( rCollName, GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId )
            pColl = pDoc->GetTxtCollFromPool( nId );
    }
    return pColl;
}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence< sal_Int8 > & SwXTextDocument::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextDocument::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    else
    {
        GetNumberFormatter();
        Any aNumTunnel = xNumFmtAgg->queryAggregation(::getCppuType((Reference<XUnoTunnel>*)0));
        Reference<XUnoTunnel> xNumTunnel;
        aNumTunnel >>= xNumTunnel;
        if(xNumTunnel.is())
            return xNumTunnel->getSomething(rId);
    }
    return 0;
}
/* -----------------------------16.03.00 14:12--------------------------------

 ---------------------------------------------------------------------------*/
Any SAL_CALL SwXTextDocument::queryInterface( const uno::Type& rType ) throw(RuntimeException)
{
    Any aRet = SwXTextDocumentBaseClass::queryInterface(rType);
    if(aRet.getValueType() == getVoidCppuType())
        aRet = SfxBaseModel::queryInterface(rType);
    if(aRet.getValueType() == getVoidCppuType() &&
        rType == ::getCppuType((Reference<lang::XMultiServiceFactory>*)0))
    {
        Reference<lang::XMultiServiceFactory> xTmp = this;
        aRet <<= xTmp;
    }
    if(aRet.getValueType() == getVoidCppuType())
    {
        GetNumberFormatter();
        if(xNumFmtAgg.is())
            aRet = xNumFmtAgg->queryAggregation(rType);
    }
    return aRet;
}
/* -----------------------------16.03.00 14:12--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextDocument::acquire()throw()
{
    SfxBaseModel::acquire();
}
/* -----------------------------16.03.00 14:12--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextDocument::release()throw()
{
    SfxBaseModel::release();
}
/* -----------------------------07.12.00 11:37--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XAdapter > SwXTextDocument::queryAdapter(  ) throw(RuntimeException)
{
    return SfxBaseModel::queryAdapter();
}
/* -----------------------------16.03.00 14:12--------------------------------

 ---------------------------------------------------------------------------*/
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
    // don't forget the ::com::sun::star::lang::XMultiServiceFactory
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
/*-- 18.12.98 11:52:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextDocument::SwXTextDocument(SwDocShell* pShell) :
    SfxBaseModel(pShell),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_DOCUMENT)),
    aRefreshCont ( static_cast < XTextDocument* > ( this ) ),
    pDocShell(pShell),
    bObjectValid(pShell != 0),
    pxXTextTables(0),
    pxXTextFrames(0),
    pxXGraphicObjects(0),
    pxXEmbeddedObjects(0),
    pxXTextSections(0),
    pxXBookmarks(0),
    pxXTextFieldTypes(0),
    pxXTextFieldMasters(0),
    pxXFootnotes(0),
    pxXEndnotes(0),
    pxXFootnoteSettings(0),
    pxXEndnoteSettings(0),
    pxXDocumentIndexes(0),
    pxXStyleFamilies(0),
    pxXChapterNumbering(0),
    pxXLineNumberingProperties(0),
    pDrawPage(0),
    pxXDrawPage(0),
    pxXReferenceMarks(0),
    pxLinkTargetSupplier(0),
    pxXRedlines(0)
{
}
/*-- 18.12.98 11:53:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextDocument::~SwXTextDocument()
{
    InitNewDoc();
    if(xNumFmtAgg.is())
    {
        Reference< XInterface >  x0;
        xNumFmtAgg->setDelegator(x0);
        xNumFmtAgg = 0;
    }
}
/*-- 18.12.98 11:55:08---------------------------------------------------

  -----------------------------------------------------------------------*/
/* -----------------18.12.98 12:49-------------------
 *
 * --------------------------------------------------*/
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
                pNumFmt = (SvNumberFormatsSupplierObj*)
                        xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());

            }
            DBG_ASSERT(pNumFmt, "No number formatter available");
            if(!pNumFmt->GetNumberFormatter())
                pNumFmt->SetNumberFormatter(pDocShell->GetDoc()->GetNumberFormatter( sal_True ));
        }
    }
}
/*-- 18.12.98 11:55:11---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:11---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::reformat(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
}
/*-- 18.12.98 11:55:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::lockControllers(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        UnoActionContext* pContext = new UnoActionContext(pDocShell->GetDoc());
        aActionArr.Insert(pContext, 0);
    }
    else
        throw RuntimeException();
}
/*-- 18.12.98 11:55:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::unlockControllers(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(aActionArr.Count())
    {
        UnoActionContext* pContext = aActionArr.GetObject(0);
        aActionArr.Remove(0);
        delete pContext;
    }
    else
        throw RuntimeException();
}
/*-- 18.12.98 11:55:17---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextDocument::hasControllersLocked(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return aActionArr.Count() > 0;
}
/*-- 18.12.98 13:12:23---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< frame::XController >  SwXTextDocument::getCurrentController(void) throw( RuntimeException )
{
    return Reference< frame::XController >();
}
/*-- 18.12.98 13:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::setCurrentController(const Reference< frame::XController > & xController)
    throw( NoSuchElementException, RuntimeException )
{
}

/* -----------------27.01.99 11:48-------------------
 *
 * --------------------------------------------------*/
Reference< XInterface >  SwXTextDocument::getCurrentSelection() throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XInterface >  xRef;
    return xRef;
}

/*-- 18.12.98 13:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextDocument::attachResource(const OUString& aURL, const Sequence< beans::PropertyValue >& aArgs)
        throw( RuntimeException )
{
    return SfxBaseModel::attachResource(aURL, aArgs);
}
/*-- 18.12.98 13:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextDocument::getURL(void) throw( RuntimeException )
{
    return SfxBaseModel::getURL();
}
/*-- 18.12.98 13:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< beans::PropertyValue > SwXTextDocument::getArgs(void) throw( RuntimeException )
{
    return SfxBaseModel::getArgs();
}
/*-- 18.12.98 13:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::connectController(const Reference< frame::XController > & xController) throw( RuntimeException )
{
    SfxBaseModel::connectController(xController);
}
/*-- 18.12.98 13:12:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::disconnectController(const Reference< frame::XController > & xController) throw( RuntimeException )
{
    SfxBaseModel::disconnectController(xController);
}
/*-- 18.12.98 13:12:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::dispose(void) throw( RuntimeException )
{
    SfxBaseModel::dispose();
}
/*-- 18.12.98 13:12:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::addEventListener(const Reference< lang::XEventListener > & aListener) throw( RuntimeException )
{
    SfxBaseModel::addEventListener(aListener);
}
/*-- 18.12.98 13:12:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::removeEventListener(const Reference< lang::XEventListener > & aListener) throw( RuntimeException )
{
    SfxBaseModel::removeEventListener(aListener);
}

/*-- 18.12.98 11:55:19---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:20---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:21---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:21---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:21---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:22---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 18.12.98 11:55:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< util::XReplaceDescriptor >  SwXTextDocument::createReplaceDescriptor(void)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< util::XReplaceDescriptor >  xRet = new SwXTextSearch;
    return xRet;
}
/* -----------------26.02.99 15:52-------------------
 *
 * --------------------------------------------------*/
SwUnoCrsr* 	SwXTextDocument::CreateCursorForSearch(Reference< XTextCursor > & xCrsr)
{
    getText();
     XText* pText = xBodyText.get();
    SwXBodyText* pBText = (SwXBodyText*)pText;
    xCrsr = pBText->CreateTextCursor(sal_True);

    Reference<XUnoTunnel> xRangeTunnel( xCrsr, UNO_QUERY);
    SwXTextCursor* pxUnoCrsr = 0;
    if(xRangeTunnel.is())
    {
        pxUnoCrsr = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }

    sal_uInt32 nResult = 0;
    SwUnoCrsr* 	pUnoCrsr = pxUnoCrsr->GetCrsr();
    pUnoCrsr->SetRemainInSection(sal_False);
    return pUnoCrsr;
}

/*-- 18.12.98 11:55:22---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXTextDocument::replaceAll(const Reference< util::XSearchDescriptor > & xDesc)
                                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY);
    if(!IsValid() || !xDescTunnel.is() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        throw RuntimeException();

    Reference< XTextCursor >  xCrsr;
    SwUnoCrsr* 	pUnoCrsr = CreateCursorForSearch(xCrsr);

    const SwXTextSearch* pSearch = (const SwXTextSearch*)
            xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId());

    sal_Bool bBackward = sal_False;
    int eRanges(FND_IN_BODY|FND_IN_SELALL);

    SearchOptions aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

    SwDocPositions eStart = pSearch->bBack ? DOCPOS_END : DOCPOS_START;
    SwDocPositions eEnd = pSearch->bBack ? DOCPOS_START : DOCPOS_END;

    // Suche soll ueberall stattfinden
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
        BOOL bCancel;
        nResult = (sal_Int32)pUnoCrsr->Find( aSearch, !pSearch->bStyles,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges,
                    pSearch->sSearchText.Len() ? &aSearchOpt : 0,
                    &aReplace );
    }
    else if(pSearch->bStyles)
    {
        SwTxtFmtColl *pSearchColl = lcl_GetParaStyle(pSearch->sSearchText, pUnoCrsr->GetDoc());
        SwTxtFmtColl *pReplaceColl = lcl_GetParaStyle(pSearch->sReplaceText, pUnoCrsr->GetDoc());;

        BOOL bCancel;
        nResult = pUnoCrsr->Find( *pSearchColl,
                    eStart, eEnd, bCancel,
                    (FindRanges)eRanges, pReplaceColl );

    }
    else
    {
        BOOL bCancel;
        nResult = pUnoCrsr->Find( aSearchOpt,
            eStart, eEnd, bCancel,
            (FindRanges)eRanges,
            sal_True );
    }
    return (sal_Int32)nResult;

}
/*-- 18.12.98 11:55:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< util::XSearchDescriptor >  SwXTextDocument::createSearchDescriptor(void)
                                                    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Reference< util::XSearchDescriptor >  xRet = new SwXTextSearch;
    return xRet;

}
/* -----------------26.02.99 16:08-------------------
 * wird fuer findAll/First/Next verwendet
 * --------------------------------------------------*/
SwUnoCrsr* 	SwXTextDocument::FindAny(const Reference< util::XSearchDescriptor > & xDesc,
                                        Reference< XTextCursor > & xCrsr, sal_Bool bAll,
                                                sal_Int32& nResult,
                                                Reference< XInterface >  xLastResult)
{
    Reference< XUnoTunnel > xDescTunnel(xDesc, UNO_QUERY);
    if(!IsValid() || !xDescTunnel.is() || !xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId()))
        return 0;

    SwUnoCrsr* 	pUnoCrsr = CreateCursorForSearch(xCrsr);
    const SwXTextSearch* pSearch = (const SwXTextSearch*)
        xDescTunnel->getSomething(SwXTextSearch::getUnoTunnelId());

    sal_Bool bParentInExtra = sal_False;
    if(xLastResult.is())
    {
        Reference<XUnoTunnel> xCursorTunnel( xLastResult, UNO_QUERY);
        OTextCursorHelper* pPosCrsr = 0;
        if(xCursorTunnel.is())
        {
            pPosCrsr = (OTextCursorHelper*)xCursorTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId());
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
                pRange = (SwXTextRange*)xCursorTunnel->getSomething(
                                        SwXTextRange::getUnoTunnelId());
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

    sal_Bool bBackward = sal_False;
    SearchOptions aSearchOpt;
    pSearch->FillSearchOptions( aSearchOpt );

/*
 * folgende Kombinationen sind erlaubt:
 *  - suche einen im Body:					-> FND_IN_BODY
 *  - suche alle im Body: 					-> FND_IN_BODYONLY | FND_IN_SELALL
 *  - suche in Selectionen: einen / alle    -> FND_IN_SEL  [ | FND_IN_SELALL ]
 *  - suche im nicht Body: einen / alle     -> FND_IN_OTHER [ | FND_IN_SELALL ]
 *  - suche ueberall alle: 					-> FND_IN_SELALL
 */
    int eRanges(FND_IN_BODY);
    if(bParentInExtra)
        eRanges = FND_IN_OTHER;
    if(bAll) //immer - ueberall?
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
                                RES_TXTATR_INETFMT,	RES_TXTATR_INETFMT,
                                0);
            pSearch->FillSearchItemSet(aSearch);
            BOOL bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( aSearch, !pSearch->bStyles,
                        eStart, eEnd, bCancel,
                        (FindRanges)eRanges,
                        pSearch->sSearchText.Len() ? &aSearchOpt : 0,
                        0 );
        }
        else if(pSearch->bStyles)
        {
            SwTxtFmtColl *pSearchColl = lcl_GetParaStyle(pSearch->sSearchText, pUnoCrsr->GetDoc());
            //pSearch->sReplaceText
            SwTxtFmtColl *pReplaceColl = 0;
            BOOL bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( *pSearchColl,
                        eStart, eEnd, bCancel,
                        (FindRanges)eRanges, pReplaceColl );
        }
        else
        {
            BOOL bCancel;
            nResult = (sal_Int32)pUnoCrsr->Find( aSearchOpt,
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
/*-- 18.12.98 11:55:23---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    if(nResult)
        xRet = new SwXTextRanges(pResultCrsr);
    else
        xRet = new SwXTextRanges();
    delete pResultCrsr;
    return xRet;
}
/*-- 18.12.98 11:55:23---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        Reference< XTextRange >  xTempRange = SwXTextRange::CreateTextRangeFromPosition(
                        pDocShell->GetDoc(),
                        *pResultCrsr->GetPoint(),
                        pResultCrsr->GetMark());
        xRet = *new SwXTextCursor(xTempRange->getText(), pResultCrsr);
        delete pResultCrsr;
    }
    return xRet;
}
/*-- 18.12.98 11:55:24---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        Reference< XTextRange >  xTempRange = SwXTextRange::CreateTextRangeFromPosition(
                        pDocShell->GetDoc(),
                        *pResultCrsr->GetPoint(),
                        pResultCrsr->GetMark());

        xRet = *new SwXTextCursor(xTempRange->getText(), pResultCrsr);
        delete pResultCrsr;
    }
    return xRet;
}
/*-- 18.12.98 11:55:24---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        pArray[0] = beans::PropertyValue(C2U("PageRows"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int16)aData.GetCol();
        pArray[1] = beans::PropertyValue(C2U("PageColumns"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetLeftSpace());
        pArray[2] = beans::PropertyValue(C2U("LeftMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetRightSpace());
        pArray[3] = beans::PropertyValue(C2U("RightMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetTopSpace());
        pArray[4] = beans::PropertyValue(C2U("TopMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetBottomSpace());
        pArray[5] = beans::PropertyValue(C2U("BottomMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetHorzSpace());
        pArray[6] = beans::PropertyValue(C2U("HoriMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        aVal <<= (sal_Int32)TWIP_TO_MM100(aData.GetVertSpace());
        pArray[7] = beans::PropertyValue(C2U("VertMargin"), -1, aVal, PropertyState_DIRECT_VALUE);
        BOOL bTemp = aData.GetLandscape();
        aVal.setValue(&bTemp, ::getCppuBooleanType());
        pArray[8] = beans::PropertyValue(C2U("IsLandscape"), -1, aVal, PropertyState_DIRECT_VALUE);
    }
    else
        throw RuntimeException();
    return aSeq;
}
/* -----------------24.02.99 10:57-------------------
 *
 * --------------------------------------------------*/
sal_uInt32 lcl_Any_To_ULONG(const Any& rValue, sal_Bool& bException)
{
    bException = sal_False;
    TypeClass eType = rValue.getValueType().getTypeClass();

    sal_uInt32 nRet = 0;
    if( eType == TypeClass_LONG )
    {
         sal_Int32 nVal;
         rValue >>= nVal;
         if(nVal >=0 )
            nRet = (sal_uInt32)nVal;
    }
    else if( eType == TypeClass_UNSIGNED_SHORT )
    {
        sal_uInt16 nVal;
        rValue >>= nVal;
        nRet = nVal;
    }
    else if( eType == TypeClass_SHORT )
    {
        sal_Int16 nVal;
        rValue >>= nVal;
        if(nVal >=0)
            nRet = (sal_uInt32)nVal;
    }
    else if( eType == TypeClass_BYTE )
    {
        BYTE nVal;
        rValue >>= nVal;
        nRet = nVal;
    }
    else if( eType == TypeClass_UNSIGNED_LONG )
        rValue >>= nRet;
    else
        bException = sal_True;

    return nRet;
}
/*-- 18.12.98 11:55:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::setPagePrintSettings(const Sequence< beans::PropertyValue >& aSettings)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        SwPagePreViewPrtData aData;
        //falls nur einige Properties kommen, dann die akt. Einstellungen benutzen
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
                if(!nVal)
                    throw RuntimeException();
                aData.SetRow(nVal);
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("PageColumns"))
            {
                if(!nVal)
                    throw RuntimeException();
                aData.SetCol(nVal);
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("LeftMargin"))
            {
                aData.SetLeftSpace(MM100_TO_TWIP(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("RightMargin"))
            {
                aData.SetRightSpace(MM100_TO_TWIP(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("TopMargin"))
            {
                aData.SetTopSpace(MM100_TO_TWIP(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("BottomMargin"))
            {
                aData.SetBottomSpace(MM100_TO_TWIP(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("HoriMargin"))
            {
                aData.SetHorzSpace(MM100_TO_TWIP(nVal));
            }
            else if(COMPARE_EQUAL == sName.CompareToAscii("VertMargin"))
            {
                aData.SetVertSpace(MM100_TO_TWIP(nVal));
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
/*-- 18.12.98 11:55:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::printPages(const Sequence< beans::PropertyValue >& xOptions)
    throw( IllegalArgumentException, RuntimeException )
{
    throw RuntimeException();
}
/*-- 18.12.98 11:55:25---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/* -----------------21.12.98 10:20-------------------
 *
 * --------------------------------------------------*/
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
/*-- 21.12.98 10:21:12---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:12---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:13---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 21.12.98 10:21:14---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/* -----------------21.12.98 10:56-------------------
 *
 * --------------------------------------------------*/
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
/*-- 22.01.99 10:18:03---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< drawing::XDrawPage >  SwXTextDocument::getDrawPage(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    if(!pxXDrawPage)
    {
        ((SwXTextDocument*)this)->pxXDrawPage = new Reference< drawing::XDrawPage > ;
        ((SwXTextDocument*)this)->pDrawPage = new SwXDrawPage(pDocShell->GetDoc());
        Reference< drawing::XShapes >  xTmp = pDrawPage;
        *pxXDrawPage = Reference< drawing::XDrawPage>(xTmp, UNO_QUERY);
    }
    return *pxXDrawPage;
}
/*-- 18.12.98 11:55:26---------------------------------------------------

  -----------------------------------------------------------------------*/
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
            pNumFmt = (SvNumberFormatsSupplierObj*)
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());
            pNumFmt->SetNumberFormatter(0);
        }
        DBG_ASSERT(pNumFmt, "No number formatter available");
    }
    InitNewDoc();
    pDocShell = 0;
    aRefreshCont.Disposing();
}
/* -----------------------------13.07.00 15:59--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextDocument::Reactivate(SwDocShell* pNewDocShell)
{
    if(pDocShell && pDocShell != pNewDocShell)
        Invalidate();
    pDocShell = pNewDocShell;
    bObjectValid = TRUE;
}
/*-- 18.12.98 11:55:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void	SwXTextDocument::InitNewDoc()
{
    // zunaechst alle Collections invalidieren, dann Referenzen loeschen und Null setzen
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
            pNumFmt = (SvNumberFormatsSupplierObj*)
                    xNumTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());

        }
        DBG_ASSERT(pNumFmt, "No number formatter available");
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
         drawing::XDrawPage* pxPage = pxXDrawPage->get();
        pDrawPage->Invalidate();
        delete pxXDrawPage;
        pxXDrawPage = 0;
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

/*-- 11.03.99 11:51:40---------------------------------------------------

  -----------------------------------------------------------------------*/
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
        if( rServiceName.compareToAscii( "com.sun.star.", 13 ) == 0 )
        {
            sal_Int32 nIndex = COM_SUN_STAR__DRAWING_LENGTH;
            OUString sCategory = rServiceName.getToken( 0, '.', nIndex );
            sal_Bool bShape = sCategory == C2U("drawing");
            if( bShape || sCategory == C2U("form"))
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
            else if (sCategory == C2U ("document") )
            {
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.Settings") ) )
                    xRet = Reference < XInterface > ( *new SwXDocumentSettings ( this ) );
            }
            else if (sCategory == C2U ("text") )
            {
                if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.PrintPreviewSettings") ) )
                    xRet = Reference < XInterface > ( *new SwXPrintPreviewSettings ( pDocShell->GetDoc() ) );
                else if( 0 == rServiceName.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.DocumentSettings") ) )
                    xRet = Reference < XInterface > ( *new SwXDocumentSettings ( this ) );
            }
            if(!xRet.is())
            {
                //hier den Draw - Service suchen
                Reference< XInterface >  xTmp = SvxFmMSFactory::createInstance(rServiceName);
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
/*-- 11.03.99 11:51:40---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XInterface >  SwXTextDocument::createInstanceWithArguments(
        const OUString& ServiceSpecifier,
        const Sequence< Any >& Arguments)
        throw( Exception, RuntimeException )
{
    Reference< XInterface >  xInt = createInstance(ServiceSpecifier);
    //die Any-Sequence dient zur Initialisierung von Objekten, die auf
    //Parameter zwingend angewiesen sind - bis jetzt haben wir das nicht
    return xInt;
}
/*-- 11.03.99 11:51:41---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXTextDocument::getAvailableServiceNames(void)
                                        throw( RuntimeException )
{
    Sequence< OUString > aRet =  SvxFmMSFactory::getAvailableServiceNames();
    Sequence< OUString > aOwn = SwXServiceProvider::GetAllServiceNames();
    return SvxFmMSFactory::concatServiceNames(aRet, aOwn);

}
/* -----------------18.03.99 11:36-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextDocument::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextDocument");
}
/* -----------------18.03.99 11:32-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextDocument::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    BOOL bWebDoc    = 0 != PTR_CAST(SwWebDocShell,    pDocShell);
    BOOL bGlobalDoc = 0 != PTR_CAST(SwGlobalDocShell, pDocShell);
    BOOL bTextDoc   = !bWebDoc && !bGlobalDoc;

    sal_Bool bRet = (
                        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.document.OfficeDocument"  ) ) ||
                        rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.text.GenericTextDocument" ) ) ||
                        (
                            bTextDoc &&
                            rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.text.TextDocument" ) )
                        ) ||
                        (
                            bWebDoc &&
                            rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.text.WebDocument" ) )
                        ) ||
                        (
                            bGlobalDoc &&
                            rServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "com.sun.star.text.GlobalDocument" ) )
                        )
                    );

    return bRet;
}
/* -----------------18.03.99 11:32-------------------
 *
 * --------------------------------------------------*/
Sequence< OUString > SwXTextDocument::getSupportedServiceNames(void) throw( RuntimeException )
{
    BOOL bWebDoc    = 0 != PTR_CAST(SwWebDocShell,    pDocShell);
    BOOL bGlobalDoc = 0 != PTR_CAST(SwGlobalDocShell, pDocShell);
    BOOL bTextDoc   = !bWebDoc && !bGlobalDoc;

    Sequence< OUString > aRet ( 3 );
    OUString* pArray = aRet.getArray();

    pArray[0] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( ( "com.sun.star.document.OfficeDocument" ) ) );
    pArray[1] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( ( "com.sun.star.text.GenericTextDocument" ) ) );

    if (bTextDoc)
        pArray[2] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( ( "com.sun.star.text.TextDocument" ) ) );
    if (bWebDoc)
        pArray[2] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( ( "com.sun.star.text.WebDocument" ) ) );
    if (bGlobalDoc)
        pArray[2] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( ( "com.sun.star.text.GlobalDocument" ) ) );

    return aRet;
}
/* -----------------05.05.99 12:10-------------------
 *
 * --------------------------------------------------*/
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

/*-- 10.05.99 13:58:58---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXTextDocument::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = aPropSet.getPropertySetInfo();
    return xRet;
}
/*-- 10.05.99 13:58:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::setPropertyValue(const OUString& rPropertyName,
    const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
                                         WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);

    if(!pMap)
        throw UnknownPropertyException();
    if(pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    switch(pMap->nWID)
    {
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
            throw RuntimeException();
        break;
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
            if(WID_DOC_CHANGES_SHOW == pMap->nWID)
            {
                eMode &= ~(REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE);
                eMode |= REDLINE_SHOW_INSERT;
                if( bSet )
                    eMode |= REDLINE_SHOW_DELETE;
            }
            else if(WID_DOC_CHANGES_RECORD == pMap->nWID)
            {
                eMode = bSet ? eMode|REDLINE_ON : eMode&~REDLINE_ON;
            }
            pDocShell->GetDoc()->SetRedlineMode( eMode );
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            Sequence <sal_Int8> aNew;
            if(aValue >>= aNew)
            {
                SwDoc* pDoc = pDocShell->GetDoc();
                pDoc->SetRedlinePasswd(aNew);
                if(aNew.getLength())
                {
                    sal_uInt16 eMode = pDoc->GetRedlineMode();
                    eMode = eMode|REDLINE_ON;
                    pDoc->SetRedlineMode( eMode );
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
            eRedMode = eRedMode & (~REDLINE_SHOW_MASK);
            sal_Int16 nSet;
            aValue >>= nSet;
            switch(nSet)
            {
                case RedlineDisplayType::NONE: break;
                case RedlineDisplayType::INSERTED: nSet |= REDLINE_SHOW_INSERT; break;
                case RedlineDisplayType::REMOVED: nSet |= REDLINE_SHOW_DELETE; 	break;
                case RedlineDisplayType::
                        INSERTED_AND_REMOVED: nSet |= REDLINE_SHOW_INSERT|REDLINE_SHOW_DELETE;
                break;
                default: throw IllegalArgumentException();
            }
            pDocShell->GetDoc()->SetRedlineMode(nSet);
        }
        break;
        case WID_DOC_TWO_DIGIT_YEAR:
        {
            sal_Int16 nYear;
            aValue >>= nYear;
            pDocShell->GetDoc()->GetNumberFormatter(TRUE)->SetYear2000(nYear);
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bAuto = *(sal_Bool*) aValue.getValue();

            if ( ( pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
                pDrawDoc->SetAutoControlFocus( bAuto );
            else if (bAuto)
            {
                // if setting to true, and we don't have an
                // SdrModel, then we are changing the default and
                // must thus create an SdrModel, if we don't have an
                // SdrModel and we are leaving the default at false,
                // we don't need to make an SdrModel and can do nothing
                pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->MakeDrawModel() );
                pDrawDoc->SetAutoControlFocus ( bAuto );
            }
        }
        break;
        case WID_DOC_APPLY_FORM_DESIGN_MODE:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bMode = *(sal_Bool*)aValue.getValue();

            if ( ( pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
                pDrawDoc->SetOpenInDesignMode( bMode );
            else if (!bMode)
            {
                // if setting to false, and we don't have an
                // SdrModel, then we are changing the default and
                // must thus create an SdrModel, if we don't have an
                // SdrModel and we are leaving the default at true,
                // we don't need to make an SdrModel and can do
                // nothing
                pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->MakeDrawModel() );
                pDrawDoc->SetOpenInDesignMode ( bMode );
            }
        }
        break;
        default:
        {
            const SfxPoolItem& rItem = pDocShell->GetDoc()->GetDefault(pMap->nWID);
            SfxPoolItem* pNewItem = rItem.Clone();
            pNewItem->PutValue(aValue, pMap->nMemberId);
            pDocShell->GetDoc()->SetDefault(*pNewItem);
            delete pNewItem;
        }
    }
}
/*-- 10.05.99 13:58:59---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextDocument::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);

    if(!pMap)
        throw UnknownPropertyException();
    Any aAny;
    switch(pMap->nWID)
    {
        case  WID_DOC_CHAR_COUNT     :
        case  WID_DOC_PARA_COUNT     :
        case  WID_DOC_WORD_COUNT     :
        {
            SwDocStat aStat(pDocShell->GetDoc()->GetDocStat());
            if(aStat.bModified)
                pDocShell->GetDoc()->UpdateDocStat( aStat );
            sal_Int32 nValue;
            switch(pMap->nWID)
            {
                case  WID_DOC_CHAR_COUNT     :nValue = aStat.nChar;break;
                case  WID_DOC_PARA_COUNT     :nValue = aStat.nPara;break;
                case  WID_DOC_WORD_COUNT     :nValue = aStat.nWord;break;
            }
            aAny <<= nValue;
        }
        break;
        case  WID_DOC_WORD_SEPARATOR :
        {
            aAny <<= OUString(SW_MOD()->GetDocStatWordDelim());
        }
        break;
        case WID_DOC_CHANGES_RECORD:
        case WID_DOC_CHANGES_SHOW:
        {
            sal_uInt16 eMode = pDocShell->GetDoc()->GetRedlineMode();
            sal_Bool bSet = sal_False;
            if(WID_DOC_CHANGES_SHOW == pMap->nWID)
            {
                sal_uInt16 nMask = REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE;
                bSet = (eMode & nMask) == nMask;
            }
            else if(WID_DOC_CHANGES_RECORD == pMap->nWID)
            {
                bSet = (eMode& REDLINE_ON)  != 0;
            }
            aAny.setValue(&bSet, ::getBooleanCppuType());
        }
        break;
        case  WID_DOC_CHANGES_PASSWORD:
        {
            SwDoc* pDoc = pDocShell->GetDoc();
            aAny <<= pDoc->GetRedlinePasswd();
        }
        break;
        case WID_DOC_AUTO_MARK_URL :
            aAny <<= OUString(pDocShell->GetDoc()->GetTOIAutoMarkURL());
        break;
        case WID_DOC_HIDE_TIPS :
        {
            BOOL bTemp = SW_MOD()->GetModuleConfig()->IsHideFieldTips();
            aAny.setValue(&bTemp, ::getBooleanCppuType());
        }
        break;
        case WID_DOC_REDLINE_DISPLAY:
        {
            sal_Int16 eRedMode = pDocShell->GetDoc()->GetRedlineMode();
            eRedMode = eRedMode & REDLINE_SHOW_MASK;
            sal_Int16 nRet = RedlineDisplayType::NONE;
            if(REDLINE_SHOW_INSERT == eRedMode)
                nRet = RedlineDisplayType::INSERTED;
            else if(REDLINE_SHOW_DELETE == eRedMode)
                nRet = RedlineDisplayType::REMOVED;
            else if(REDLINE_SHOW_MASK == eRedMode)
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
            aAny <<= static_cast < sal_Int16 > (pDocShell->GetDoc()->GetNumberFormatter ( TRUE )->GetYear2000());
        }
        break;
        case WID_DOC_AUTOMATIC_CONTROL_FOCUS:
        {
            SwDrawDocument * pDrawDoc;
            sal_Bool bAuto;
            if ( ( pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
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
            if ( ( pDrawDoc = reinterpret_cast < SwDrawDocument * > (pDocShell->GetDoc()->GetDrawModel() ) ) )
                bMode = pDrawDoc->GetOpenInDesignMode();
            else
                bMode = sal_True;
            aAny.setValue(&bMode, ::getBooleanCppuType());
        }
        break;
        case WID_DOC_BASIC_LIBRARIES:
            aAny <<= pDocShell->GetBasicContainer();
        break;
        default:
        {
            const SfxPoolItem& rItem = pDocShell->GetDoc()->GetDefault(pMap->nWID);
            rItem.QueryValue(aAny, pMap->nMemberId);
        }
    }
    return aAny;
}
/*-- 10.05.99 13:58:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::addPropertyChangeListener(const OUString& PropertyName,
    const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.05.99 13:58:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::removePropertyChangeListener(const OUString& PropertyName,
    const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.05.99 13:59:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::addVetoableChangeListener(const OUString& PropertyName,
    const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 10.05.99 13:59:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::removeVetoableChangeListener(const OUString& PropertyName,
                        const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/* -----------------25.10.99 10:42-------------------

 --------------------------------------------------*/
Reference< XNameAccess >  SwXTextDocument::getLinks(void) throw( RuntimeException )
{
    if(!pxLinkTargetSupplier)
    {
        pxLinkTargetSupplier = new Reference< XNameAccess > ;
        (*pxLinkTargetSupplier) = new SwXLinkTargetSupplier(*(SwXTextDocument*)this);
    }
    return (*pxLinkTargetSupplier);
}
/* -----------------------------11.01.01 15:01--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XEnumerationAccess > SwXTextDocument::getRedlines(  ) throw(RuntimeException)
{
    if(!pxXRedlines)
    {
        pxXRedlines = new Reference< XEnumerationAccess > ;
        (*pxXRedlines) = new SwXRedlines(pDocShell->GetDoc());
    }
    return *pxXRedlines;
}
/*-- 21.02.00 08:41:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::refresh(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    SwWrtShell *pWrtShell = pDocShell->GetWrtShell();
    notifyRefreshListeners();
    if(pWrtShell)
        pWrtShell->CalcLayout();
}
/*-- 21.02.00 08:41:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::addRefreshListener(const Reference< util::XRefreshListener > & l)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !IsValid() )
        throw RuntimeException();
    aRefreshCont.AddListener ( reinterpret_cast < const Reference < lang::XEventListener > &> ( l ));
}
/*-- 21.02.00 08:41:07---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextDocument::removeRefreshListener(const Reference< util::XRefreshListener > & l)
    throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !IsValid() || !aRefreshCont.RemoveListener ( reinterpret_cast < const Reference < lang::XEventListener > &> ( l ) ) )
        throw RuntimeException();
}
/* -----------------------------26.02.01 12:22--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextDocument::updateLinks(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    SwDoc* pDoc = pDocShell->GetDoc();
      SvxLinkManager& rLnkMan = pDoc->GetLinkManager();
    if( rLnkMan.GetLinks().Count() )
    {
        UnoActionContext aAction(pDoc);
        rLnkMan.UpdateAllLinks( FALSE, TRUE );
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
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);

    if(!pMap)
        throw UnknownPropertyException();
    Any aAny;
    switch(pMap->nWID)
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
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    switch(pMap->nWID)
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
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    aPropSet.getPropertyMap(), rPropertyName);
    if(!pMap)
        throw UnknownPropertyException();
    Any aAny;
    switch(pMap->nWID)
    {
        case 0:default:break;
    }
    return aAny;
}
/* -----------------------------23.08.02 16:00--------------------------------

 ---------------------------------------------------------------------------*/
/*
 *  GetRenderDoc:
 *  returns the document to be rendered, usually this will be the 'regular'
 *  document but in case of PDF export of (multi-)selection it will
 *  be a temporary document that gets created if not already done.
 *  The rpView variable will be set (if not already done) to the used
 *  SfxViewShell.
*/

SwDoc* SwXTextDocument::GetRenderDoc( const uno::Any& rSelection )
{
    SwDoc *pDoc = 0;

    uno::Reference< frame::XModel > xModel;
    rSelection >>= xModel;
    if (xModel == pDocShell->GetModel())
        pDoc = pDocShell->GetDoc();
    else
        DBG_BF_ASSERT(0, "STRIP");
    return pDoc;
}
/* -----------------------------23.08.02 16:00--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int32 SAL_CALL SwXTextDocument::getRendererCount(
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();

    SwDoc *pDoc = GetRenderDoc( rSelection );
    if (!pDoc)
        throw RuntimeException();

    SwWrtShell *pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    if( pWrtShell )
    {
        SwViewOption aViewOpt( *pWrtShell->GetViewOptions() );
        aViewOpt.SetPDFExport( TRUE );
        if ( pWrtShell->IsBrowseMode() )
            aViewOpt.SetPrtFormat( TRUE );
        pWrtShell->ApplyViewOptions( aViewOpt );
        pWrtShell->CalcLayout();
        aViewOpt.SetPDFExport( FALSE );
        pWrtShell->ApplyViewOptions( aViewOpt );
    }

    return pDoc->GetPageCount();
}
/* -----------------------------23.08.02 16:00--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SAL_CALL SwXTextDocument::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    SwDoc *pDoc = GetRenderDoc( rSelection );
    if (!pDoc)
        throw RuntimeException();

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (!(0 <= nRenderer /* &&  nRenderer < pDoc->GetPageCount()*/))
        throw IllegalArgumentException();
    if (nRenderer >= pDoc->GetPageCount())
        return uno::Sequence< beans::PropertyValue >();

    Size aPgSize( pDoc->GetPageSize( nRenderer + 1 ) );
    DBG_ASSERT( aPgSize != Size(), "no page size" );

    awt::Size aPageSize( TWIP_TO_MM100( aPgSize.Width() ),
                         TWIP_TO_MM100( aPgSize.Height() ));
    uno::Sequence< beans::PropertyValue > aRenderer(1);
    PropertyValue  &rValue = aRenderer.getArray()[0];
    rValue.Name  = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) );
    rValue.Value <<= aPageSize;

    return aRenderer;
}
/* -----------------------------28.10.02 16:00--------------------------------
 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextDocument::render(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        throw RuntimeException();
    SwDoc *pDoc = GetRenderDoc( rSelection );
    if (!pDoc)
        throw RuntimeException();

    // due to #110067# (document page count changes sometimes during
    // PDF export/printing) we can not check for the upper bound properly.
    // Thus instead of throwing the exception we silently return.
    if (!(0 <= nRenderer /* &&  nRenderer < pDoc->GetPageCount()*/))
        throw IllegalArgumentException();
    if (nRenderer >= pDoc->GetPageCount())
        return;

    DBG_BF_ASSERT(0, "STRIP");
}
/* -----------------------------20.06.00 09:54--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextDocument::operator new( size_t t) throw()
{
    return SwXTextDocumentBaseClass::operator new(t);
}
/* -----------------------------20.06.00 09:54--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextDocument::operator delete( void * p) throw()
{
    SwXTextDocumentBaseClass::operator delete(p);
}
/* -----------------25.10.99 11:06-------------------

 --------------------------------------------------*/
SwXLinkTargetSupplier::SwXLinkTargetSupplier(SwXTextDocument& rxDoc) :
    pxDoc(&rxDoc)
{
    sTables 	= String(SW_RES(STR_CONTENT_TYPE_TABLE));
    sFrames     = String(SW_RES(STR_CONTENT_TYPE_FRAME));
    sGraphics	= String(SW_RES(STR_CONTENT_TYPE_GRAPHIC));
    sOLEs		= String(SW_RES(STR_CONTENT_TYPE_OLE));
    sSections	= String(SW_RES(STR_CONTENT_TYPE_REGION));
    sOutlines	= String(SW_RES(STR_CONTENT_TYPE_OUTLINE));
    sBookmarks  = String(SW_RES(STR_CONTENT_TYPE_BOOKMARK));
}
/* -----------------25.10.99 11:11-------------------

 --------------------------------------------------*/
SwXLinkTargetSupplier::~SwXLinkTargetSupplier()
{
}

/*-- 25.10.99 11:12:45---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXLinkTargetSupplier::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    if(!pxDoc)
        throw RuntimeException();
    String sToCompare(rName);
    String sSuffix('|');
    if(sToCompare == sTables)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToTable);

        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextTables(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference<XPropertySet>*)0));
    }
    else if(sToCompare == sFrames)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToFrame);
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextFrames(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sSections)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToRegion);
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getTextSections(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
/*	else if(sToCompare == )
    {
        sSuffix += UniString::CreateFromAscii(pMarkToText);
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((const XPropertySet*)0));
    }*/
    else if(sToCompare == sGraphics)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToGraphic);
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getGraphicObjects(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sOLEs)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToOLE);
        Reference< XNameAccess >  xTbls = new SwXLinkNameAccessWrapper(
                                        pxDoc->getEmbeddedObjects(), sToCompare, sSuffix );
        Reference< XPropertySet >  xRet(xTbls, UNO_QUERY);
        aRet.setValue(&xRet, ::getCppuType((Reference< XPropertySet>*)0));
    }
    else if(sToCompare == sOutlines)
    {
        sSuffix += UniString::CreateFromAscii(pMarkToOutline);
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
/*-- 25.10.99 11:12:46---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXLinkTargetSupplier::getElementNames(void)
                                        throw( RuntimeException )
{
    Sequence< OUString > aRet(7);
    OUString* pNames = aRet.getArray();
    pNames[0] = sTables;
    pNames[1] = sFrames  ;
    pNames[2] = sGraphics;
    pNames[3] = sOLEs	;
    pNames[4] = sSections;
    pNames[5] = sOutlines;
    pNames[6] = sBookmarks;
    return aRet;
}
/*-- 25.10.99 11:12:46---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkTargetSupplier::hasByName(const OUString& rName)
                                    throw( RuntimeException )
{
    String sToCompare(rName);
    if(	sToCompare == sTables  ||
        sToCompare == sFrames  ||
        sToCompare == sGraphics||
        sToCompare == sOLEs	  ||
        sToCompare == sSections ||
        sToCompare == sOutlines ||
        sToCompare == sBookmarks	)
        return sal_True;
    return sal_False;
}
/*-- 25.10.99 11:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXLinkTargetSupplier::getElementType(void)
                                    throw( RuntimeException )
{
    return ::getCppuType((Reference< XPropertySet>*)0);

}
/*-- 25.10.99 11:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkTargetSupplier::hasElements(void) throw( RuntimeException )
{
    return 0 != pxDoc;
}
/*-- 25.10.99 11:12:47---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXLinkTargetSupplier::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXLinkTargetSupplier");
}
/*-- 25.10.99 11:12:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkTargetSupplier::supportsService(const OUString& rServiceName)
                                                throw( RuntimeException )
{
    return (rServiceName == C2U("com.sun.star.document.LinkTargets"));
}
/*-- 25.10.99 11:12:48---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXLinkTargetSupplier::getSupportedServiceNames(void)
                                                throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pNames = aRet.getArray();
    pNames[0] = C2U("com.sun.star.document.LinkTargets");
    return aRet;
}

/*-- 26.10.99 09:16:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(
            Reference< XNameAccess >  xAccess, const String& rLinkDisplayName, String sSuffix ) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_LINK_TARGET)),
    sLinkDisplayName(rLinkDisplayName),
    xRealAccess(xAccess),
    sLinkSuffix(sSuffix),
    pxDoc(0)
{
}
/* -----------------26.10.99 14:17-------------------

 --------------------------------------------------*/
SwXLinkNameAccessWrapper::SwXLinkNameAccessWrapper(SwXTextDocument& rxDoc,
            const String& rLinkDisplayName, String sSuffix) :
    sLinkSuffix(sSuffix),
    sLinkDisplayName(rLinkDisplayName),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_LINK_TARGET)),
    pxDoc(&rxDoc),
    xDoc(&rxDoc)
{
}
/*-- 26.10.99 09:16:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXLinkNameAccessWrapper::~SwXLinkNameAccessWrapper()
{
}
/*-- 26.10.99 09:16:24---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXLinkNameAccessWrapper::getByName(const OUString& rName)
    throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    Any aRet;
    sal_Bool bFound = sal_False;
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
                sal_uInt16 nOutlineCount = pDoc->GetNodes().GetOutLineNds().Count();

                for (sal_uInt16 i = 0; i < nOutlineCount && !bFound; ++i)
                {
                    const SwNodes& rNds = pDoc->GetNodes();
                    if(sParam ==
                        rNds.GetOutLineNds()[ i ]->GetTxtNode()->GetExpandTxt( 0, STRING_LEN, sal_True ))
                    {
                        Reference< XPropertySet >  xOutline = new SwXOutlineTarget(sParam);
                        aRet.setValue(&xOutline, ::getCppuType((Reference<XPropertySet>*)0));
                        bFound = sal_True;
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
                bFound = sal_True;
            }
        }
    }
    if(!bFound)
        throw NoSuchElementException();
    return aRet;
}
/*-- 26.10.99 09:16:24---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXLinkNameAccessWrapper::getElementNames(void)
                                    throw( RuntimeException )
{
    Sequence< OUString > aRet;
    if(pxDoc)
    {
        if(!pxDoc->GetDocShell())
            throw RuntimeException();

        SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
        sal_uInt16 nOutlineCount = pDoc->GetNodes().GetOutLineNds().Count();
        aRet.realloc(nOutlineCount);
        OUString* pResArr = aRet.getArray();
        String sSuffix('|');
        sSuffix += UniString::CreateFromAscii(pMarkToOutline);
        for (sal_uInt16 i = 0; i < nOutlineCount; ++i)
        {
            const SwNodes& rNds = pDoc->GetNodes();
            String sEntry(
                rNds.GetOutLineNds()[ i ]->GetTxtNode()->GetExpandTxt( 0, STRING_LEN, sal_True ));
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
/*-- 26.10.99 09:16:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkNameAccessWrapper::hasByName(const OUString& rName)
    throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
    String sParam(rName);
    if(sParam.Len() > sLinkSuffix.Len() )
    {
        String sCmp = sParam.Copy(sParam.Len() - sLinkSuffix.Len(),
                                                    sLinkSuffix.Len());
        if(sCmp == sLinkSuffix)
        {
                sParam = sParam.Copy(0, sParam.Len() - sLinkSuffix.Len());
            if(pxDoc)
            {
                if(!pxDoc->GetDocShell())
                    throw RuntimeException();
                SwDoc* pDoc = pxDoc->GetDocShell()->GetDoc();
                sal_uInt16 nOutlineCount = pDoc->GetNodes().GetOutLineNds().Count();

                for (sal_uInt16 i = 0; i < nOutlineCount && !bRet; ++i)
                {
                    const SwNodes& rNds = pDoc->GetNodes();
                    if(sParam ==
                        rNds.GetOutLineNds()[ i ]->GetTxtNode()->GetExpandTxt( 0, STRING_LEN, sal_True ))
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
/*-- 26.10.99 09:16:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXLinkNameAccessWrapper::getElementType(void)
                                throw( RuntimeException )
{
    return ::getCppuType((Reference<XPropertySet>*)0);
}
/*-- 26.10.99 09:16:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkNameAccessWrapper::hasElements(void) throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
    if(pxDoc)
    {
        DBG_ERROR("not implemented");
    }
    else
    {
        bRet = xRealAccess->hasElements();
    }
    return bRet;
}
/*-- 26.10.99 09:16:26---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXLinkNameAccessWrapper::getPropertySetInfo(void)
                                        throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = aPropSet.getPropertySetInfo();
    return xRet;
}
/*-- 26.10.99 09:16:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLinkNameAccessWrapper::setPropertyValue(
    const OUString& PropertyName, const Any& aValue)
    throw( UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
             WrappedTargetException,
            RuntimeException)
{
    throw UnknownPropertyException();
}
/* -----------------------------08.12.99 11:10--------------------------------

 ---------------------------------------------------------------------------*/
Any lcl_GetDisplayBitmap(String sLinkSuffix)
{
    Any aRet;
    if(sLinkSuffix.Len())
        sLinkSuffix.Erase(0, 1);
    sal_uInt16 nImgId = USHRT_MAX;

    if(COMPARE_EQUAL == sLinkSuffix.CompareToAscii(pMarkToOutline))
        nImgId = CONTENT_TYPE_OUTLINE;
    else if(COMPARE_EQUAL  == sLinkSuffix.CompareToAscii(pMarkToTable))
        nImgId = CONTENT_TYPE_TABLE;
    else if(COMPARE_EQUAL  == sLinkSuffix.CompareToAscii(pMarkToFrame))
        nImgId = CONTENT_TYPE_FRAME;
    else if(COMPARE_EQUAL  == sLinkSuffix.CompareToAscii(pMarkToGraphic))
        nImgId = CONTENT_TYPE_GRAPHIC;
//	else if(== sLinkSuffix)
//		nImgId = CONTENT_TYPE_BOOKMARK;
    else if(COMPARE_EQUAL  == sLinkSuffix.CompareToAscii(pMarkToRegion))
        nImgId = CONTENT_TYPE_REGION;
    else if(COMPARE_EQUAL == sLinkSuffix.CompareToAscii(pMarkToOLE))
        nImgId = CONTENT_TYPE_OLE;
    else if(!sLinkSuffix.Len())
        nImgId = CONTENT_TYPE_BOOKMARK;
    if(USHRT_MAX != nImgId)
    {
    }
    return aRet;
}
/*-- 26.10.99 09:16:27---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 26.10.99 09:16:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLinkNameAccessWrapper::addPropertyChangeListener(
    const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}
/*-- 26.10.99 09:16:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLinkNameAccessWrapper::removePropertyChangeListener(
    const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}
/*-- 26.10.99 09:16:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLinkNameAccessWrapper::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}
/*-- 26.10.99 09:16:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLinkNameAccessWrapper::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{}
/*-- 26.10.99 09:16:32---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XNameAccess >  SwXLinkNameAccessWrapper::getLinks(void)
                                    throw( RuntimeException )
{
    return (SwXLinkNameAccessWrapper*)this;
}

/*-- 26.10.99 09:21:48---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXLinkNameAccessWrapper::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXLinkNameAccessWrapper");
}
/*-- 26.10.99 09:21:48---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXLinkNameAccessWrapper::supportsService(const OUString& rServiceName)
                                                    throw( RuntimeException )
{
    return (rServiceName == C2U("com.sun.star.document.LinkTargets"));
}
/*-- 26.10.99 09:21:48---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXLinkNameAccessWrapper::getSupportedServiceNames(void)
                                                    throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pNames = aRet.getArray();
    pNames[0] = C2U("com.sun.star.document.LinkTargets");
    return aRet;
}
/* -----------------26.10.99 15:50-------------------

 --------------------------------------------------*/
SwXOutlineTarget::SwXOutlineTarget(const String& rOutlineText) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_LINK_TARGET)),
    sOutlineText(rOutlineText)
{
}
/*-- 26.10.99 15:51:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXOutlineTarget::~SwXOutlineTarget()
{
}
/*-- 26.10.99 15:51:46---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXOutlineTarget::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xRet = aPropSet.getPropertySetInfo();
    return xRet;
}
/*-- 26.10.99 15:51:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXOutlineTarget::setPropertyValue(
    const OUString& PropertyName, const Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
         IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw UnknownPropertyException();
}
/*-- 26.10.99 15:51:46---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 26.10.99 15:51:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXOutlineTarget::addPropertyChangeListener(
    const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
/*-- 26.10.99 15:51:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXOutlineTarget::removePropertyChangeListener(
    const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
/*-- 26.10.99 15:51:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXOutlineTarget::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
/*-- 26.10.99 15:51:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXOutlineTarget::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXOutlineTarget::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXOutlineTarget");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXOutlineTarget::supportsService(const OUString& ServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.document.LinkTarget") == ServiceName;
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
Sequence< OUString > SwXOutlineTarget::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.document.LinkTarget");

    return aRet;
}
/* -----------------------------17.01.01 16:06--------------------------------

 ---------------------------------------------------------------------------*/
SwXDocumentPropertyHelper::SwXDocumentPropertyHelper(SwDoc& rDoc)
: m_pDoc(&rDoc)
, SvxUnoForbiddenCharsTable ( rDoc.GetForbiddenCharacterTbl() )
{
}
/* -----------------------------17.01.01 16:06--------------------------------

 ---------------------------------------------------------------------------*/
SwXDocumentPropertyHelper::~SwXDocumentPropertyHelper()
{
}
/* -----------------------------17.01.01 16:06--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XInterface> SwXDocumentPropertyHelper::GetDrawTable(short nWhich)
{
    Reference<XInterface> xRet;
    if(m_pDoc)
    {
        switch(nWhich)
        {
            case SW_CREATE_DASH_TABLE         :
                if(!xDashTable.is())
                    xDashTable = SvxUnoDashTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xDashTable;
            break;
            case SW_CREATE_GRADIENT_TABLE     :
                if(!xGradientTable.is())
                    xGradientTable = SvxUnoGradientTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xGradientTable;
            break;
            case SW_CREATE_HATCH_TABLE        :
                if(!xHatchTable.is())
                    xHatchTable = SvxUnoHatchTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xHatchTable;
            break;
            case SW_CREATE_BITMAP_TABLE       :
                if(!xBitmapTable.is())
                    xBitmapTable = SvxUnoBitmapTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xBitmapTable;
            break;
            case SW_CREATE_TRANSGRADIENT_TABLE:
                if(!xTransGradientTable.is())
                    xTransGradientTable = SvxUnoTransGradientTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xTransGradientTable;
            break;
            case SW_CREATE_MARKER_TABLE       :
                if(!xMarkerTable.is())
                    xMarkerTable = SvxUnoMarkerTable_createInstance( m_pDoc->GetDrawModel() );
                xRet = xMarkerTable;
            break;
            case  SW_CREATE_DRAW_DEFAULTS:
                if(!xDrawDefaults.is())
                    xDrawDefaults = (cppu::OWeakObject*)new SwSvxUnoDrawPool(m_pDoc);
                xRet = xDrawDefaults;
            break;
#ifdef DBG_UTIL
            default: DBG_ERROR("which table?");
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
