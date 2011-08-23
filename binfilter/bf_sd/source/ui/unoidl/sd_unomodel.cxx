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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>



#include <rtl/uuid.h>
#include <rtl/memory.h>

#include <bf_svx/unofield.hxx>




#include <bf_svx/UnoForbiddenCharsTable.hxx>

#include <bf_svx/forbiddencharacterstable.hxx>

#include <bf_svx/UnoNamespaceMap.hxx>

#include <bf_svx/unoapi.hxx>

#include <bf_svx/unofill.hxx>

#include <bf_svx/svdorect.hxx>

#include <toolkit/awt/vclxdevice.hxx>

#include <bf_svtools/unoimap.hxx>

#include <bf_svx/unolingu.hxx>
#include <bf_svx/svdpagv.hxx>

#include <bf_svx/unoshape.hxx>
#include <bf_svx/unonrule.hxx>

// #99870# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#include <bf_svx/xmleohlp.hxx>

#include <bf_svx/xmlgrhlp.hxx>

#include "bf_sd/docshell.hxx"

#include <UnoDocumentSettings.hxx>

#include <glob.hrc>
#include <sdresid.hxx>
#include <sdpage.hxx>

#include "unohelp.hxx"
#include <unolayer.hxx>
#include <unopage.hxx>
#include <unopres.hxx>
#include <unocpres.hxx>
#include <unoobj.hxx>
#include <unostyls.hxx>
#include <stlpool.hxx>
#include <unopback.hxx>
#include <unogstyl.hxx>
#include <unokywds.hxx>
#include <frmview.hxx>

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
namespace binfilter {

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;

extern uno::Reference< uno::XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel );

///////////////////////////////////////////////////////////////////////

class SdUnoForbiddenCharsTable : public SvxUnoForbiddenCharsTable,
                                 public SfxListener
{
private:
    SdrModel*	mpModel;

protected:
    virtual void onChange();

public:
    SdUnoForbiddenCharsTable( SdrModel* pModel );
    ~SdUnoForbiddenCharsTable();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();
};

SdUnoForbiddenCharsTable::SdUnoForbiddenCharsTable( SdrModel* pModel )
: SvxUnoForbiddenCharsTable( pModel->GetForbiddenCharsTable() ), mpModel( pModel )
{
    StartListening( *pModel );
}

void SdUnoForbiddenCharsTable::onChange()
{
    if( mpModel )
    {
        mpModel->ReformatAllTextObjects();
    }
}

SdUnoForbiddenCharsTable::~SdUnoForbiddenCharsTable()
{
    if( mpModel )
        EndListening( *mpModel );
}

void SdUnoForbiddenCharsTable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw()
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        if( HINT_MODELCLEARED == pSdrHint->GetKind() )
        {
            mpModel = NULL;
        }
    }
}

///////////////////////////////////////////////////////////////////////

const sal_Int32 WID_MODEL_LANGUAGE = 1;
const sal_Int32 WID_MODEL_TABSTOP  = 2;
const sal_Int32 WID_MODEL_VISAREA  = 3;
const sal_Int32 WID_MODEL_MAPUNIT  = 4;
const sal_Int32 WID_MODEL_FORBCHARS= 5;
const sal_Int32 WID_MODEL_CONTFOCUS = 6;
const sal_Int32 WID_MODEL_DSGNMODE	= 7;
const sal_Int32 WID_MODEL_BASICLIBS = 8;

const SfxItemPropertyMap* ImplGetDrawModelPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    const static SfxItemPropertyMap aDrawModelPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(sUNO_Prop_CharLocale),		  	WID_MODEL_LANGUAGE,	&::getCppuType((const lang::Locale*)0),		0,	0},
        { MAP_CHAR_LEN(sUNO_Prop_TabStop),				WID_MODEL_TABSTOP,	&::getCppuType((const sal_Int32*)0),		0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_VisibleArea),			WID_MODEL_VISAREA,	&::getCppuType((const awt::Rectangle*)0),	0,	0},
        { MAP_CHAR_LEN(sUNO_Prop_MapUnit),				WID_MODEL_MAPUNIT,	&::getCppuType((const sal_Int16*)0),		beans::PropertyAttribute::READONLY,	0},
        { MAP_CHAR_LEN(sUNO_Prop_ForbiddenCharacters),	WID_MODEL_FORBCHARS,&::getCppuType((const uno::Reference< i18n::XForbiddenCharacters > *)0), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN(sUNO_Prop_AutomContFocus ),	WID_MODEL_CONTFOCUS,	&::getBooleanCppuType(),					0,	0},
        { MAP_CHAR_LEN(sUNO_Prop_ApplyFrmDsgnMode),	WID_MODEL_DSGNMODE,		&::getBooleanCppuType(),					0,	0},
        { MAP_CHAR_LEN("BasicLibraries"),				WID_MODEL_BASICLIBS,&::getCppuType((const uno::Reference< script::XLibraryContainer > *)0), beans::PropertyAttribute::READONLY, 0 },
        { 0,0,0,0,0 }
    };

    return aDrawModelPropertyMap_Impl;
}

// this ctor is used from the DocShell
SdXImpressDocument::SdXImpressDocument( SdDrawDocShell* pShell ) throw()
:	SfxBaseModel( pShell ),
    pDocShell( pShell ),
    aPropSet( ImplGetDrawModelPropertyMap() ),
    mbClipBoard( sal_False )
{
    if( pDocShell )
    {
        pDoc = pDocShell->GetDoc();
        StartListening( *pDoc );
    }
    else
    {
        DBG_ERROR("DocShell is invalid");
    }

    mbImpressDoc = pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
SdXImpressDocument::~SdXImpressDocument() throw()
{
}

// uno helper


/******************************************************************************
* Erzeugt anhand der uebergebennen SdPage eine SdDrawPage. Wurde fuer diese   *
* SdPage bereits eine SdDrawPage erzeugt, wird keine neue SdDrawPage erzeug.  *
******************************************************************************/
/*
uno::Reference< drawing::XDrawPage >  SdXImpressDocument::CreateXDrawPage( SdPage* pPage ) throw()
{
    DBG_ASSERT(pPage,"SdXImpressDocument::CreateXDrawPage( NULL? )");

    uno::Reference< drawing::XDrawPage >  xDrawPage;

    if(pPage)
    {
        xDrawPage = SvxDrawPage::GetPageForSdrPage(pPage);

        if(!xDrawPage.is())
        {
            if(pPage->IsMasterPage())
            {
                xDrawPage = (presentation::XPresentationPage*)new SdMasterPage( this, pPage );
            }
            else
            {
                xDrawPage = (SvxDrawPage*)new SdDrawPage( this, pPage );
            }
        }
    }

    return xDrawPage;
}
*/

UNO3_GETIMPLEMENTATION_IMPL( SdXImpressDocument );

// XInterface
uno::Any SAL_CALL SdXImpressDocument::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aAny;

    QUERYINT(lang::XServiceInfo);
    else QUERYINT(beans::XPropertySet);
    else QUERYINT(lang::XMultiServiceFactory);
    else QUERYINT(drawing::XDrawPageDuplicator);
    else QUERYINT(drawing::XLayerSupplier);
    else QUERYINT(drawing::XMasterPagesSupplier);
    else QUERYINT(drawing::XDrawPagesSupplier);
    else QUERYINT(presentation::XHandoutMasterSupplier);
    else QUERYINT(document::XLinkTargetSupplier);
    else QUERYINT(style::XStyleFamiliesSupplier);
    else QUERYINT(lang::XUnoTunnel);
    else QUERYINT(::com::sun::star::ucb::XAnyCompareFactory);
    else QUERYINT(view::XRenderable);
    else if( mbImpressDoc && rType == ITYPE(presentation::XPresentationSupplier) )
            aAny <<= uno::Reference< presentation::XPresentationSupplier >(this);
    else if( mbImpressDoc && rType == ITYPE(presentation::XCustomPresentationSupplier) )
            aAny <<= uno::Reference< presentation::XCustomPresentationSupplier >(this);
    else
        return SfxBaseModel::queryInterface( rType );

    return aAny;
}

void SAL_CALL SdXImpressDocument::acquire() throw ( )
{
    SfxBaseModel::acquire();
}

void SAL_CALL SdXImpressDocument::release() throw ( )
{
    SfxBaseModel::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SdXImpressDocument::getTypes(  ) throw(uno::RuntimeException)
{
    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SfxBaseModel::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = mbImpressDoc ? 15 : 12;		// !DANGER! Keep this updated!

        maTypeSequence.realloc(  nBaseTypes + nOwnTypes );
        uno::Type* pTypes = maTypeSequence.getArray();

        *pTypes++ = ITYPE(beans::XPropertySet);
        *pTypes++ = ITYPE(lang::XServiceInfo);
        *pTypes++ = ITYPE(lang::XMultiServiceFactory);
        *pTypes++ = ITYPE(drawing::XDrawPageDuplicator);
        *pTypes++ = ITYPE(drawing::XLayerSupplier);
        *pTypes++ = ITYPE(drawing::XMasterPagesSupplier);
        *pTypes++ = ITYPE(drawing::XDrawPagesSupplier);
        *pTypes++ = ITYPE(document::XLinkTargetSupplier);
        *pTypes++ = ITYPE(style::XStyleFamiliesSupplier);
        *pTypes++ = ITYPE(lang::XUnoTunnel);
        *pTypes++ = ITYPE(::com::sun::star::ucb::XAnyCompareFactory);
        *pTypes++ = ITYPE(view::XRenderable);
        if( mbImpressDoc )
        {
            *pTypes++ = ITYPE(presentation::XPresentationSupplier);
            *pTypes++ = ITYPE(presentation::XCustomPresentationSupplier);
            *pTypes++ = ITYPE(presentation::XHandoutMasterSupplier);
        }

        for( sal_Int32 nType = 0; nType < nBaseTypes; nType++ )
            *pTypes++ = *pBaseTypes++;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SdXImpressDocument::getImplementationId(  ) throw(uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
void SdXImpressDocument::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( pDoc )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

        if( pSdrHint )
        {
            if( hasEventListeners() )
            {

                bool bBackgroundShape = false;

                // the background shape itself has no api representation, so filter all notifies for it
                const SdrObject* pSdrObj = pSdrHint->GetObject();
                if( pSdrObj && (pSdrObj->GetObjInventor() == SdrInventor) && (pSdrObj->GetObjIdentifier() == OBJ_RECT) )
                {
                    SdPage* pPage = (SdPage*)pSdrObj->GetPage();
                    bBackgroundShape = pPage && (pPage->GetPresObjKind(const_cast<SdrObject*>(pSdrObj)) == PRESOBJ_BACKGROUND);
                }

                if( !bBackgroundShape )
                {
                    document::EventObject aEvent;
                    if( SvxUnoDrawMSFactory::createEvent( pDoc, pSdrHint, aEvent ) )
                        notifyEvent( aEvent );
                }
            }

            if( pSdrHint->GetKind() == HINT_MODELCLEARED )
            {
                if( pDoc )
                    EndListening( *pDoc );
                pDoc = NULL;
                pDocShell = NULL;
            }
        }
        else
        {
            const SfxSimpleHint* pSfxHint = PTR_CAST(SfxSimpleHint, &rHint );

            // ist unser SdDrawDocument gerade gestorben?
            if(pSfxHint && pSfxHint->GetId() == SFX_HINT_DYING)
            {
                // yup, also schnell ein neues erfragen
                if( pDocShell )
                {
                    SdDrawDocument *pNewDoc = pDocShell->GetDoc();

                    // ist ueberhaupt ein neues da?
                    if( pNewDoc != pDoc )
                    {
                        pDoc = pNewDoc;
                        if(pDoc)
                            StartListening( *pDoc );
                    }
                }
            }
        }
    }
    SfxBaseModel::Notify( rBC, rHint );
}

/******************************************************************************
*                                                                             *
******************************************************************************/
SdPage* SdXImpressDocument::InsertSdPage( sal_uInt16 nPage, sal_Bool bDuplicate ) throw()
{
    sal_uInt16 nPageCount = pDoc->GetSdPageCount( PK_STANDARD );
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )), sal_False);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )), sal_False);

    SdPage* pStandardPage = NULL;

    if( 0 == nPageCount )
    {
        // this is only used for clipboard where we only have one page
        pStandardPage = (SdPage*) pDoc->AllocPage(sal_False);

        Size aDefSize(21000, 29700);   // A4-Hochformat
        pStandardPage->SetSize( aDefSize );
        pDoc->InsertPage(pStandardPage, 0);
    }
    else
    {
        // Hier wird die Seite ermittelt, hinter der eingefuegt werden soll
        SdPage* pPreviousStandardPage = pDoc->GetSdPage( Min( (sal_uInt16)(nPageCount - 1), nPage ), PK_STANDARD );

        sal_uInt16 nPos = 0;
        SetOfByte aVisibleLayers = pPreviousStandardPage->GetMasterPageVisibleLayers( nPos );
        sal_Bool bIsPageBack = aVisibleLayers.IsSet( aBckgrnd );
        sal_Bool bIsPageObj = aVisibleLayers.IsSet( aBckgrndObj );

        /**************************************************************
        * Es wird stets zuerst eine Standardseite und dann eine
        * Notizseite erzeugt. Es ist sichergestellt, dass auf eine
        * Standardseite stets die zugehoerige Notizseite folgt.
        **************************************************************/

        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        SdPage* pPreviousNotesPage = (SdPage*) pDoc->GetPage( nStandardPageNum - 1 );
        sal_uInt16 nNotesPageNum = nStandardPageNum + 1;
        String aStandardPageName;
        String aNotesPageName;

        /**************************************************************
        * Standardseite
        **************************************************************/
        if( bDuplicate )
            pStandardPage = (SdPage*) pPreviousStandardPage->Clone();
        else
            pStandardPage = (SdPage*) pDoc->AllocPage(sal_False);

        pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
        pStandardPage->SetBorder( pPreviousStandardPage->GetLftBorder(),
                                    pPreviousStandardPage->GetUppBorder(),
                                    pPreviousStandardPage->GetRgtBorder(),
                                    pPreviousStandardPage->GetLwrBorder() );

        pStandardPage->SetName(aStandardPageName);

        // Seite hinter aktueller Seite einfuegen
        pDoc->InsertPage(pStandardPage, nStandardPageNum);

        if( !bDuplicate )
        {
            // MasterPage der aktuellen Seite verwenden
            sal_uInt16 nPgNum = pPreviousStandardPage->GetMasterPageNum(nPos=0);
            pStandardPage->InsertMasterPage(nPgNum);
            pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
            pStandardPage->SetAutoLayout(AUTOLAYOUT_NONE, sal_True, sal_True);
        }

        aBckgrnd = rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )), sal_False);
        aBckgrndObj = rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )), sal_False);
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pStandardPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos=0);

        /**************************************************************
        * Notizseite
        **************************************************************/
        SdPage* pNotesPage = NULL;

        if( bDuplicate )
            pNotesPage = (SdPage*) pPreviousNotesPage->Clone();
        else
            pNotesPage = (SdPage*) pDoc->AllocPage(sal_False);

        pNotesPage->SetSize( pPreviousNotesPage->GetSize() );
        pNotesPage->SetBorder( pPreviousNotesPage->GetLftBorder(),
                                pPreviousNotesPage->GetUppBorder(),
                                pPreviousNotesPage->GetRgtBorder(),
                                pPreviousNotesPage->GetLwrBorder() );
        pNotesPage->SetName(aNotesPageName);
        pNotesPage->SetPageKind(PK_NOTES);

        // Seite hinter aktueller Seite einfuegen
        pDoc->InsertPage(pNotesPage, nNotesPageNum);

        if( !bDuplicate )
        {
            // MasterPage der aktuellen Seite verwenden
            sal_uInt16 nPgNum = pPreviousNotesPage->GetMasterPageNum(nPos=0);
            pNotesPage->InsertMasterPage(nPgNum);
            pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
            pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
        }
    }

    SetModified();

    return( pStandardPage );
}

void SdXImpressDocument::SetModified( sal_Bool bModified /* = sal_True */ ) throw()
{
    if( pDoc )
        pDoc->SetChanged( bModified );
}

// XModel
void SAL_CALL SdXImpressDocument::lockControllers(  )
    throw(uno::RuntimeException)
{
    if( NULL == pDoc )
        throw lang::DisposedException();

    pDoc->setLock( sal_True );
}

void SAL_CALL SdXImpressDocument::unlockControllers(  )
    throw(uno::RuntimeException)
{
    if( NULL == pDoc )
        throw lang::DisposedException();

    if( pDoc->isLocked() )
    {
        pDoc->setLock( sal_False );
    }
}

sal_Bool SAL_CALL SdXImpressDocument::hasControllersLocked(  )
    throw(uno::RuntimeException)
{
    return pDoc && pDoc->isLocked();
}
} //namespace binfilter
namespace binfilter {//STRIP009
uno::Reference < container::XIndexAccess > SAL_CALL SdXImpressDocument::getViewData() throw( uno::RuntimeException )
{
    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        List* pFrameViewList = pDoc->GetFrameViewList();

        if( pFrameViewList && pFrameViewList->Count() )
        {
            xRet = uno::Reference < container::XIndexAccess >::query(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.IndexedPropertyValues"))));


            uno::Reference < container::XIndexContainer > xCont( xRet, uno::UNO_QUERY );
            DBG_ASSERT( xCont.is(), "SdXImpressDocument::getViewData() failed for OLE object" );
            if( xCont.is() )
            {
                sal_uInt32 i;
                for( i = 0; i < pFrameViewList->Count(); i++ )
                {
                    FrameView* pFrameView = (FrameView*) pFrameViewList->GetObject(i);

                    if(pFrameView)
                    {
                        uno::Sequence< beans::PropertyValue > aSeq;
                        pFrameView->WriteUserDataSequence( aSeq );
                        xCont->insertByIndex( i, uno::makeAny( aSeq ) );
                    }
                }
            }
        }
    }

    return xRet;
}

void SAL_CALL SdXImpressDocument::setViewData( const uno::Reference < container::XIndexAccess >& xData ) throw(::com::sun::star::uno::RuntimeException)
{
    if( NULL == pDoc )
        throw lang::DisposedException();

    SfxBaseModel::setViewData( xData );
    if( pDocShell && (pDocShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED) && xData.is() )
    {
        const sal_Int32 nCount = xData->getCount();

        List* pFrameViewList = pDoc->GetFrameViewList();

        DBG_ASSERT( pFrameViewList, "No FrameViewList?" );
        if( pFrameViewList )
        {
            FrameView* pFrameView;

            sal_uInt32 i;
            for ( i = 0; i < pFrameViewList->Count(); i++)
            {
                // Ggf. FrameViews loeschen
                pFrameView = (FrameView*) pFrameViewList->GetObject(i);

                if (pFrameView)
                    delete pFrameView;
            }

            pFrameViewList->Clear();

            uno::Sequence< beans::PropertyValue > aSeq;
            sal_Int32 nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                if( xData->getByIndex( nIndex ) >>= aSeq )
                {
                    pFrameView = new FrameView( pDoc );
                    pFrameView->ReadUserDataSequence( aSeq );
                    pFrameViewList->Insert( pFrameView );
                }
            }
        }
    }
}

// XDrawPageDuplicator
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::duplicate( const uno::Reference< drawing::XDrawPage >& xPage )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    // pPage von xPage besorgen und dann die Id (nPos )ermitteln
    SvxDrawPage* pSvxPage = SvxDrawPage::getImplementation( xPage );
    if( pSvxPage )
    {
        SdPage* pPage = (SdPage*) pSvxPage->GetSdrPage();
        sal_uInt16 nPos = pPage->GetPageNum();
        nPos = ( nPos - 1 ) / 2;
        pPage = InsertSdPage( nPos, sal_True );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            return xDrawPage;
        }
    }

    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}


// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getDrawPages()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPages >  xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
    {
        initializeDocument();
        mxDrawPagesAccess = xDrawPages = (drawing::XDrawPages*)new SdDrawPagesAccess(*this);
    }

    return xDrawPages;
}

// XMasterPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getMasterPages()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPages >  xMasterPages( mxMasterPagesAccess );

    if( !xMasterPages.is() )
    {
        initializeDocument();
        mxMasterPagesAccess = xMasterPages = new SdMasterPagesAccess(*this);
    }

    return xMasterPages;
}

// XLayerManagerSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLayerManager(  )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess >  xLayerManager( mxLayerManager );

    if( !xLayerManager.is() )
        mxLayerManager = xLayerManager = new SdLayerManager(*this);

    return xLayerManager;
}

// XCustomPresentationSupplier
uno::Reference< container::XNameContainer > SAL_CALL SdXImpressDocument::getCustomPresentations()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameContainer >  xCustomPres( mxCustomPresentationAccess );

    if( !xCustomPres.is() )
        mxCustomPresentationAccess = xCustomPres = new SdXCustomPresentationAccess(*this);

    return xCustomPres;
}

// XPresentationSupplier
uno::Reference< presentation::XPresentation > SAL_CALL SdXImpressDocument::getPresentation()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< presentation::XPresentation >  aPresentation( mxPresentation );

    if( !aPresentation.is() )
        mxPresentation = aPresentation = new SdXPresentation(*this);

    return aPresentation;
}

// XHandoutMasterSupplier
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::getHandoutMasterPage()
    throw (uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xPage;

    if( pDoc )
    {
        initializeDocument();
        SdPage* pPage = pDoc->GetMasterSdPage( 0, PK_HANDOUT );
        if( pPage )
            xPage = uno::Reference< drawing::XDrawPage >::query( pPage->getUnoPage() );
    }
    return xPage;
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SdXImpressDocument::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.DashTable") ) )
    {
        if( !mxDashTable.is() )
            mxDashTable = SvxUnoDashTable_createInstance( pDoc );

        return mxDashTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.GradientTable") ) )
    {
        if( !mxGradientTable.is() )
            mxGradientTable = SvxUnoGradientTable_createInstance( pDoc );

        return mxGradientTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.HatchTable") ) )
    {
        if( !mxHatchTable.is() )
            mxHatchTable = SvxUnoHatchTable_createInstance( pDoc );

        return mxHatchTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.BitmapTable") ) )
    {
        if( !mxBitmapTable.is() )
            mxBitmapTable = SvxUnoBitmapTable_createInstance( pDoc );

        return mxBitmapTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TransparencyGradientTable") ) )
    {
        if( !mxTransGradientTable.is() )
            mxTransGradientTable = SvxUnoTransGradientTable_createInstance( pDoc );

        return mxTransGradientTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MarkerTable") ) )
    {
        if( !mxMarkerTable.is() )
            mxMarkerTable = SvxUnoMarkerTable_createInstance( pDoc );

        return mxMarkerTable;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.NumberingRules" ) ) )
    {
        return SvxCreateNumRule( pDoc );
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Background" ) ) )
    {
        return uno::Reference< uno::XInterface >( (uno::XWeak*)(new SdUnoPageBackground( pDoc )) );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.style.Style") ) )
    {
        uno::Reference< style::XStyle > xStyle( new SdUnoGraphicStyle() );
        return xStyle;
    }
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.Defaults") ) )
    {
        if( !mxDrawingPool.is() )
            mxDrawingPool = SdUnoCreatePool( pDoc );

        return mxDrawingPool;

    }

    if( aServiceSpecifier.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_Service_ImageMapRectangleObject) ) )
    {
        return SvUnoImageMapRectangleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( aServiceSpecifier.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_Service_ImageMapCircleObject) ) )
    {
        return SvUnoImageMapCircleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( aServiceSpecifier.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(sUNO_Service_ImageMapPolygonObject) ) )
    {
        return SvUnoImageMapPolygonObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( ( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.Settings") ) ) ||
        ( !mbImpressDoc && ( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.DocumentSettings") ) ) ) ||
        ( mbImpressDoc && ( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.DocumentSettings") ) ) ) )
    {
        return sd::DocumentSettings_createInstance( this );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextField.DateTime") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_EXT_DATEFIELD );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.xml.NamespaceMap") ) )
    {
        static sal_uInt16 aWhichIds[] = { SDRATTR_XMLATTRIBUTES, EE_CHAR_XMLATTRIBS, EE_PARA_XMLATTRIBS, 0 };

        return svx::NamespaceMap_createInstance( aWhichIds, &pDoc->GetItemPool() );
    }

    // #99870# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.ExportGraphicObjectResolver") ) )
    {
        return (::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.ImportGraphicObjectResolver") ) )
    {
        return (::cppu::OWeakObject * )new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.ExportEmbeddedObjectResolver") ) )
    {
        SvPersist *pPersist = pDoc ? pDoc->GetPersist() : NULL;

        if( NULL == pPersist )
            throw lang::DisposedException();

        return (::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pPersist, EMBEDDEDOBJECTHELPER_MODE_WRITE );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.ImportEmbeddedObjectResolver") ) )
    {
        SvPersist *pPersist = pDoc ? pDoc->GetPersist() : NULL;

        if( NULL == pPersist )
            throw lang::DisposedException();

        return (::cppu::OWeakObject * )new SvXMLEmbeddedObjectHelper( *pPersist, EMBEDDEDOBJECTHELPER_MODE_READ );
    }

    uno::Reference< uno::XInterface > xRet;

    const String aType( aServiceSpecifier );
    if( aType.EqualsAscii( "com.sun.star.presentation.", 0, 26 ) )
    {
        SvxShape* pShape = NULL;

        sal_uInt16 nType = OBJ_TEXT;
        // create a shape wrapper
        if( aType.EqualsAscii( "TitleTextShape", 26, 14 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "OutlinerShape", 26, 13 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "SubtitleShape", 26, 13 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "GraphicObjectShape", 26, 18 ) )
        {
            nType = OBJ_GRAF;
        }
        else if( aType.EqualsAscii( "PageShape", 26, 9 ) )
        {
            nType = OBJ_PAGE;
        }
        else if( aType.EqualsAscii( "OLE2Shape", 26, 9 ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.EqualsAscii( "ChartShape", 26, 10 ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.EqualsAscii( "TableShape", 26, 10 ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.EqualsAscii( "OrgChartShape", 26, 13 ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.EqualsAscii( "NotesShape", 26, 13 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "HandoutShape", 26, 13 ) )
        {
            nType = OBJ_PAGE;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor );

        // set shape type
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = (uno::XWeak*)pShape;
    }
    else
    {
        xRet = SvxFmMSFactory::createInstance( aServiceSpecifier );
    }

    uno::Reference< drawing::XShape > xShape( xRet, uno::UNO_QUERY );
    if( xShape.is() )
    {
        xRet.clear();
        new SdXShape( SvxShape::getImplementation( xShape ), (SdXImpressDocument*)this );
        xRet = xShape;
        xShape.clear();
    }

    return xRet;
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getAvailableServiceNames()
    throw(uno::RuntimeException)
{
    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( mbImpressDoc ? (26 + 4) : (15 + 4) );

    sal_uInt16 i(0);

    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DashTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GradientTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.HatchTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.BitmapTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TransparencyGradientTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MarkerTable"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.NumberingRules"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Background"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.Style"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_Service_ImageMapRectangleObject));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_Service_ImageMapCircleObject));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM(sUNO_Service_ImageMapPolygonObject));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.NamespaceMap"));

    // #99870# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ExportGraphicObjectResolver"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ImportGraphicObjectResolver"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ExportEmbeddedObjectResolver"));
    aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ImportEmbeddedObjectResolver"));

    if(mbImpressDoc)
    {
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TitleTextShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OutlinerShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.SubtitleShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.GraphicObjectShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.ChartShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PageShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OLE2Shape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TableShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OrgChartShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.NotesShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.HandoutShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.DocumentSettings"));
    }
    else
    {
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DocumentSettings"));
    }

    DBG_ASSERT( i == aSNS.getLength(), "Sequence overrun!" );

    return comphelper::concatSequences( aSNS_ORG, aSNS );
}

// lang::XServiceInfo
OUString SAL_CALL SdXImpressDocument::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdXImpressDocument"));
}

sal_Bool SAL_CALL SdXImpressDocument::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSeq( 4 );
    OUString* pServices = aSeq.getArray();

    *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OfficeDocument"));
    *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GenericDrawingDocument"));

    if( mbImpressDoc )
        *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"));
    else
        *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"));

    return aSeq;
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdXImpressDocument::getPropertySetInfo(  )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;
    return aPropSet.getPropertySetInfo();
}

void SAL_CALL SdXImpressDocument::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(aPropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            lang::Locale aLocale;
            if(!(aValue >>= aLocale))
                throw lang::IllegalArgumentException();

            pDoc->SetLanguage( SvxLocaleToLanguage(aLocale), EE_CHAR_LANGUAGE );
            break;
        }
        case WID_MODEL_TABSTOP:
        {
            sal_Int32 nValue;
            if(!(aValue >>= nValue) || nValue < 0 )
                throw lang::IllegalArgumentException();

            pDoc->SetDefaultTabulator((sal_uInt16)nValue);
            break;
        }
        case WID_MODEL_VISAREA:
            {
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                awt::Rectangle aVisArea;
                if( !(aValue >>= aVisArea) || (aVisArea.Width < 0) || (aVisArea.Height < 0) )
                    throw lang::IllegalArgumentException();

                pEmbeddedObj->SetVisArea( Rectangle( aVisArea.X, aVisArea.Y, aVisArea.X + aVisArea.Width - 1, aVisArea.Y + aVisArea.Height - 1 ) );
            }
            break;
        case WID_MODEL_CONTFOCUS:
            {
                sal_Bool bFocus;
                if( !(aValue >>= bFocus ) )
                    throw lang::IllegalArgumentException();
                pDoc->SetAutoControlFocus( bFocus );
            }
            break;
        case WID_MODEL_DSGNMODE:
            {
                sal_Bool bMode;
                if( !(aValue >>= bMode ) )
                    throw lang::IllegalArgumentException();
                pDoc->SetOpenInDesignMode( bMode );
            }
            break;
        case WID_MODEL_MAPUNIT:
        case WID_MODEL_BASICLIBS:
            throw beans::PropertyVetoException();
        default:
            throw beans::UnknownPropertyException();
            break;
    }

    SetModified();
}

uno::Any SAL_CALL SdXImpressDocument::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    uno::Any aAny;
    if( NULL == pDoc )
        throw lang::DisposedException();

    const SfxItemPropertyMap* pMap = aPropSet.getPropertyMapEntry(PropertyName);

    switch( pMap ? pMap->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            LanguageType eLang = pDoc->GetLanguage( EE_CHAR_LANGUAGE );
            lang::Locale aLocale;
            SvxLanguageToLocale( aLocale, eLang );
            aAny <<= aLocale;
            break;
        }
        case WID_MODEL_TABSTOP:
            aAny <<= (sal_Int32)pDoc->GetDefaultTabulator();
            break;
        case WID_MODEL_VISAREA:
            {
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                const Rectangle& aRect = pEmbeddedObj->GetVisArea();
                awt::Rectangle aVisArea( aRect.nLeft, aRect.nTop, aRect.getWidth(), aRect.getHeight() );

                aAny <<= aVisArea;
            }
            break;
        case WID_MODEL_MAPUNIT:
            {
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                sal_Int16 nMeasureUnit = 0;
                SvxMapUnitToMeasureUnit( pEmbeddedObj->GetMapUnit(), nMeasureUnit );
                aAny <<= (sal_Int16)nMeasureUnit;
        }
        break;
        case WID_MODEL_FORBCHARS:
        {
            aAny <<= getForbiddenCharsTable();
        }
        break;
        case WID_MODEL_CONTFOCUS:
            aAny <<= (sal_Bool)pDoc->GetAutoControlFocus();
            break;
        case WID_MODEL_DSGNMODE:
            aAny <<= pDoc->GetOpenInDesignMode();
            break;
        case WID_MODEL_BASICLIBS:
            aAny <<= pDocShell->GetBasicContainer();
            break;
        default:
            throw beans::UnknownPropertyException();
            break;
    }

    return aAny;
}

void SAL_CALL SdXImpressDocument::addPropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdXImpressDocument::removePropertyChangeListener( const OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdXImpressDocument::addVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}
void SAL_CALL SdXImpressDocument::removeVetoableChangeListener( const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException) {}

// XLinkTargetSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLinks()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    return new SdDocLinkTargets( *this );
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getStyleFamilies(  )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xStyles(mxStyleFamilies);

    if( !xStyles.is() )
        mxStyleFamilies = xStyles = new SdUnoStyleFamilies( this );

    return xStyles;
}

// XAnyCompareFactory
uno::Reference< ::com::sun::star::ucb::XAnyCompare > SAL_CALL SdXImpressDocument::createAnyCompareByName( const OUString& PropertyName )
    throw (uno::RuntimeException)
{
    return SvxCreateNumRuleCompare();
}

// XRenderable
sal_Int32 SAL_CALL SdXImpressDocument::getRendererCount( const uno::Any& rSelection,
                                                         const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;
    sal_Int32   nRet = 0;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Sequence< beans::PropertyValue > aRenderer;

    if( pDocShell && pDoc )
    {
        uno::Reference< frame::XModel > xModel;

        rSelection >>= xModel;

        if( xModel == pDocShell->GetModel() )
            nRet = pDoc->GetSdPageCount( PK_STANDARD );
        else
        {
            uno::Reference< drawing::XShapes > xShapes;

            rSelection >>= xShapes;

            if( xShapes.is() && xShapes->getCount() )
                nRet = 1;
        }
    }

    return nRet;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SdXImpressDocument::getRenderer( sal_Int32 nRenderer, const uno::Any& rSelection,
                                                                                const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Sequence< beans::PropertyValue > aRenderer;

    if( pDocShell && pDoc )
    {
        const Rectangle aVisArea( pDocShell->GetVisArea( ASPECT_DOCPRINT ) );
        awt::Size       aPageSize( aVisArea.GetWidth(), aVisArea.GetHeight() );

        aRenderer.realloc( 1 );

        aRenderer[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) );
        aRenderer[ 0 ].Value <<= aPageSize;
    }

    return aRenderer;
}

struct ImplRenderPaintProc
{
    const SdrLayerAdmin& rLayerAdmin;
    SdrPageView* pSdrPageView;

    sal_Bool IsVisible  ( const SdrObject* pObj ) const;
    sal_Bool IsPrintable( const SdrObject* pObj ) const;

    ImplRenderPaintProc( const SdrLayerAdmin& rLA, SdrPageView* pView ) :
        rLayerAdmin		( rLA ),
        pSdrPageView	( pView )
    {}

    DECL_LINK(_ImplRenderPaintProc, SdrPaintProcRec*);
};
sal_Bool ImplRenderPaintProc::IsVisible( const SdrObject* pObj ) const
{
    sal_Bool bVisible = sal_True;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayer( nLayerId );
        if ( pSdrLayer )
        {
            String aLayerName = pSdrLayer->GetName();
            bVisible = pSdrPageView->IsLayerVisible( aLayerName );
        }
    }
    return bVisible;
}
sal_Bool ImplRenderPaintProc::IsPrintable( const SdrObject* pObj ) const
{
    sal_Bool bPrintable = sal_True;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayer( nLayerId );
        if ( pSdrLayer )
        {
            String aLayerName = pSdrLayer->GetName();
            bPrintable = pSdrPageView->IsLayerPrintable( aLayerName );
        }
    }
    return bPrintable;

}
IMPL_LINK( ImplRenderPaintProc, _ImplRenderPaintProc, SdrPaintProcRec*, pRecord )
{
    SdrObject* pObj = pRecord->pObj;
    if( !pObj->IsEmptyPresObj() && IsVisible( pObj ) && IsPrintable( pObj ) )
    {
        pObj->Paint( pRecord->rOut, pRecord->rInfoRec );
    }
    else
    {
        if( pObj->GetPage()->IsMasterPage() && (pObj->GetPage() == pObj->GetObjList()) && (pObj->GetOrdNum() == 0) && pObj->ISA( SdrRectObj ) )
            pObj->Paint( pRecord->rOut, pRecord->rInfoRec );
    }
    return 0;
}

void SAL_CALL SdXImpressDocument::render( sal_Int32 nRenderer, const uno::Any& rSelection,
                                          const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == pDoc )
        throw lang::DisposedException();

    if( pDocShell && pDoc )
    {
        uno::Reference< awt::XDevice >  xRenderDevice;
        const sal_Int32					nPageNumber = nRenderer + 1;

        for( sal_Int32 nProperty = 0, nPropertyCount = rxOptions.getLength(); nProperty < nPropertyCount; ++nProperty )
        {
            if( rxOptions[ nProperty ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) )
                rxOptions[ nProperty].Value >>= xRenderDevice;
        }

        if( xRenderDevice.is() && nPageNumber && ( nPageNumber <= pDoc->GetSdPageCount( PK_STANDARD ) ) )
        {
            VCLXDevice*     pDevice = VCLXDevice::GetImplementation( xRenderDevice );
            OutputDevice*   pOut = pDevice ? pDevice->GetOutputDevice() : NULL;

            if( pOut )
            {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SdClientView*			pView = new SdClientView( pDocShell, pOut, NULL );
            }
        }
    }
}

uno::Reference< i18n::XForbiddenCharacters > SdXImpressDocument::getForbiddenCharsTable()
{
    uno::Reference< i18n::XForbiddenCharacters > xForb(mxForbidenCharacters);

    if( !xForb.is() )
        mxForbidenCharacters = xForb = new SdUnoForbiddenCharsTable( pDoc );

    return xForb;
}

void SdXImpressDocument::initializeDocument()
{
    if( (pDoc->GetPageCount() <= 1) && !mbClipBoard )
    {
        pDoc->CreateFirstPages();
    }
}

void SAL_CALL SdXImpressDocument::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    {
        ::SolarMutexGuard aGuard;
        if( pDoc )
        {
            EndListening( *pDoc );
            pDoc = NULL;
        }
    }

    SfxBaseModel::dispose();
}

// -----------------------------------------------------------------------------
void SAL_CALL SdXImpressDocument::setPrinter( const uno::Sequence< beans::PropertyValue >& rPrinter)
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    SfxPrinter* pPrinter = NULL;
    sal_uInt16 nChangeFlags = 0;
    SfxViewShell *pViewSh=0;
    impl_setPrinter(rPrinter,pPrinter,nChangeFlags,pViewSh);
}

//=============================================================================
// class SdDrawPagesAccess
//=============================================================================

SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  throw()
:	rModel(rMyModel)
{
    rModel.acquire();
}

SdDrawPagesAccess::~SdDrawPagesAccess() throw()
{
    rModel.release();
}

// XIndexAccess
sal_Int32 SAL_CALL SdDrawPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    return rModel.pDoc->GetSdPageCount( PK_STANDARD );
}

uno::Any SAL_CALL SdDrawPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= rModel.pDoc->GetSdPageCount( PK_STANDARD ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = rModel.pDoc->GetSdPage( (sal_uInt16)Index, PK_STANDARD );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XNameAccess
uno::Any SAL_CALL SdDrawPagesAccess::getByName( const OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    if( aName.getLength() != 0 )
    {
        const sal_uInt16 nCount = rModel.pDoc->GetSdPageCount( PK_STANDARD );
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nCount; nPage++ )
        {
            SdPage* pPage = rModel.pDoc->GetSdPage( nPage, PK_STANDARD );
            if(NULL == pPage)
                continue;

            if( aName == SdDrawPage::getPageApiName( pPage ) )
            {
                uno::Any aAny;
                uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
                aAny <<= xDrawPage;
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getElementNames() throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    const sal_uInt16 nCount = rModel.pDoc->GetSdPageCount( PK_STANDARD );
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();

    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = rModel.pDoc->GetSdPage( nPage, PK_STANDARD );
        *pNames++ = SdDrawPage::getPageApiName( pPage );
    }

    return aNames;
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasByName( const OUString& aName ) throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    const sal_uInt16 nCount = rModel.pDoc->GetSdPageCount( PK_STANDARD );
    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = rModel.pDoc->GetSdPage( nPage, PK_STANDARD );
        if(NULL == pPage)
            continue;

        if( aName == SdDrawPage::getPageApiName( pPage ) )
            return sal_True;
    }

    return sal_False;
}

// XElementAccess
uno::Type SAL_CALL SdDrawPagesAccess::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE( drawing::XDrawPage );
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasElements()
    throw(uno::RuntimeException)
{
    return getCount() > 0;
}

// XDrawPages

/******************************************************************************
* Erzeugt eine neue Seite mit Model an der angegebennen Position und gibt die *
* dazugehoerige SdDrawPage zurueck.                                           *
******************************************************************************/
uno::Reference< drawing::XDrawPage > SAL_CALL SdDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    if( rModel.pDoc )
    {
        SdPage* pPage = rModel.InsertSdPage( (sal_uInt16)nIndex );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            return xDrawPage;
        }
    }
    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}

/******************************************************************************
* Entfernt die angegebenne SdDrawPage aus dem Model und aus der internen      *
* Liste. Dies funktioniert nur, wenn mindestens eine *normale* Seite im Model *
* nach dem entfernen dieser Seite vorhanden ist.							  *
******************************************************************************/
void SAL_CALL SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
        throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    sal_uInt16 nPageCount = rModel.pDoc->GetSdPageCount( PK_STANDARD );
    if( nPageCount > 1 )
    {
        // pPage von xPage besorgen und dann die Id (nPos )ermitteln
        SdDrawPage* pSvxPage = SdDrawPage::getImplementation( xPage );
        if( pSvxPage )
        {
            SdPage* pPage = (SdPage*) pSvxPage->GetSdrPage();
            if(pPage)
            {
                // Es duerfen nur Standardpages DIREKT geloescht werden
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    sal_uInt16 nPage = pPage->GetPageNum();
                    rModel.pDoc->RemovePage( nPage );

                    // Die darauffolgende Seite ist die dazugeoerige Notizseite
                    rModel.pDoc->RemovePage( nPage );
                }
            }
            pSvxPage->Invalidate();
        }
    }

    rModel.SetModified();
}

// XServiceInfo
sal_Char pSdDrawPagesAccessService[sizeof("com.sun.star.drawing.DrawPages")] = "com.sun.star.drawing.DrawPages";

OUString SAL_CALL SdDrawPagesAccess::getImplementationName(  ) throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdDrawPagesAccess" ) );
}

sal_Bool SAL_CALL SdDrawPagesAccess::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( pSdDrawPagesAccessService ) );
}

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pSdDrawPagesAccessService ) );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

//=============================================================================
// class SdMasterPagesAccess
//=============================================================================

SdMasterPagesAccess::SdMasterPagesAccess( SdXImpressDocument& rMyModel ) throw()
:	rModel(rMyModel)
{
    rModel.acquire();
}

SdMasterPagesAccess::~SdMasterPagesAccess() throw()
{
    rModel.release();
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    return rModel.pDoc->GetMasterSdPageCount(PK_STANDARD);
}

/******************************************************************************
* Liefert ein drawing::XDrawPage Interface fuer den Zugriff auf die Masterpage and der *
* angegebennen Position im Model.                                             *
******************************************************************************/
uno::Any SAL_CALL SdMasterPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= rModel.pDoc->GetMasterSdPageCount( PK_STANDARD ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = rModel.pDoc->GetMasterSdPage( (sal_uInt16)Index, PK_STANDARD );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XElementAccess
uno::Type SAL_CALL SdMasterPagesAccess::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE(drawing::XDrawPage);
}

sal_Bool SAL_CALL SdMasterPagesAccess::hasElements()
    throw(uno::RuntimeException)
{
    return getCount() > 0;
}

// XDrawPages
uno::Reference< drawing::XDrawPage > SAL_CALL SdMasterPagesAccess::insertNewByIndex( sal_Int32 nInsertPos )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xDrawPage;

    SdDrawDocument* pDoc = rModel.pDoc;
    if( pDoc )
    {
        // calculate internal index and check for range errors
        const sal_Int32 nMPageCount = pDoc->GetMasterPageCount();
        nInsertPos = nInsertPos * 2 + 1;
        if( nInsertPos < 0 || nInsertPos > nMPageCount )
            nInsertPos = nMPageCount;

        // now generate a unique name for the new masterpage
        const String aStdPrefix( SdResId(STR_LAYOUT_DEFAULT_NAME) );
        String aPrefix( aStdPrefix );

        sal_Bool bUnique = sal_True;
        sal_Int32 i = 0;
        do
        {
            bUnique = sal_True;
            for( sal_Int32 nMaster = 1; nMaster < nMPageCount; nMaster++ )
            {
                SdPage* pPage = (SdPage*)pDoc->GetMasterPage((USHORT)nMaster);
                if( pPage && pPage->GetName() == aPrefix )
                {
                    bUnique = sal_False;
                    break;
                }
            }

            if( !bUnique )
            {
                i++;
                aPrefix = aStdPrefix;
                aPrefix += sal_Unicode( ' ' );
                aPrefix += String::CreateFromInt32( i );
            }

        } while( !bUnique );

        String aLayoutName( aPrefix );
        aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
        aLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

        // create styles
        ((SdStyleSheetPool*)pDoc->GetStyleSheetPool())->CreateLayoutStyleSheets( aPrefix );

        // get the first page for initial size and border settings
        SdPage* pPage = rModel.pDoc->GetSdPage( (sal_uInt16)0, PK_STANDARD );
        SdPage* pRefNotesPage = rModel.pDoc->GetSdPage( (sal_uInt16)0, PK_NOTES);

        // create and instert new draw masterpage
        SdPage* pMPage = (SdPage*)rModel.pDoc->AllocPage(sal_True);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLftBorder(),
                           pPage->GetUppBorder(),
                           pPage->GetRgtBorder(),
                           pPage->GetLwrBorder() );
        pDoc->InsertMasterPage(pMPage,  (USHORT)nInsertPos);
        pMPage->SetLayoutName( aLayoutName );

        { // insert background object
            Point aBackgroundPos ( pMPage->GetLftBorder(), pMPage->GetUppBorder() );
            Size aBackgroundSize ( pMPage->GetSize() );
            aBackgroundSize.Width()  -= pMPage->GetLftBorder() + pMPage->GetRgtBorder() - 1;
            aBackgroundSize.Height() -= pMPage->GetUppBorder() + pMPage->GetLwrBorder() - 1;
            Rectangle aBackgroundRect (aBackgroundPos, aBackgroundSize);
            pMPage->CreatePresObj(PRESOBJ_BACKGROUND, FALSE, aBackgroundRect, sal_True );
        }

        xDrawPage = uno::Reference< drawing::XDrawPage >::query( pMPage->getUnoPage() );

        // create and instert new notes masterpage
        SdPage* pMNotesPage = (SdPage*)rModel.pDoc->AllocPage(sal_True);
        pMNotesPage->SetSize( pRefNotesPage->GetSize() );
        pMNotesPage->SetPageKind(PK_NOTES);
        pMNotesPage->SetBorder( pRefNotesPage->GetLftBorder(),
                                pRefNotesPage->GetUppBorder(),
                                pRefNotesPage->GetRgtBorder(),
                                pRefNotesPage->GetLwrBorder() );
        pDoc->InsertMasterPage(pMNotesPage,  nInsertPos + 1);
//		pMNotesPage->InsertMasterPage( pMPage->GetPageNum() );
        pMNotesPage->SetLayoutName( aLayoutName );
        pMNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
        rModel.SetModified();
    }

    return( xDrawPage );
}

/******************************************************************************
* Entfernt die angegebenne SdMasterPage aus dem Model und aus der internen    *
* Liste. Dies funktioniert nur, wenn keine *normale* Seite im Model diese     *
* Seite als Masterpage benutzt.                                               *
******************************************************************************/
void SAL_CALL SdMasterPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.pDoc )
        throw lang::DisposedException();

    SdMasterPage* pSdPage = SdMasterPage::getImplementation( xPage );
    if(pSdPage == NULL)
        return;

    SdrPage* pSdrPage = pSdPage->GetSdrPage();

    DBG_ASSERT( pSdrPage->IsMasterPage(), "SdMasterPage is not masterpage?");

    if(rModel.pDoc->GetMasterPageUserCount(pSdrPage) > 0)
        return; //Todo: hier fehlt eine uno::Exception

    sal_uInt16 nCount = rModel.pDoc->GetMasterPageCount();
    for( sal_uInt16 nPgNum = 0; nPgNum < nCount; nPgNum++ )
    {
        if(rModel.pDoc->GetMasterPage(nPgNum) == pSdrPage)
        {
            rModel.pDoc->DeleteMasterPage(nPgNum);
            break;
        }
    }

    pSdPage->Invalidate();
}

// XServiceInfo
sal_Char pSdMasterPagesAccessService[sizeof("com.sun.star.drawing.MasterPages")] = "com.sun.star.drawing.MasterPages";

OUString SAL_CALL SdMasterPagesAccess::getImplementationName(  ) throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SdMasterPagesAccess" ) );
}

sal_Bool SAL_CALL SdMasterPagesAccess::supportsService( const OUString& ServiceName ) throw(uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( pSdMasterPagesAccessService ) );
}

uno::Sequence< OUString > SAL_CALL SdMasterPagesAccess::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( pSdMasterPagesAccessService ) );
    uno::Sequence< OUString > aSeq( &aService, 1 );
    return aSeq;
}

//=============================================================================
// class SdDocLinkTargets
//=============================================================================

SdDocLinkTargets::SdDocLinkTargets( SdXImpressDocument& rMyModel ) throw()
:	rModel( rMyModel )
{
    rModel.acquire();
}

SdDocLinkTargets::~SdDocLinkTargets() throw()
{
    rModel.release();
}

// XNameAccess
uno::Any SAL_CALL SdDocLinkTargets::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.GetDoc() )
        throw lang::DisposedException();

    SdPage* pPage = FindPage( aName );

    if( pPage == NULL )
        throw container::NoSuchElementException();

    uno::Any aAny;

    uno::Reference< beans::XPropertySet > xProps( pPage->getUnoPage(), uno::UNO_QUERY );
    if( xProps.is() )
        aAny <<= xProps;

    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getElementNames()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.GetDoc() )
        throw lang::DisposedException();

    SdDrawDocument* pDoc = rModel.GetDoc();
    if( pDoc == NULL )
    {
        uno::Sequence< OUString > aSeq;
        return aSeq;
    }

    const sal_uInt16 nMaxPages = pDoc->GetPageCount();
    const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

    uno::Sequence< OUString > aSeq( nMaxPages + nMaxMasterPages );
    OUString* pStr = aSeq.getArray();

    sal_uInt16 nPage;
    // standard pages
    for( nPage = 0; nPage < nMaxPages; nPage++ )
        *pStr++ = ((SdPage*)pDoc->GetPage( nPage ))->GetName();

    // master pages
    for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
        *pStr++ = ((SdPage*)pDoc->GetMasterPage( nPage ))->GetName();

    return aSeq;
}

sal_Bool SAL_CALL SdDocLinkTargets::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == rModel.GetDoc() )
        throw lang::DisposedException();

    return FindPage( aName ) != NULL;
}

// container::XElementAccess
uno::Type SAL_CALL SdDocLinkTargets::getElementType()
    throw(uno::RuntimeException)
{
    return ITYPE(beans::XPropertySet);
}

sal_Bool SAL_CALL SdDocLinkTargets::hasElements()
    throw(uno::RuntimeException)
{
    SdDrawDocument* pDoc = rModel.GetDoc();
    return pDoc != NULL;
}

SdPage* SdDocLinkTargets::FindPage( const OUString& rName ) const throw()
{
    SdDrawDocument* pDoc = rModel.GetDoc();
    if( pDoc == NULL )
        return NULL;

    const sal_uInt16 nMaxPages = pDoc->GetPageCount();
    const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

    sal_uInt16 nPage;
    SdPage* pPage;

    const String aName( rName );

    // standard pages
    for( nPage = 0; nPage < nMaxPages; nPage++ )
    {
        pPage = (SdPage*)pDoc->GetPage( nPage );
        if( pPage->GetName() == aName )
            return pPage;
    }

    // master pages
    for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
    {
        pPage = (SdPage*)pDoc->GetMasterPage( nPage );
        if( pPage->GetName() == aName )
            return pPage;
    }

    return NULL;
}

// XServiceInfo
OUString SAL_CALL SdDocLinkTargets::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SdDocLinkTargets") );
}

sal_Bool SAL_CALL SdDocLinkTargets::supportsService( const OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    const OUString aSN( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.LinkTargets") );
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
