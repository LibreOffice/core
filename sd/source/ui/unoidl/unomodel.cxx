/*************************************************************************
 *
 *  $RCSfile: unomodel.cxx,v $
 *
 *  $Revision: 1.77 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 10:31:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _UTL_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <rtl/uuid.h>
#include <rtl/memory.h>

#ifndef _SVX_UNOFIELD_HXX
#include <svx/unofield.hxx>
#endif

#ifndef _UNOMODEL_HXX
#include <unomodel.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#include <svx/UnoForbiddenCharsTable.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#ifndef _SVX_UNONAMESPACEMAP_HXX_
#include <svx/UnoNamespaceMap.hxx>
#endif

#ifndef _SVDLAYER_HXX //autogen
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVDSOB_HXX //autogen
#include <svx/svdsob.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif

#ifndef _SVX_UNOFILL_HXX_
#include <svx/unofill.hxx>
#endif

#ifndef _SVX_UNOPOOL_HXX_
#include <svx/unopool.hxx>
#endif
#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXDEVICE_HXX_
#include <toolkit/awt/vclxdevice.hxx>
#endif

#include <svtools/unoimap.hxx>

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#include <svx/unoshape.hxx>
#include <svx/unonrule.hxx>
#include <svx/eeitem.hxx>

// #99870# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif

#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

#ifndef _SD_UNODOCUMENTSETTINGS_HXX_
#include <UnoDocumentSettings.hxx>
#endif

#include <drawdoc.hxx>
#include <glob.hrc>
#include <sdresid.hxx>
#include <sdpage.hxx>

#include "unohelp.hxx"
#include <unolayer.hxx>
#include <unoprnms.hxx>
#include <unopage.hxx>
#include <unopres.hxx>
#include <unocpres.hxx>
#include <unoobj.hxx>
#include <unostyls.hxx>
#include <stlpool.hxx>
#include <unopback.hxx>
#include <unogstyl.hxx>
#include <unokywds.hxx>
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_CLIENT_VIEW_HXX
#include "ClientView.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "app.hrc"

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

using namespace ::osl;
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

extern uno::Reference< uno::XInterface > SdUnoCreatePool( SdDrawDocument* pDrawModel );

///////////////////////////////////////////////////////////////////////

class SdUnoForbiddenCharsTable : public SvxUnoForbiddenCharsTable,
                                 public SfxListener
{
public:
    SdUnoForbiddenCharsTable( SdrModel* pModel );
    ~SdUnoForbiddenCharsTable();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();
protected:
    virtual void onChange();

private:
    SdrModel*   mpModel;
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
const sal_Int32 WID_MODEL_DSGNMODE  = 7;
const sal_Int32 WID_MODEL_BASICLIBS = 8;
const sal_Int32 WID_MODEL_RUNTIMEUID = 9;

const SfxItemPropertyMap* ImplGetDrawModelPropertyMap()
{
    // Achtung: Der erste Parameter MUSS sortiert vorliegen !!!
    const static SfxItemPropertyMap aDrawModelPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN(sUNO_Prop_CharLocale),           WID_MODEL_LANGUAGE, &::getCppuType((const lang::Locale*)0),     0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_TabStop),              WID_MODEL_TABSTOP,  &::getCppuType((const sal_Int32*)0),        0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_VisibleArea),          WID_MODEL_VISAREA,  &::getCppuType((const awt::Rectangle*)0),   0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_MapUnit),              WID_MODEL_MAPUNIT,  &::getCppuType((const sal_Int16*)0),        beans::PropertyAttribute::READONLY, 0},
        { MAP_CHAR_LEN(sUNO_Prop_ForbiddenCharacters),  WID_MODEL_FORBCHARS,&::getCppuType((const uno::Reference< i18n::XForbiddenCharacters > *)0), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN(sUNO_Prop_AutomContFocus ),  WID_MODEL_CONTFOCUS,    &::getBooleanCppuType(),                    0,  0},
        { MAP_CHAR_LEN(sUNO_Prop_ApplyFrmDsgnMode), WID_MODEL_DSGNMODE,     &::getBooleanCppuType(),                    0,  0},
        { MAP_CHAR_LEN("BasicLibraries"),               WID_MODEL_BASICLIBS,&::getCppuType((const uno::Reference< script::XLibraryContainer > *)0), beans::PropertyAttribute::READONLY, 0 },
        { MAP_CHAR_LEN(sUNO_Prop_RuntimeUID),           WID_MODEL_RUNTIMEUID,   &::getCppuType(static_cast< const rtl::OUString * >(0)), beans::PropertyAttribute::READONLY, 0 },
        { 0,0,0,0,0 }
    };

    return aDrawModelPropertyMap_Impl;
}

// this ctor is used from the DocShell
SdXImpressDocument::SdXImpressDocument (::sd::DrawDocShell* pShell ) throw()
:   SfxBaseModel( pShell ),
    pDocShell( pShell ),
    aPropSet( ImplGetDrawModelPropertyMap() ),
    mbClipBoard( sal_False ),
    mbDisposed(false)
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

SdXImpressDocument::SdXImpressDocument( SdDrawDocument* _pDoc, sal_Bool bClipBoard ) throw()
:   SfxBaseModel( NULL ),
    pDocShell( NULL ),
    aPropSet( ImplGetDrawModelPropertyMap() ),
    pDoc( _pDoc ),
    mbClipBoard( bClipBoard ),
    mbDisposed(false)
{
    if( pDoc )
    {
        StartListening( *pDoc );
    }
    else
    {
        DBG_ERROR("SdDrawDocument is invalid");
    }

    mbImpressDoc = pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS;
}

/***********************************************************************
*                                                                      *
***********************************************************************/
SdXImpressDocument::~SdXImpressDocument() throw()
{
    dispose();
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

UNO3_GETIMPLEMENTATION2_IMPL( SdXImpressDocument, SfxBaseModel );

// XInterface
uno::Any SAL_CALL SdXImpressDocument::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    else QUERYINT(com::sun::star::ucb::XAnyCompareFactory);
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( maTypeSequence.getLength() == 0 )
    {
        const uno::Sequence< uno::Type > aBaseTypes( SfxBaseModel::getTypes() );
        const sal_Int32 nBaseTypes = aBaseTypes.getLength();
        const uno::Type* pBaseTypes = aBaseTypes.getConstArray();

        const sal_Int32 nOwnTypes = mbImpressDoc ? 14 : 11;     // !DANGER! Keep this updated!

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
        *pTypes++ = ITYPE(com::sun::star::ucb::XAnyCompareFactory);
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
    OGuard aGuard( Application::GetSolarMutex() );

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
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);

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
        SetOfByte aVisibleLayers = pPreviousStandardPage->TRG_GetMasterPageVisibleLayers();
        sal_Bool bIsPageBack = aVisibleLayers.IsSet( aBckgrnd );
        sal_Bool bIsPageObj = aVisibleLayers.IsSet( aBckgrndObj );

        // AutoLayouts muessen fertig sein
        pDoc->StopWorkStartupDelay();

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
        pStandardPage->SetOrientation( pPreviousStandardPage->GetOrientation() );
        pStandardPage->SetName(aStandardPageName);

        // Seite hinter aktueller Seite einfuegen
        pDoc->InsertPage(pStandardPage, nStandardPageNum);

        if( !bDuplicate )
        {
            // MasterPage der aktuellen Seite verwenden
            pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());
            pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
            pStandardPage->SetAutoLayout(AUTOLAYOUT_NONE, sal_True );
        }

        aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
        aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pStandardPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

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
        pNotesPage->SetOrientation( pPreviousNotesPage->GetOrientation() );
        pNotesPage->SetName(aNotesPageName);
        pNotesPage->SetPageKind(PK_NOTES);

        // Seite hinter aktueller Seite einfuegen
        pDoc->InsertPage(pNotesPage, nNotesPageNum);

        if( !bDuplicate )
        {
            // MasterPage der aktuellen Seite verwenden
            pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());
            pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
            pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True );
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
void SAL_CALL SdXImpressDocument    ::lockControllers(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    pDoc->setLock( sal_True );
}

void SAL_CALL SdXImpressDocument::unlockControllers(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    return pDoc && pDoc->isLocked();
}

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

uno::Reference < container::XIndexAccess > SAL_CALL SdXImpressDocument::getViewData() throw( uno::RuntimeException )
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        List* pFrameViewList = pDoc->GetFrameViewList();

        if( pFrameViewList && pFrameViewList->Count() )
        {
            xRet = uno::Reference < container::XIndexAccess >::query(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.IndexedPropertyValues"))));


            uno::Reference < container::XIndexContainer > xCont( xRet, uno::UNO_QUERY );
            DBG_ASSERT( xCont.is(), "SdXImpressDocument::getViewData() failed for OLE object" );
            if( xCont.is() )
            {
                sal_uInt32 i;
                for( i = 0; i < pFrameViewList->Count(); i++ )
                {
                    ::sd::FrameView* pFrameView =
                          static_cast< ::sd::FrameView*>(
                              pFrameViewList->GetObject(i));

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
    OGuard aGuard( Application::GetSolarMutex() );

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
            ::sd::FrameView* pFrameView;

            sal_uInt32 i;
            for ( i = 0; i < pFrameViewList->Count(); i++)
            {
                // Ggf. FrameViews loeschen
                pFrameView = static_cast< ::sd::FrameView*>(
                    pFrameViewList->GetObject(i));

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
                    pFrameView = new ::sd::FrameView( pDoc );
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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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
        return uno::Reference< uno::XInterface >(
            static_cast<uno::XWeak*>(new SdUnoPageBackground( pDoc )));
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.style.Style") ) )
    {
        uno::Reference<uno::XInterface> xStyle(
            static_cast<uno::XWeak*>(new SdUnoGraphicStyle()));
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

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.TextField.Header") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_HEADERFIELD );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.TextField.Footer") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_FOOTERFIELD );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.TextField.DateTime") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_DATETIMEFIELD );
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
        else if( aType.EqualsAscii( "FooterShape", 26, 12 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "HeaderShape", 26, 12 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "SlideNumberShape", 26, 17 ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.EqualsAscii( "DateTimeShape", 26, 17 ) )
        {
            nType = OBJ_TEXT;
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( mbImpressDoc ? (30 + 4) : (19 + 4) );

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
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.FooterShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.HeaderShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.SlideNumberShape"));
        aSNS[i++] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.DateTimeShape"));
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
    OGuard aGuard( Application::GetSolarMutex() );

    if (
        (ServiceName.equalsAscii("com.sun.star.document.OfficeDocument"       )) ||
        (ServiceName.equalsAscii("com.sun.star.drawing.GenericDrawingDocument")) ||
        (ServiceName.equalsAscii("com.sun.star.drawing.DrawingDocumentFactory"))
       )
    {
        return sal_True;
    }

    return (
            ( mbImpressDoc && ServiceName.equalsAscii("com.sun.star.presentation.PresentationDocument")) ||
            (!mbImpressDoc && ServiceName.equalsAscii("com.sun.star.drawing.DrawingDocument"          ))
           );
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getSupportedServiceNames() throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< OUString > aSeq( 4 );
    OUString* pServices = aSeq.getArray();

    *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OfficeDocument"));
    *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GenericDrawingDocument"));
    *pServices++ = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocumentFactory"));

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
    OGuard aGuard( Application::GetSolarMutex() );
    return aPropSet.getPropertySetInfo();
}

void SAL_CALL SdXImpressDocument::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
        case WID_MODEL_RUNTIMEUID: // is read-only
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
    OGuard aGuard( Application::GetSolarMutex() );

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

                DBG_ASSERT( (aVisArea.Width >= 0) && (aVisArea.Height >= 0), "corrupted visible area for sd document! [CL]" );
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
        case WID_MODEL_RUNTIMEUID:
            aAny <<= getRuntimeUID();
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xLinks( mxLinks );
    if( !xLinks.is() )
        mxLinks = xLinks = new SdDocLinkTargets( *this );
    return xLinks;
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getStyleFamilies(  )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xStyles(mxStyleFamilies);

    if( !xStyles.is() )
        mxStyleFamilies = xStyles = new SdUnoStyleFamilies( this );

    return xStyles;
}

// XAnyCompareFactory
uno::Reference< com::sun::star::ucb::XAnyCompare > SAL_CALL SdXImpressDocument::createAnyCompareByName( const OUString& PropertyName )
    throw (uno::RuntimeException)
{
    return SvxCreateNumRuleCompare();
}

// XRenderable
sal_Int32 SAL_CALL SdXImpressDocument::getRendererCount( const uno::Any& rSelection,
                                                         const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard      aGuard( Application::GetSolarMutex() );
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
    OGuard aGuard( Application::GetSolarMutex() );

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

class ImplRenderPaintProc : public ::sdr::contact::ViewObjectContactRedirector
{
    const SdrLayerAdmin& rLayerAdmin;
    SdrPageView* pSdrPageView;

public:
    sal_Bool IsVisible  ( const SdrObject* pObj ) const;
    sal_Bool IsPrintable( const SdrObject* pObj ) const;

    ImplRenderPaintProc( const SdrLayerAdmin& rLA, SdrPageView* pView );
    virtual ~ImplRenderPaintProc();

    // all default implementations just call the same methods at the original. To do something
    // different, overload the method and at least do what the method does.
    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);
};

ImplRenderPaintProc::ImplRenderPaintProc( const SdrLayerAdmin& rLA, SdrPageView* pView )
:   ViewObjectContactRedirector(),
    rLayerAdmin     ( rLA ),
    pSdrPageView    ( pView )
{
}

ImplRenderPaintProc::~ImplRenderPaintProc()
{
}

// all default implementations just call the same methods at the original. To do something
// different, overload the method and at least do what the method does.
void ImplRenderPaintProc::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        if(pObject->GetPage())
        {
            if(pObject->GetPage()->checkVisibility(rOriginal, rDisplayInfo, false))
            {
                if(IsVisible(pObject) && IsPrintable(pObject))
                {
                    rOriginal.PaintObject(rDisplayInfo);
                }
            }
        }
    }
    else
    {
        // not an object, maybe a page
        rOriginal.PaintObject(rDisplayInfo);
    }
}

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
void SAL_CALL SdXImpressDocument::render( sal_Int32 nRenderer, const uno::Any& rSelection,
                                          const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    if( pDocShell && pDoc )
    {
        uno::Reference< awt::XDevice >  xRenderDevice;
        const sal_Int32                 nPageNumber = nRenderer + 1;

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
                ::sd::ClientView* pView = new ::sd::ClientView( pDocShell, pOut, NULL );
                Rectangle               aVisArea( pDocShell->GetVisArea( ASPECT_DOCPRINT ) );
                Region                  aRegion( aVisArea );
                Point                   aOrigin;

                ::sd::ViewShell* pOldViewSh = pDocShell->GetViewShell();
                ::sd::View* pOldSdView = pOldViewSh ? pOldViewSh->GetView() : NULL;

                if  ( pOldSdView )
                    pOldSdView->EndTextEdit();

                ImplRenderPaintProc aImplRenderPaintProc( pDoc->GetLayerAdmin(),
                    pOldSdView ? pOldSdView->GetPageViewPvNum( 0 ) : NULL );

                pView->SetHlplVisible( sal_False );
                pView->SetGridVisible( sal_False );
                pView->SetBordVisible( sal_False );
                pView->SetPageVisible( sal_False );
                pView->SetGlueVisible( sal_False );

                pOut->SetMapMode( MAP_100TH_MM );
                pOut->IntersectClipRegion( aVisArea );

                uno::Reference< frame::XModel > xModel;
                rSelection >>= xModel;

                if( xModel == pDocShell->GetModel() )
                {
                    pView->ShowPage( pDoc->GetSdPage( (USHORT)nPageNumber - 1, PK_STANDARD ), aOrigin );
                    SdrPageView* pPV = pView->GetPageViewPvNum( 0 );

                    // background color for outliner :o
                    SdPage* pPage = (SdPage*)pPV->GetPage();
                    if( pPage )
                    {
                        SdrOutliner& rOutl = pDoc->GetDrawOutliner( NULL );
                        rOutl.SetBackgroundColor( pPage->GetBackgroundColor( pPV ) );
                    }
                    pPV->InitRedraw( pOut, aRegion, 0, &aRenderPaintProc );
                }
                else
                {
                    uno::Reference< drawing::XShapes > xShapes;
                    rSelection >>= xShapes;

                    if( xShapes.is() && xShapes->getCount() )
                    {
                       SdrPageView* pPV = NULL;

                        for( sal_uInt32 i = 0, nCount = xShapes->getCount(); i < nCount; i++ )
                        {
                            uno::Reference< drawing::XShape > xShape;
                            xShapes->getByIndex( i ) >>= xShape;

                            if( xShape.is() )
                            {
                                SvxShape* pShape = SvxShape::getImplementation( xShape );

                                if( pShape )
                                {
                                    SdrObject* pObj = pShape->GetSdrObject();
                                    if( pObj && pObj->GetPage()
                                        && aImplRenderPaintProc.IsVisible( pObj )
                                            && aImplRenderPaintProc.IsPrintable( pObj ) )
                                    {
                                        if( !pPV )
                                            pPV = pView->ShowPage( pObj->GetPage(), aOrigin );

                                        if( pPV )
                                            pView->MarkObj( pObj, pPV );
                                    }
                                }
                            }
                        }

                        pView->DrawAllMarked( *pOut, aOrigin );
                    }
                }

                delete pView;
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
        pDoc->StopWorkStartupDelay();
    }
}

void SAL_CALL SdXImpressDocument::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    if( !mbDisposed )
    {
        {
            OGuard aGuard( Application::GetSolarMutex() );
            mbDisposed = true;

            if( pDoc )
            {
                EndListening( *pDoc );
                pDoc = NULL;
            }

            uno::Reference< container::XNameAccess > xStyles(mxStyleFamilies);
            if( xStyles.is() )
            {
                uno::Reference< lang::XComponent > xComp( xStyles, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xStyles = 0;
            }

            uno::Reference< presentation::XPresentation > xPresentation( mxPresentation );
            if( xPresentation.is() )
            {
                uno::Reference< lang::XComponent > xComp( xPresentation, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xPresentation = 0;
            }

            uno::Reference< container::XNameAccess > xLinks( mxLinks );
            if( xLinks.is() )
            {
                uno::Reference< lang::XComponent > xComp( xLinks, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xLinks = 0;
            }

            uno::Reference< drawing::XDrawPages > xDrawPagesAccess( mxDrawPagesAccess );
            if( xDrawPagesAccess.is() )
            {
                uno::Reference< lang::XComponent > xComp( xDrawPagesAccess, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xDrawPagesAccess = 0;
            }

            uno::Reference< drawing::XDrawPages > xMasterPagesAccess( mxMasterPagesAccess );
            if( xDrawPagesAccess.is() )
            {
                uno::Reference< lang::XComponent > xComp( xMasterPagesAccess, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xDrawPagesAccess = 0;
            }

            uno::Reference< container::XNameAccess > xLayerManager( mxLayerManager );
            if( xLayerManager.is() )
            {
                uno::Reference< lang::XComponent > xComp( xLayerManager, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xLayerManager = 0;
            }

            uno::Reference< container::XNameContainer > xCustomPresentationAccess( mxCustomPresentationAccess );
            if( xCustomPresentationAccess.is() )
            {
                uno::Reference< lang::XComponent > xComp( xCustomPresentationAccess, uno::UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();

                xCustomPresentationAccess = 0;
            }

            mxDashTable = 0;
            mxGradientTable = 0;
            mxHatchTable = 0;
            mxBitmapTable = 0;
            mxTransGradientTable = 0;
            mxMarkerTable = 0;
            mxDrawingPool = 0;

        }

        SfxBaseModel::dispose();
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL SdXImpressDocument::setPrinter( const uno::Sequence< beans::PropertyValue >& rPrinter)
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == pDoc )
        throw lang::DisposedException();

    SfxViewShell* pViewSh = NULL;
    SfxPrinter* pPrinter = NULL;
    sal_uInt16 nChangeFlags = 0;
    impl_setPrinter(rPrinter,pPrinter,nChangeFlags,pViewSh);
    ::sd::ViewShell* pSdViewSh = PTR_CAST(::sd::ViewShell,pViewSh);
    // set new printer
    if ( pSdViewSh && pPrinter )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        //do not show the dialog here
        pSdViewSh->GetViewShellBase().SetPrinterOptDlg (
            pPrinter,
            nChangeFlags,
            FALSE );
    }
}

//=============================================================================
// class SdDrawPagesAccess
//=============================================================================

SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  throw()
:   mpModel( &rMyModel)
{
}

SdDrawPagesAccess::~SdDrawPagesAccess() throw()
{
}

// XIndexAccess
sal_Int32 SAL_CALL SdDrawPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    return mpModel->pDoc->GetSdPageCount( PK_STANDARD );
}

uno::Any SAL_CALL SdDrawPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->pDoc->GetSdPageCount( PK_STANDARD ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->pDoc->GetSdPage( (sal_uInt16)Index, PK_STANDARD );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    if( aName.getLength() != 0 )
    {
        const sal_uInt16 nCount = mpModel->pDoc->GetSdPageCount( PK_STANDARD );
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nCount; nPage++ )
        {
            SdPage* pPage = mpModel->pDoc->GetSdPage( nPage, PK_STANDARD );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->pDoc->GetSdPageCount( PK_STANDARD );
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();

    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->pDoc->GetSdPage( nPage, PK_STANDARD );
        *pNames++ = SdDrawPage::getPageApiName( pPage );
    }

    return aNames;
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasByName( const OUString& aName ) throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->pDoc->GetSdPageCount( PK_STANDARD );
    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->pDoc->GetSdPage( nPage, PK_STANDARD );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    if( mpModel->pDoc )
    {
        SdPage* pPage = mpModel->InsertSdPage( (sal_uInt16)nIndex );
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
* nach dem entfernen dieser Seite vorhanden ist.                              *
******************************************************************************/
void SAL_CALL SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
        throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    sal_uInt16 nPageCount = mpModel->pDoc->GetSdPageCount( PK_STANDARD );
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
                    mpModel->pDoc->RemovePage( nPage );

                    // Die darauffolgende Seite ist die dazugeoerige Notizseite
                    mpModel->pDoc->RemovePage( nPage );
                }
            }
            pSvxPage->Invalidate();
        }
    }

    mpModel->SetModified();
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

// XComponent
void SAL_CALL SdDrawPagesAccess::dispose(  ) throw (uno::RuntimeException)
{
    mpModel = NULL;
}

void SAL_CALL SdDrawPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

void SAL_CALL SdDrawPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >& aListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

//=============================================================================
// class SdMasterPagesAccess
//=============================================================================

SdMasterPagesAccess::SdMasterPagesAccess( SdXImpressDocument& rMyModel ) throw()
:   mpModel(&rMyModel)
{
}

SdMasterPagesAccess::~SdMasterPagesAccess() throw()
{
}

// XComponent
void SAL_CALL SdMasterPagesAccess::dispose(  ) throw (uno::RuntimeException)
{
    mpModel = NULL;
}

void SAL_CALL SdMasterPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

void SAL_CALL SdMasterPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >& aListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel->pDoc )
        throw lang::DisposedException();

    return mpModel->pDoc->GetMasterSdPageCount(PK_STANDARD);
}

/******************************************************************************
* Liefert ein drawing::XDrawPage Interface fuer den Zugriff auf die Masterpage and der *
* angegebennen Position im Model.                                             *
******************************************************************************/
uno::Any SAL_CALL SdMasterPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->pDoc->GetMasterSdPageCount( PK_STANDARD ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->pDoc->GetMasterSdPage( (sal_uInt16)Index, PK_STANDARD );
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xDrawPage;

    SdDrawDocument* pDoc = mpModel->pDoc;
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
        SdPage* pPage = mpModel->pDoc->GetSdPage( (sal_uInt16)0, PK_STANDARD );
        SdPage* pRefNotesPage = mpModel->pDoc->GetSdPage( (sal_uInt16)0, PK_NOTES);

        // create and instert new draw masterpage
        SdPage* pMPage = (SdPage*)mpModel->pDoc->AllocPage(sal_True);
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
        SdPage* pMNotesPage = (SdPage*)mpModel->pDoc->AllocPage(sal_True);
        pMNotesPage->SetSize( pRefNotesPage->GetSize() );
        pMNotesPage->SetPageKind(PK_NOTES);
        pMNotesPage->SetBorder( pRefNotesPage->GetLftBorder(),
                                pRefNotesPage->GetUppBorder(),
                                pRefNotesPage->GetRgtBorder(),
                                pRefNotesPage->GetLwrBorder() );
        pDoc->InsertMasterPage(pMNotesPage,  (USHORT)nInsertPos + 1);
//      pMNotesPage->InsertMasterPage( pMPage->GetPageNum() );
        pMNotesPage->SetLayoutName( aLayoutName );
        pMNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, sal_True, sal_True);
        mpModel->SetModified();
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    SdMasterPage* pSdPage = SdMasterPage::getImplementation( xPage );
    if(pSdPage == NULL)
        return;

    SdrPage* pSdrPage = pSdPage->GetSdrPage();

    DBG_ASSERT( pSdrPage->IsMasterPage(), "SdMasterPage is not masterpage?")

    if(mpModel->pDoc->GetMasterPageUserCount(pSdrPage) > 0)
        return; //Todo: hier fehlt eine uno::Exception

    sal_uInt16 nCount = mpModel->pDoc->GetMasterPageCount();
    for( sal_uInt16 nPgNum = 0; nPgNum < nCount; nPgNum++ )
    {
        if(mpModel->pDoc->GetMasterPage(nPgNum) == pSdrPage)
        {
            mpModel->pDoc->DeleteMasterPage(nPgNum);
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
: mpModel( &rMyModel )
{
}

SdDocLinkTargets::~SdDocLinkTargets() throw()
{
}

// XComponent
void SAL_CALL SdDocLinkTargets::dispose(  ) throw (uno::RuntimeException)
{
    mpModel = NULL;
}

void SAL_CALL SdDocLinkTargets::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

void SAL_CALL SdDocLinkTargets::removeEventListener( const uno::Reference< lang::XEventListener >& aListener ) throw (uno::RuntimeException)
{
    DBG_ERROR( "not implemented!" );
}

// XNameAccess
uno::Any SAL_CALL SdDocLinkTargets::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    SdDrawDocument* pDoc = mpModel->GetDoc();
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
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
    OGuard aGuard( Application::GetSolarMutex() );

    if( NULL == mpModel )
        throw lang::DisposedException();

    return mpModel->GetDoc() != NULL;
}

SdPage* SdDocLinkTargets::FindPage( const OUString& rName ) const throw()
{
    SdDrawDocument* pDoc = mpModel->GetDoc();
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

