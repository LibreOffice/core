/*************************************************************************
 *
 *  $RCSfile: unomodel.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: cl $ $Date: 2001-04-06 14:08:56 $
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

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
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

#ifndef SVX_LIGHT
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#include <svx/UnoForbiddenCharsTable.hxx>
#endif

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

// folgende fuer InsertSdPage()
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

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#include <svtools/unoimap.hxx>

#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#include <svx/unoshape.hxx>
#include <svx/unonrule.hxx>

#ifndef SVX_LIGHT
#include <docshell.hxx>
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

#include "viewshel.hxx"
#include "app.hrc"

using namespace ::osl;
using namespace ::rtl;
using namespace ::vos;
using namespace ::cppu;
using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////

class SdUnoForbiddenCharsTable : public SvxUnoForbiddenCharsTable,
                                 public SfxListener
{
private:
    SdrModel*   mpModel;

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

    if( pSdrHint && HINT_MODELCLEARED == pSdrHint->GetKind() )
        mpModel = NULL;
}

///////////////////////////////////////////////////////////////////////

const sal_Int32 WID_MODEL_LANGUAGE = 1;
const sal_Int32 WID_MODEL_TABSTOP  = 2;
const sal_Int32 WID_MODEL_VISAREA  = 3;
const sal_Int32 WID_MODEL_MAPUNIT  = 4;
const sal_Int32 WID_MODEL_FORBCHARS= 5;
const sal_Int32 WID_MODEL_CONTFOCUS = 6;
const sal_Int32 WID_MODEL_DSGNMODE  = 7;

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
        { 0,0,0,0,0 }
    };

    return aDrawModelPropertyMap_Impl;
}

// this ctor is used from the DocShell
SdXImpressDocument::SdXImpressDocument( SdDrawDocShell* pShell ) throw()
:   SfxBaseModel( pShell ),
    pDocShell( pShell ),
    aPropSet( ImplGetDrawModelPropertyMap() )
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
    if( pDoc )
        EndListening( *pDoc );
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
    else QUERYINT(document::XLinkTargetSupplier);
    else QUERYINT(style::XStyleFamiliesSupplier);
    else QUERYINT(lang::XUnoTunnel);
    else QUERYINT(ucb::XAnyCompareFactory);
    else if( mbImpressDoc && rType == ITYPE(presentation::XPresentationSupplier) )
            aAny <<= uno::Reference< presentation::XPresentationSupplier >(this);
    else if( mbImpressDoc && rType == ITYPE(presentation::XCustomPresentationSupplier) )
            aAny <<= uno::Reference< presentation::XCustomPresentationSupplier >(this);
    else
        return SfxBaseModel::queryInterface( rType );

    return aAny;
}

void SAL_CALL SdXImpressDocument::acquire() throw(uno::RuntimeException)
{
    SfxBaseModel::acquire();
}

void SAL_CALL SdXImpressDocument::release() throw(uno::RuntimeException)
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

        const sal_Int32 nOwnTypes = mbImpressDoc ? 13 : 11;     // !DANGER! Keep this updated!

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
        *pTypes++ = ITYPE(ucb::XAnyCompareFactory);
        if( mbImpressDoc )
        {
            *pTypes++ = ITYPE(presentation::XPresentationSupplier);
            *pTypes++ = ITYPE(presentation::XCustomPresentationSupplier);
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
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        if( pSdrHint->GetKind() == HINT_MODELCLEARED )
        {
            pDoc = NULL;
            pDocShell = NULL;
        }
    }
    else
    {
#ifndef SVX_LIGHT
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
#endif
    }
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

    // Hier wird die Seite ermittelt, hinter der eingefuegt werden soll
    SdPage* pPreviousStandardPage = pDoc->GetSdPage( Min( (sal_uInt16)(nPageCount - 1), nPage ), PK_STANDARD );

    sal_uInt16 nPos = 0;
    SetOfByte aVisibleLayers = pPreviousStandardPage->GetMasterPageVisibleLayers( nPos );
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
    AutoLayout eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    AutoLayout eNotesLayout = pPreviousNotesPage->GetAutoLayout();

    /**************************************************************
    * Standardseite
    **************************************************************/
    SdPage* pStandardPage = NULL;

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
        pStandardPage->SetAutoLayout(eStandardLayout, sal_True);
    }

    aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), sal_False);
    aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), sal_False);
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
        pNotesPage->SetAutoLayout(eNotesLayout, sal_True);
    }

    SetModified();

    return( pStandardPage );
}

void SdXImpressDocument::SetModified( sal_Bool bModified /* = sal_True */ ) throw()
{
    if( pDoc )
        pDoc->SetChanged( bModified );
}

void SAL_CALL SdXImpressDocument::lockControllers(  )
    throw(uno::RuntimeException)
{
    if( pDoc )
        pDoc->setLock( sal_True );
}

void SAL_CALL SdXImpressDocument::unlockControllers(  )
    throw(uno::RuntimeException)
{
    if( pDoc && pDoc->isLocked() )
    {
        pDoc->setLock( sal_False );
        pDoc->ReformatAllTextObjects();
    }
}

sal_Bool SAL_CALL SdXImpressDocument::hasControllersLocked(  )
    throw(uno::RuntimeException)
{
    return pDoc && pDoc->isLocked();
}

// XDrawPageDuplicator
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::duplicate( const uno::Reference< drawing::XDrawPage >& xPage )
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    if( pDoc )
    {
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
    }

    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}


// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getDrawPages()
    throw(uno::RuntimeException)
{
    uno::Reference< drawing::XDrawPages >  xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
        mxDrawPagesAccess = xDrawPages = (drawing::XDrawPages*)new SdDrawPagesAccess(*this);

    return xDrawPages;
}

// XMasterPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getMasterPages()
    throw(uno::RuntimeException)
{
    uno::Reference< drawing::XDrawPages >  xMasterPages( mxMasterPagesAccess );

    if( !xMasterPages.is() )
        mxMasterPagesAccess = xMasterPages = new SdMasterPagesAccess(*this);

    return xMasterPages;
}

// XLayerManagerSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLayerManager(  )
    throw(uno::RuntimeException)
{
    uno::Reference< container::XNameAccess >  xLayerManager( mxLayerManager );

    if( !xLayerManager.is() )
        mxLayerManager = xLayerManager = new SdLayerManager(*this);

    return xLayerManager;
}

// XCustomPresentationSupplier
uno::Reference< container::XNameContainer > SAL_CALL SdXImpressDocument::getCustomPresentations()
    throw(uno::RuntimeException)
{
    uno::Reference< container::XNameContainer >  xCustomPres( mxCustomPresentationAccess );

    if( !xCustomPres.is() )
        mxCustomPresentationAccess = xCustomPres = new SdXCustomPresentationAccess(*this);

    return xCustomPres;
}

// XPresentationSupplier
uno::Reference< presentation::XPresentation > SAL_CALL SdXImpressDocument::getPresentation()
    throw(uno::RuntimeException)
{
    uno::Reference< presentation::XPresentation >  aPresentation( mxPresentation );

    if( !aPresentation.is() )
        mxPresentation = aPresentation = new SdXPresentation(*this);

    return aPresentation;
}

// XMultiServiceFactory ( SvxFmMSFactory )
uno::Reference< uno::XInterface > SAL_CALL SdXImpressDocument::createInstance( const OUString& aServiceSpecifier )
    throw(uno::Exception, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
            mxDrawingPool = (uno::XAggregation*)new SvxUnoDrawPool( pDoc );

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

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.document.Settings") ) )
    {
        return sd::DocumentSettings_createInstance( this );
    }

    if( 0 == aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.text.TextField.DateTime") ) )
    {
        return (::cppu::OWeakObject * )new SvxUnoTextField( ID_EXT_DATEFIELD );
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
            nType = PRESOBJ_NOTES;
        }
        else if( aType.EqualsAscii( "HandoutShape", 26, 13 ) )
        {
            nType = PRESOBJ_HANDOUT;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor );

        // set shape type
        if( pShape )
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
        new SdXShape( xShape, (SdXImpressDocument*)this );
        xRet = xShape;
        xShape.clear();
    }

    return xRet;
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getAvailableServiceNames()
    throw(uno::RuntimeException)
{
    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( mbImpressDoc ? 24 : 13 );

    sal_uInt16 i = 0;

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
    OUString aSN( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"));
    uno::Sequence< OUString > aSeq( &aSN, 1 );

    if( mbImpressDoc )
        SvxServiceInfoHelper::addToSequence( aSeq, 1, "com.sun.star.presentation.PresentationDocument");

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

    if(pDoc==NULL)
        throw beans::UnknownPropertyException();

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
#ifndef SVX_LIGHT
            {
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                awt::Rectangle aVisArea;
                if( !(aValue >>= aVisArea) )
                    throw lang::IllegalArgumentException();

                pEmbeddedObj->SetVisArea( Rectangle( aVisArea.X, aVisArea.Y, aVisArea.X + aVisArea.Width - 1, aVisArea.Y + aVisArea.Height - 1 ) );
            }
#endif
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
    if( pDoc == NULL )
        throw beans::UnknownPropertyException();

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
#ifndef SVX_LIGHT
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                const Rectangle& aRect = pEmbeddedObj->GetVisArea();
                awt::Rectangle aVisArea( aRect.nLeft, aRect.nTop, aRect.getWidth(), aRect.getHeight() );

                aAny <<= aVisArea;
#endif
            }
            break;
        case WID_MODEL_MAPUNIT:
            {
#ifndef SVX_LIGHT
                SvEmbeddedObject* pEmbeddedObj = pDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                sal_Int16 nMeasureUnit = 0;
                SvxMapUnitToMeasureUnit( pEmbeddedObj->GetMapUnit(), nMeasureUnit );
                aAny <<= (sal_Int16)nMeasureUnit;
#endif
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
    return new SdDocLinkTargets( *this );
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getStyleFamilies(  )
    throw(uno::RuntimeException)
{
    uno::Reference< container::XNameAccess > xStyles(mxStyleFamilies);

    if( !xStyles.is() )
        mxStyleFamilies = xStyles = new SdUnoStyleFamilies( this );

    return xStyles;
}

// XAnyCompareFactory
uno::Reference< ucb::XAnyCompare > SAL_CALL SdXImpressDocument::createAnyCompareByName( const OUString& PropertyName )
    throw(uno::RuntimeException)
{
    return SvxCreateNumRuleCompare();
}

uno::Reference< i18n::XForbiddenCharacters > SdXImpressDocument::getForbiddenCharsTable()
{
    uno::Reference< i18n::XForbiddenCharacters > xForb(mxForbidenCharacters);

    if( !xForb.is() )
        mxForbidenCharacters = xForb = new SdUnoForbiddenCharsTable( pDoc );

    return xForb;
}

//=============================================================================
// class SdDrawPagesAccess
//=============================================================================

SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  throw()
:   rModel(rMyModel)
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

    sal_Int32 nCount = 0;

    if( rModel.pDoc )
        nCount = rModel.pDoc->GetSdPageCount( PK_STANDARD );

    return( nCount );
}

uno::Any SAL_CALL SdDrawPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    if( rModel.pDoc )
    {
        if( (Index < 0) || (Index >= rModel.pDoc->GetSdPageCount( PK_STANDARD ) ) )
            throw lang::IndexOutOfBoundsException();

        SdPage* pPage = rModel.pDoc->GetSdPage( (sal_uInt16)Index, PK_STANDARD );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            aAny <<= xDrawPage;
        }
    }
    return aAny;
}

// XNameAccess
uno::Any SAL_CALL SdDrawPagesAccess::getByName( const OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
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
    OGuard aGuard( Application::GetSolarMutex() );


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
* nach dem entfernen dieser Seite vorhanden ist.                              *
******************************************************************************/
void SAL_CALL SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
        throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
:   rModel(rMyModel)
{
}

SdMasterPagesAccess::~SdMasterPagesAccess() throw()
{
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPagesAccess::getCount()
    throw(uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    sal_Int32 nCount = 0;

    if( rModel.pDoc )
        nCount = rModel.pDoc->GetMasterSdPageCount(PK_STANDARD);

    return( nCount );
}

/******************************************************************************
* Liefert ein drawing::XDrawPage Interface fuer den Zugriff auf die Masterpage and der *
* angegebennen Position im Model.                                             *
******************************************************************************/
uno::Any SAL_CALL SdMasterPagesAccess::getByIndex( sal_Int32 Index )
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

    uno::Any aAny;

    if( rModel.pDoc )
    {
        if( (Index < 0) || (Index >= rModel.pDoc->GetMasterSdPageCount( PK_STANDARD ) ) )
            throw lang::IndexOutOfBoundsException();

        SdPage* pPage = rModel.pDoc->GetMasterSdPage( (sal_uInt16)Index, PK_STANDARD );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            aAny <<= xDrawPage;
        }
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
        aLayoutName += String( RTL_CONSTASCII_USTRINGPARAM("~LT~Outline1") );

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
        pMNotesPage->InsertMasterPage( pMPage->GetPageNum() );
        pMNotesPage->SetLayoutName( aLayoutName );
        pMNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, TRUE);
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
    OGuard aGuard( Application::GetSolarMutex() );

    SdMasterPage* pSdPage = SdMasterPage::getImplementation( xPage );
    if(pSdPage == NULL)
        return;

    SdrPage* pSdrPage = pSdPage->GetSdrPage();

    DBG_ASSERT( pSdrPage->IsMasterPage(), "SdMasterPage is not masterpage?")

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
:   rModel( rMyModel )
{
}

SdDocLinkTargets::~SdDocLinkTargets() throw()
{
}

// XNameAccess
uno::Any SAL_CALL SdDocLinkTargets::getByName( const OUString& aName )
    throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    OGuard aGuard( Application::GetSolarMutex() );

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
    OGuard aGuard( Application::GetSolarMutex() );

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

#ifdef SVX_LIGHT
uno::Reference< frame::XModel > SdDrawDocShell::GetModel()
{
    if( !mxModel.is() )
        mxModel = new SdXImpressDocument( this );

    return mxModel;
}
#endif

