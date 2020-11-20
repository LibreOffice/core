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

#include <memory>
#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/presentation/XPresentation2.hpp>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/embed/Aspects.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>

#include <sal/log.hxx>
#include <editeng/unofield.hxx>
#include <notifydocumentevent.hxx>
#include <unomodel.hxx>
#include "unopool.hxx"
#include <sfx2/lokhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <editeng/UnoForbiddenCharsTable.hxx>
#include <svx/svdoutl.hxx>
#include <o3tl/safeint.hxx>
#include <svx/UnoNamespaceMap.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdundo.hxx>
#include <svx/unoapi.hxx>
#include <svx/unofill.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/unoimap.hxx>
#include <svtools/slidesorterbaropt.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unonrule.hxx>
#include <editeng/eeitem.hxx>
#include <unotools/datetime.hxx>
#include <unotools/saveopt.hxx>
#include <xmloff/autolayout.hxx>

// Support creation of GraphicStorageHandler and EmbeddedObjectResolver
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include "UnoDocumentSettings.hxx"

#include <Annotation.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>

#include <strings.hrc>
#include <strings.hxx>
#include "unolayer.hxx"
#include <unopage.hxx>
#include "unocpres.hxx"
#include "unoobj.hxx"
#include <stlpool.hxx>
#include "unopback.hxx"
#include <unokywds.hxx>

#include <FrameView.hxx>
#include <ClientView.hxx>
#include <DrawViewShell.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <optsitem.hxx>

#include <vcl/pdfextoutdevdata.hxx>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>

#include <sfx2/lokcharthelper.hxx>
#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/json_writer.hxx>
#include <tools/UnitConversion.hxx>

#define TWIPS_PER_PIXEL 15

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::sd;

namespace {

class SdUnoForbiddenCharsTable : public SvxUnoForbiddenCharsTable,
                                 public SfxListener
{
public:
    explicit SdUnoForbiddenCharsTable(SdrModel* pModel);
    virtual ~SdUnoForbiddenCharsTable() override;

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw () override;
protected:
    virtual void onChange() override;

private:
    SdrModel*   mpModel;
};

}

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
    SolarMutexGuard g;

    if( mpModel )
        EndListening( *mpModel );
}

void SdUnoForbiddenCharsTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
    if( SdrHintKind::ModelCleared == pSdrHint->GetKind() )
    {
        mpModel = nullptr;
    }
}

const sal_uInt16 WID_MODEL_LANGUAGE           =  1;
const sal_uInt16 WID_MODEL_TABSTOP            =  2;
const sal_uInt16 WID_MODEL_VISAREA            =  3;
const sal_uInt16 WID_MODEL_MAPUNIT            =  4;
const sal_uInt16 WID_MODEL_FORBCHARS          =  5;
const sal_uInt16 WID_MODEL_CONTFOCUS          =  6;
const sal_uInt16 WID_MODEL_DSGNMODE           =  7;
const sal_uInt16 WID_MODEL_BASICLIBS          =  8;
const sal_uInt16 WID_MODEL_RUNTIMEUID         =  9;
const sal_uInt16 WID_MODEL_BUILDID            = 10;
const sal_uInt16 WID_MODEL_HASVALIDSIGNATURES = 11;
const sal_uInt16 WID_MODEL_DIALOGLIBS         = 12;
const sal_uInt16 WID_MODEL_FONTS              = 13;
const sal_uInt16 WID_MODEL_INTEROPGRABBAG     = 14;

static const SvxItemPropertySet* ImplGetDrawModelPropertySet()
{
    // Attention: the first parameter HAS TO BE sorted!!!
    const static SfxItemPropertyMapEntry aDrawModelPropertyMap_Impl[] =
    {
        { u"BuildId",                      WID_MODEL_BUILDID,            ::cppu::UnoType<OUString>::get(),                      0, 0},
        { sUNO_Prop_CharLocale,           WID_MODEL_LANGUAGE,           ::cppu::UnoType<lang::Locale>::get(),                                  0, 0},
        { sUNO_Prop_TabStop,              WID_MODEL_TABSTOP,            ::cppu::UnoType<sal_Int32>::get(),                                     0, 0},
        { sUNO_Prop_VisibleArea,          WID_MODEL_VISAREA,            ::cppu::UnoType<awt::Rectangle>::get(),                                0, 0},
        { sUNO_Prop_MapUnit,              WID_MODEL_MAPUNIT,            ::cppu::UnoType<sal_Int16>::get(),                                     beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_ForbiddenCharacters,  WID_MODEL_FORBCHARS,          cppu::UnoType<i18n::XForbiddenCharacters>::get(), beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_AutomContFocus,       WID_MODEL_CONTFOCUS,          cppu::UnoType<bool>::get(),                                                 0, 0},
        { sUNO_Prop_ApplyFrmDsgnMode,     WID_MODEL_DSGNMODE,           cppu::UnoType<bool>::get(),                                                 0, 0},
        { u"BasicLibraries",               WID_MODEL_BASICLIBS,          cppu::UnoType<script::XLibraryContainer>::get(),  beans::PropertyAttribute::READONLY, 0},
        { u"DialogLibraries",              WID_MODEL_DIALOGLIBS,         cppu::UnoType<script::XLibraryContainer>::get(),  beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_RuntimeUID,           WID_MODEL_RUNTIMEUID,         ::cppu::UnoType<OUString>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_HasValidSignatures,   WID_MODEL_HASVALIDSIGNATURES, ::cppu::UnoType<sal_Bool>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { u"Fonts",                        WID_MODEL_FONTS,              cppu::UnoType<uno::Sequence<uno::Any>>::get(),                     beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_InteropGrabBag,       WID_MODEL_INTEROPGRABBAG,     cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(),       0, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aDrawModelPropertySet_Impl( aDrawModelPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aDrawModelPropertySet_Impl;
}

// this ctor is used from the DocShell
SdXImpressDocument::SdXImpressDocument(::sd::DrawDocShell* pShell, bool bClipBoard)
:   SfxBaseModel( pShell ),
    mpDocShell( pShell ),
    mpDoc( pShell ? pShell->GetDoc() : nullptr ),
    mbDisposed(false),
    mbImpressDoc( pShell && pShell->GetDoc() && pShell->GetDoc()->GetDocumentType() == DocumentType::Impress ),
    mbClipBoard( bClipBoard ),
    mpPropSet( ImplGetDrawModelPropertySet() )
{
    if( mpDoc )
    {
        StartListening( *mpDoc );
    }
    else
    {
        OSL_FAIL("DocShell is invalid");
    }
}

SdXImpressDocument::SdXImpressDocument(SdDrawDocument* pDoc, bool bClipBoard)
:   SfxBaseModel( nullptr ),
    mpDocShell( nullptr ),
    mpDoc( pDoc ),
    mbDisposed(false),
    mbImpressDoc( pDoc && pDoc->GetDocumentType() == DocumentType::Impress ),
    mbClipBoard( bClipBoard ),
    mpPropSet( ImplGetDrawModelPropertySet() )
{
    if( mpDoc )
    {
        StartListening( *mpDoc );
    }
    else
    {
        OSL_FAIL("SdDrawDocument is invalid");
    }
}

/***********************************************************************
*                                                                      *
***********************************************************************/
SdXImpressDocument::~SdXImpressDocument() throw()
{
}

// XInterface
uno::Any SAL_CALL SdXImpressDocument::queryInterface( const uno::Type & rType )
{
    uno::Any aAny;

    if (rType == cppu::UnoType<lang::XServiceInfo>::get())
        aAny <<= uno::Reference<lang::XServiceInfo>(this);
    else if (rType == cppu::UnoType<beans::XPropertySet>::get())
        aAny <<= uno::Reference<beans::XPropertySet>(this);
    else if (rType == cppu::UnoType<lang::XMultiServiceFactory>::get())
        aAny <<= uno::Reference<lang::XMultiServiceFactory>(this);
    else if (rType == cppu::UnoType<drawing::XDrawPageDuplicator>::get())
        aAny <<= uno::Reference<drawing::XDrawPageDuplicator>(this);
    else if (rType == cppu::UnoType<drawing::XLayerSupplier>::get())
        aAny <<= uno::Reference<drawing::XLayerSupplier>(this);
    else if (rType == cppu::UnoType<drawing::XMasterPagesSupplier>::get())
        aAny <<= uno::Reference<drawing::XMasterPagesSupplier>(this);
    else if (rType == cppu::UnoType<drawing::XDrawPagesSupplier>::get())
        aAny <<= uno::Reference<drawing::XDrawPagesSupplier>(this);
    else if (rType == cppu::UnoType<presentation::XHandoutMasterSupplier>::get())
        aAny <<= uno::Reference<presentation::XHandoutMasterSupplier>(this);
    else if (rType == cppu::UnoType<document::XLinkTargetSupplier>::get())
        aAny <<= uno::Reference<document::XLinkTargetSupplier>(this);
    else if (rType == cppu::UnoType<style::XStyleFamiliesSupplier>::get())
        aAny <<= uno::Reference<style::XStyleFamiliesSupplier>(this);
    else if (rType == cppu::UnoType<css::ucb::XAnyCompareFactory>::get())
        aAny <<= uno::Reference<css::ucb::XAnyCompareFactory>(this);
    else if (rType == cppu::UnoType<view::XRenderable>::get())
        aAny <<= uno::Reference<view::XRenderable>(this);
    else if (mbImpressDoc && rType == cppu::UnoType<presentation::XPresentationSupplier>::get())
        aAny <<= uno::Reference< presentation::XPresentationSupplier >(this);
    else if (mbImpressDoc && rType == cppu::UnoType<presentation::XCustomPresentationSupplier>::get())
        aAny <<= uno::Reference< presentation::XCustomPresentationSupplier >(this);
    else
        return SfxBaseModel::queryInterface(rType);

    return aAny;
}

void SAL_CALL SdXImpressDocument::acquire() throw ( )
{
    SfxBaseModel::acquire();
}

void SAL_CALL SdXImpressDocument::release() throw ( )
{
    if (osl_atomic_decrement( &m_refCount ) != 0)
        return;

    // restore reference count:
    osl_atomic_increment( &m_refCount );
    if(!mbDisposed)
    {
        try
        {
            dispose();
        }
        catch (const uno::RuntimeException&)
        {
            // don't break throw ()
            TOOLS_WARN_EXCEPTION( "sd", "" );
        }
    }
    SfxBaseModel::release();
}

namespace
{
    class theSdXImpressDocumentUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdXImpressDocumentUnoTunnelId> {};
}

// XUnoTunnel
const css::uno::Sequence< sal_Int8 > & SdXImpressDocument::getUnoTunnelId() throw()
{
    return theSdXImpressDocumentUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SdXImpressDocument::getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier )
{
    if( isUnoTunnelId<SdXImpressDocument>(rIdentifier) )
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));

    if( isUnoTunnelId<SdrModel>(rIdentifier) )
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(mpDoc));

    return SfxBaseModel::getSomething( rIdentifier );
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SdXImpressDocument::getTypes(  )
{
    ::SolarMutexGuard aGuard;

    if( !maTypeSequence.hasElements() )
    {
        uno::Sequence< uno::Type > aTypes( SfxBaseModel::getTypes() );
        aTypes = comphelper::concatSequences(aTypes,
            uno::Sequence {
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XMultiServiceFactory>::get(),
                cppu::UnoType<drawing::XDrawPageDuplicator>::get(),
                cppu::UnoType<drawing::XLayerSupplier>::get(),
                cppu::UnoType<drawing::XMasterPagesSupplier>::get(),
                cppu::UnoType<drawing::XDrawPagesSupplier>::get(),
                cppu::UnoType<document::XLinkTargetSupplier>::get(),
                cppu::UnoType<style::XStyleFamiliesSupplier>::get(),
                cppu::UnoType<css::ucb::XAnyCompareFactory>::get(),
                cppu::UnoType<view::XRenderable>::get() });
        if( mbImpressDoc )
        {
            aTypes = comphelper::concatSequences(aTypes,
                uno::Sequence {
                    cppu::UnoType<presentation::XPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XCustomPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XHandoutMasterSupplier>::get() });
        }
        maTypeSequence = aTypes;
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SdXImpressDocument::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

/***********************************************************************
*                                                                      *
***********************************************************************/
void SdXImpressDocument::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( mpDoc )
    {
        if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
        {
            const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
            if( hasEventListeners() )
            {
                document::EventObject aEvent;
                if( SvxUnoDrawMSFactory::createEvent( mpDoc, pSdrHint, aEvent ) )
                    notifyEvent( aEvent );
            }

            if( pSdrHint->GetKind() == SdrHintKind::ModelCleared )
            {
                if( mpDoc )
                    EndListening( *mpDoc );
                mpDoc = nullptr;
                mpDocShell = nullptr;
            }
        }
        else
        {
            // did our SdDrawDocument just died?
            if(rHint.GetId() == SfxHintId::Dying)
            {
                // yes, so we ask for a new one
                if( mpDocShell )
                {
                    SdDrawDocument *pNewDoc = mpDocShell->GetDoc();

                    // is there a new one?
                    if( pNewDoc != mpDoc )
                    {
                        mpDoc = pNewDoc;
                        if(mpDoc)
                            StartListening( *mpDoc );
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
SdPage* SdXImpressDocument::InsertSdPage( sal_uInt16 nPage, bool bDuplicate )
{
    sal_uInt16 nPageCount = mpDoc->GetSdPageCount( PageKind::Standard );
    SdrLayerAdmin& rLayerAdmin = mpDoc->GetLayerAdmin();
    SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
    SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);

    SdPage* pStandardPage = nullptr;

    if( 0 == nPageCount )
    {
        // this is only used for clipboard where we only have one page
        pStandardPage = mpDoc->AllocSdPage(false);

        Size aDefSize(21000, 29700);   // A4 portrait orientation
        pStandardPage->SetSize( aDefSize );
        mpDoc->InsertPage(pStandardPage, 0);
    }
    else
    {
        // here we determine the page after which we should insert
        SdPage* pPreviousStandardPage = mpDoc->GetSdPage( std::min( static_cast<sal_uInt16>(nPageCount - 1), nPage ), PageKind::Standard );
        SdrLayerIDSet aVisibleLayers = pPreviousStandardPage->TRG_GetMasterPageVisibleLayers();
        bool bIsPageBack = aVisibleLayers.IsSet( aBckgrnd );
        bool bIsPageObj = aVisibleLayers.IsSet( aBckgrndObj );

        // AutoLayouts must be ready
        mpDoc->StopWorkStartupDelay();

        /* First we create a standard page and then a notes page. It is
           guaranteed, that after a standard page the corresponding notes page
           follows. */

        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        SdPage* pPreviousNotesPage = static_cast<SdPage*>( mpDoc->GetPage( nStandardPageNum - 1 ) );
        sal_uInt16 nNotesPageNum = nStandardPageNum + 1;

        /**************************************************************
        * standard page
        **************************************************************/
        if( bDuplicate )
            pStandardPage = static_cast<SdPage*>( pPreviousStandardPage->CloneSdrPage(*mpDoc) );
        else
            pStandardPage = mpDoc->AllocSdPage(false);

        pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
        pStandardPage->SetBorder( pPreviousStandardPage->GetLeftBorder(),
                                    pPreviousStandardPage->GetUpperBorder(),
                                    pPreviousStandardPage->GetRightBorder(),
                                    pPreviousStandardPage->GetLowerBorder() );
        pStandardPage->SetOrientation( pPreviousStandardPage->GetOrientation() );
        pStandardPage->SetName(OUString());

        // insert page after current page
        mpDoc->InsertPage(pStandardPage, nStandardPageNum);

        if( !bDuplicate )
        {
            // use MasterPage of the current page
            pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());
            pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
            pStandardPage->SetAutoLayout(AUTOLAYOUT_NONE, true );
        }

        aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pStandardPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

        /**************************************************************
        * notes page
        **************************************************************/
        SdPage* pNotesPage = nullptr;

        if( bDuplicate )
            pNotesPage = static_cast<SdPage*>( pPreviousNotesPage->CloneSdrPage(*mpDoc) );
        else
            pNotesPage = mpDoc->AllocSdPage(false);

        pNotesPage->SetSize( pPreviousNotesPage->GetSize() );
        pNotesPage->SetBorder( pPreviousNotesPage->GetLeftBorder(),
                                pPreviousNotesPage->GetUpperBorder(),
                                pPreviousNotesPage->GetRightBorder(),
                                pPreviousNotesPage->GetLowerBorder() );
        pNotesPage->SetOrientation( pPreviousNotesPage->GetOrientation() );
        pNotesPage->SetName(OUString());
        pNotesPage->SetPageKind(PageKind::Notes);

        // insert page after current page
        mpDoc->InsertPage(pNotesPage, nNotesPageNum);

        if( !bDuplicate )
        {
            // use MasterPage of the current page
            pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());
            pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
            pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true );
        }
    }

    SetModified();

    return pStandardPage;
}

void SdXImpressDocument::SetModified() throw()
{
    if( mpDoc )
        mpDoc->SetChanged();
}

// XModel
void SAL_CALL SdXImpressDocument::lockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    mpDoc->setLock(true);
}

void SAL_CALL SdXImpressDocument::unlockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if( mpDoc->isLocked() )
    {
        mpDoc->setLock(false);
    }
}

sal_Bool SAL_CALL SdXImpressDocument::hasControllersLocked(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return mpDoc->isLocked();
}

uno::Reference < container::XIndexAccess > SAL_CALL SdXImpressDocument::getViewData()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        const std::vector<std::unique_ptr<sd::FrameView>> &rList = mpDoc->GetFrameViewList();

        if( !rList.empty() )
        {
            xRet = document::IndexedPropertyValues::create( ::comphelper::getProcessComponentContext() );

            uno::Reference < container::XIndexContainer > xCont( xRet, uno::UNO_QUERY );
            DBG_ASSERT( xCont.is(), "SdXImpressDocument::getViewData() failed for OLE object" );
            if( xCont.is() )
            {
                for( sal_uInt32 i = 0, n = rList.size(); i < n; i++ )
                {
                    ::sd::FrameView* pFrameView = rList[ i ].get();

                    uno::Sequence< beans::PropertyValue > aSeq;
                    pFrameView->WriteUserDataSequence( aSeq );
                    xCont->insertByIndex( i, uno::makeAny( aSeq ) );
                }
            }
        }
    }

    return xRet;
}

void SAL_CALL SdXImpressDocument::setViewData( const uno::Reference < container::XIndexAccess >& xData )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    SfxBaseModel::setViewData( xData );
    if( !(mpDocShell && (mpDocShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED) && xData.is()) )
        return;

    const sal_Int32 nCount = xData->getCount();

    std::vector<std::unique_ptr<sd::FrameView>> &rViews = mpDoc->GetFrameViewList();

    rViews.clear();

    uno::Sequence< beans::PropertyValue > aSeq;
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        if( xData->getByIndex( nIndex ) >>= aSeq )
        {
            std::unique_ptr<::sd::FrameView> pFrameView(new ::sd::FrameView( mpDoc ));
            pFrameView->ReadUserDataSequence( aSeq );
            rViews.push_back( std::move(pFrameView) );
        }
    }
}

// XDrawPageDuplicator
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::duplicate( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    // get pPage from xPage and determine the Id (nPos ) afterwards
    SvxDrawPage* pSvxPage = comphelper::getUnoTunnelImplementation<SvxDrawPage>( xPage );
    if( pSvxPage )
    {
        SdPage* pPage = static_cast<SdPage*>( pSvxPage->GetSdrPage() );
        sal_uInt16 nPos = pPage->GetPageNum();
        nPos = ( nPos - 1 ) / 2;
        pPage = InsertSdPage( nPos, true );
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
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPages >  xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
    {
        initializeDocument();
        mxDrawPagesAccess = xDrawPages = new SdDrawPagesAccess(*this);
    }

    return xDrawPages;
}

// XMasterPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getMasterPages()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPages >  xMasterPages( mxMasterPagesAccess );

    if( !xMasterPages.is() )
    {
        if ( !hasControllersLocked() )
            initializeDocument();
        mxMasterPagesAccess = xMasterPages = new SdMasterPagesAccess(*this);
    }

    return xMasterPages;
}

// XLayerManagerSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLayerManager(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess >  xLayerManager( mxLayerManager );

    if( !xLayerManager.is() )
        mxLayerManager = xLayerManager = new SdLayerManager(*this);

    return xLayerManager;
}

// XCustomPresentationSupplier
uno::Reference< container::XNameContainer > SAL_CALL SdXImpressDocument::getCustomPresentations()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameContainer >  xCustomPres( mxCustomPresentationAccess );

    if( !xCustomPres.is() )
        mxCustomPresentationAccess = xCustomPres = new SdXCustomPresentationAccess(*this);

    return xCustomPres;
}

// XPresentationSupplier
uno::Reference< presentation::XPresentation > SAL_CALL SdXImpressDocument::getPresentation()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return uno::Reference< presentation::XPresentation >( mpDoc->getPresentation().get() );
}

// XHandoutMasterSupplier
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::getHandoutMasterPage()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xPage;

    initializeDocument();
    SdPage* pPage = mpDoc->GetMasterSdPage(0, PageKind::Handout);
    if (pPage)
        xPage.set(pPage->getUnoPage(), uno::UNO_QUERY);
    return xPage;
}

// XMultiServiceFactory ( SvxFmMSFactory )

css::uno::Reference<css::uno::XInterface> SdXImpressDocument::create(
    OUString const & aServiceSpecifier, OUString const & referer)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
    {
        if( !mxDashTable.is() )
            mxDashTable = SvxUnoDashTable_createInstance( mpDoc );

        return mxDashTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
    {
        if( !mxGradientTable.is() )
            mxGradientTable = SvxUnoGradientTable_createInstance( mpDoc );

        return mxGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
    {
        if( !mxHatchTable.is() )
            mxHatchTable = SvxUnoHatchTable_createInstance( mpDoc );

        return mxHatchTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.BitmapTable" )
    {
        if( !mxBitmapTable.is() )
            mxBitmapTable = SvxUnoBitmapTable_createInstance( mpDoc );

        return mxBitmapTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
    {
        if( !mxTransGradientTable.is() )
            mxTransGradientTable = SvxUnoTransGradientTable_createInstance( mpDoc );

        return mxTransGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
    {
        if( !mxMarkerTable.is() )
            mxMarkerTable = SvxUnoMarkerTable_createInstance( mpDoc );

        return mxMarkerTable;
    }
    if( aServiceSpecifier == "com.sun.star.text.NumberingRules" )
    {
        return uno::Reference< uno::XInterface >( SvxCreateNumRule( mpDoc ), uno::UNO_QUERY );
    }
    if( aServiceSpecifier == "com.sun.star.drawing.Background" )
    {
        return uno::Reference< uno::XInterface >(
            static_cast<uno::XWeak*>(new SdUnoPageBackground( mpDoc )));
    }

    if( aServiceSpecifier == "com.sun.star.drawing.Defaults" )
    {
        if( !mxDrawingPool.is() )
            mxDrawingPool = SdUnoCreatePool( mpDoc );

        return mxDrawingPool;

    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapRectangleObject )
    {
        return SvUnoImageMapRectangleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapCircleObject )
    {
        return SvUnoImageMapCircleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapPolygonObject )
    {
        return SvUnoImageMapPolygonObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( aServiceSpecifier == "com.sun.star.document.Settings" ||
        ( !mbImpressDoc && ( aServiceSpecifier == "com.sun.star.drawing.DocumentSettings" ) ) ||
        (  mbImpressDoc && ( aServiceSpecifier == "com.sun.star.presentation.DocumentSettings" ) ) )
    {
        return sd::DocumentSettings_createInstance( this );
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.DateTime" ||
        aServiceSpecifier == "com.sun.star.text.textfield.DateTime" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::DATE ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Header" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Header" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_HEADER ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Footer" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Footer" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_FOOTER ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.DateTime" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.DateTime" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_DATE_TIME ));
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.PageName" ||
        aServiceSpecifier == "com.sun.star.text.textfield.PageName" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PAGE_NAME ));
    }

    if (aServiceSpecifier == "com.sun.star.text.TextField.DocInfo.Custom" ||
        aServiceSpecifier == "com.sun.star.text.textfield.DocInfo.Custom")
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField(text::textfield::Type::DOCINFO_CUSTOM));
    }

    if( aServiceSpecifier == "com.sun.star.xml.NamespaceMap" )
    {
        static sal_uInt16 aWhichIds[] = { SDRATTR_XMLATTRIBUTES, EE_CHAR_XMLATTRIBS, EE_PARA_XMLATTRIBS, 0 };

        return svx::NamespaceMap_createInstance( aWhichIds, &mpDoc->GetItemPool() );
    }

    // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
    if (aServiceSpecifier == "com.sun.star.document.ExportGraphicStorageHandler")
    {
        return static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Write ));
    }

    if (aServiceSpecifier == "com.sun.star.document.ImportGraphicStorageHandler")
    {
        return static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Read ));
    }

    if( aServiceSpecifier == "com.sun.star.document.ExportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Write ));
    }

    if( aServiceSpecifier == "com.sun.star.document.ImportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Read ));
    }

    uno::Reference< uno::XInterface > xRet;

    if( aServiceSpecifier.startsWith( "com.sun.star.presentation.") )
    {
        const OUString aType( aServiceSpecifier.copy(26) );
        SvxShape* pShape = nullptr;

        sal_uInt16 nType = OBJ_TEXT;
        // create a shape wrapper
        if( aType.startsWith( "TitleTextShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "OutlinerShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "SubtitleShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "GraphicObjectShape" ) )
        {
            nType = OBJ_GRAF;
        }
        else if( aType.startsWith( "PageShape" ) )
        {
            nType = OBJ_PAGE;
        }
        else if( aType.startsWith( "OLE2Shape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.startsWith( "ChartShape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.startsWith( "CalcShape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.startsWith( "TableShape" ) )
        {
            nType = OBJ_TABLE;
        }
        else if( aType.startsWith( "OrgChartShape" ) )
        {
            nType = OBJ_OLE2;
        }
        else if( aType.startsWith( "NotesShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "HandoutShape" ) )
        {
            nType = OBJ_PAGE;
        }
        else if( aType.startsWith( "FooterShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "HeaderShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "SlideNumberShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "DateTimeShape" ) )
        {
            nType = OBJ_TEXT;
        }
        else if( aType.startsWith( "MediaShape" ) )
        {
            nType = OBJ_MEDIA;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor::Default, referer );

        // set shape type
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = static_cast<uno::XWeak*>(pShape);
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.TableShape" )
    {
        SvxShape* pShape = CreateSvxShapeByTypeAndInventor( OBJ_TABLE, SdrInventor::Default, referer );
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = static_cast<uno::XWeak*>(pShape);
    }
    else
    {
        xRet = SvxFmMSFactory::createInstance( aServiceSpecifier );
    }

    uno::Reference< drawing::XShape > xShape( xRet, uno::UNO_QUERY );
    SvxShape* pShape = xShape.is() ? comphelper::getUnoTunnelImplementation<SvxShape>(xShape) : nullptr;
    if (pShape)
    {
        xRet.clear();
        new SdXShape( pShape, this );
        xRet = xShape;
        xShape.clear();
    }

    return xRet;
}

uno::Reference< uno::XInterface > SAL_CALL SdXImpressDocument::createInstance( const OUString& aServiceSpecifier )
{
    return create(aServiceSpecifier, "");
}

css::uno::Reference<css::uno::XInterface>
SdXImpressDocument::createInstanceWithArguments(
    OUString const & ServiceSpecifier,
    css::uno::Sequence<css::uno::Any> const & Arguments)
{
    OUString arg;
    if ((ServiceSpecifier == "com.sun.star.drawing.GraphicObjectShape"
         || ServiceSpecifier == "com.sun.star.drawing.MediaShape"
         || ServiceSpecifier == "com.sun.star.presentation.MediaShape")
        && Arguments.getLength() == 1 && (Arguments[0] >>= arg))
    {
        return create(ServiceSpecifier, arg);
    }
    return SvxFmMSFactory::createInstanceWithArguments(
        ServiceSpecifier, Arguments);
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getAvailableServiceNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS( mbImpressDoc ? 36 : 19 );

    sal_uInt16 i(0);

    aSNS[i++] = "com.sun.star.drawing.DashTable";
    aSNS[i++] = "com.sun.star.drawing.GradientTable";
    aSNS[i++] = "com.sun.star.drawing.HatchTable";
    aSNS[i++] = "com.sun.star.drawing.BitmapTable";
    aSNS[i++] = "com.sun.star.drawing.TransparencyGradientTable";
    aSNS[i++] = "com.sun.star.drawing.MarkerTable";
    aSNS[i++] = "com.sun.star.text.NumberingRules";
    aSNS[i++] = "com.sun.star.drawing.Background";
    aSNS[i++] = "com.sun.star.document.Settings";
    aSNS[i++] = sUNO_Service_ImageMapRectangleObject;
    aSNS[i++] = sUNO_Service_ImageMapCircleObject;
    aSNS[i++] = sUNO_Service_ImageMapPolygonObject;
    aSNS[i++] = "com.sun.star.xml.NamespaceMap";

    // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
    aSNS[i++] = "com.sun.star.document.ExportGraphicStorageHandler";
    aSNS[i++] = "com.sun.star.document.ImportGraphicStorageHandler";
    aSNS[i++] = "com.sun.star.document.ExportEmbeddedObjectResolver";
    aSNS[i++] = "com.sun.star.document.ImportEmbeddedObjectResolver";
    aSNS[i++] = "com.sun.star.drawing.TableShape";

    if(mbImpressDoc)
    {
        aSNS[i++] = "com.sun.star.presentation.TitleTextShape";
        aSNS[i++] = "com.sun.star.presentation.OutlinerShape";
        aSNS[i++] = "com.sun.star.presentation.SubtitleShape";
        aSNS[i++] = "com.sun.star.presentation.GraphicObjectShape";
        aSNS[i++] = "com.sun.star.presentation.ChartShape";
        aSNS[i++] = "com.sun.star.presentation.PageShape";
        aSNS[i++] = "com.sun.star.presentation.OLE2Shape";
        aSNS[i++] = "com.sun.star.presentation.TableShape";
        aSNS[i++] = "com.sun.star.presentation.OrgChartShape";
        aSNS[i++] = "com.sun.star.presentation.NotesShape";
        aSNS[i++] = "com.sun.star.presentation.HandoutShape";
        aSNS[i++] = "com.sun.star.presentation.DocumentSettings";
        aSNS[i++] = "com.sun.star.presentation.FooterShape";
        aSNS[i++] = "com.sun.star.presentation.HeaderShape";
        aSNS[i++] = "com.sun.star.presentation.SlideNumberShape";
        aSNS[i++] = "com.sun.star.presentation.DateTimeShape";
        aSNS[i++] = "com.sun.star.presentation.CalcShape";
        aSNS[i++] = "com.sun.star.presentation.MediaShape";
    }
    else
    {
        aSNS[i++] = "com.sun.star.drawing.DocumentSettings";
    }

    DBG_ASSERT( i == aSNS.getLength(), "Sequence overrun!" );

    return comphelper::concatSequences( aSNS_ORG, aSNS );
}

// lang::XServiceInfo
OUString SAL_CALL SdXImpressDocument::getImplementationName()
{
    return "SdXImpressDocument";
    /* // Matching the .component information:
       return mbImpressDoc
           ? OUString("com.sun.star.comp.Draw.PresentationDocument")
           : OUString("com.sun.star.comp.Draw.DrawingDocument");
    */
}

sal_Bool SAL_CALL SdXImpressDocument::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    return { "com.sun.star.document.OfficeDocument",
             "com.sun.star.drawing.GenericDrawingDocument",
             "com.sun.star.drawing.DrawingDocumentFactory",
             mbImpressDoc?OUString("com.sun.star.presentation.PresentationDocument"):OUString("com.sun.star.drawing.DrawingDocument") };
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdXImpressDocument::getPropertySetInfo(  )
{
    ::SolarMutexGuard aGuard;
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdXImpressDocument::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            lang::Locale aLocale;
            if(!(aValue >>= aLocale))
                throw lang::IllegalArgumentException();

            mpDoc->SetLanguage( LanguageTag::convertToLanguageType(aLocale), EE_CHAR_LANGUAGE );
            break;
        }
        case WID_MODEL_TABSTOP:
        {
            sal_Int32 nValue = 0;
            if(!(aValue >>= nValue) || nValue < 0 )
                throw lang::IllegalArgumentException();

            mpDoc->SetDefaultTabulator(static_cast<sal_uInt16>(nValue));
            break;
        }
        case WID_MODEL_VISAREA:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                awt::Rectangle aVisArea;
                if( !(aValue >>= aVisArea) || (aVisArea.Width < 0) || (aVisArea.Height < 0) )
                    throw lang::IllegalArgumentException();

                sal_Int32 nRight, nTop;
                if (o3tl::checked_add(aVisArea.X, aVisArea.Width, nRight) || o3tl::checked_add(aVisArea.Y, aVisArea.Height, nTop))
                    throw lang::IllegalArgumentException();

                pEmbeddedObj->SetVisArea(::tools::Rectangle(aVisArea.X, aVisArea.Y, nRight, nTop));
            }
            break;
        case WID_MODEL_CONTFOCUS:
            {
                bool bFocus = false;
                if( !(aValue >>= bFocus ) )
                    throw lang::IllegalArgumentException();
                mpDoc->SetAutoControlFocus( bFocus );
            }
            break;
        case WID_MODEL_DSGNMODE:
            {
                bool bMode = false;
                if( !(aValue >>= bMode ) )
                    throw lang::IllegalArgumentException();
                mpDoc->SetOpenInDesignMode( bMode );
            }
            break;
        case WID_MODEL_BUILDID:
            aValue >>= maBuildId;
            return;
        case WID_MODEL_MAPUNIT:
        case WID_MODEL_BASICLIBS:
        case WID_MODEL_RUNTIMEUID: // is read-only
        case WID_MODEL_DIALOGLIBS:
        case WID_MODEL_FONTS:
            throw beans::PropertyVetoException();
        case WID_MODEL_INTEROPGRABBAG:
            setGrabBagItem(aValue);
            break;
        default:
            throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    SetModified();
}

uno::Any SAL_CALL SdXImpressDocument::getPropertyValue( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    uno::Any aAny;
    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const SfxItemPropertySimpleEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            LanguageType eLang = mpDoc->GetLanguage( EE_CHAR_LANGUAGE );
            aAny <<= LanguageTag::convertToLocale( eLang);
            break;
        }
        case WID_MODEL_TABSTOP:
            aAny <<= static_cast<sal_Int32>(mpDoc->GetDefaultTabulator());
            break;
        case WID_MODEL_VISAREA:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                const ::tools::Rectangle& aRect = pEmbeddedObj->GetVisArea();
                awt::Rectangle aVisArea( aRect.Left(), aRect.Top(), aRect.getWidth(), aRect.getHeight() );
                aAny <<= aVisArea;
            }
            break;
        case WID_MODEL_MAPUNIT:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                sal_Int16 nMeasureUnit = 0;
                SvxMapUnitToMeasureUnit( pEmbeddedObj->GetMapUnit(), nMeasureUnit );
                aAny <<= nMeasureUnit;
        }
        break;
        case WID_MODEL_FORBCHARS:
        {
            aAny <<= getForbiddenCharsTable();
        }
        break;
        case WID_MODEL_CONTFOCUS:
            aAny <<= mpDoc->GetAutoControlFocus();
            break;
        case WID_MODEL_DSGNMODE:
            aAny <<= mpDoc->GetOpenInDesignMode();
            break;
        case WID_MODEL_BASICLIBS:
            aAny <<= mpDocShell->GetBasicContainer();
            break;
        case WID_MODEL_DIALOGLIBS:
            aAny <<= mpDocShell->GetDialogContainer();
            break;
        case WID_MODEL_RUNTIMEUID:
            aAny <<= getRuntimeUID();
            break;
        case WID_MODEL_BUILDID:
            return uno::Any( maBuildId );
        case WID_MODEL_HASVALIDSIGNATURES:
            aAny <<= hasValidSignatures();
            break;
        case WID_MODEL_FONTS:
            {
                uno::Sequence<uno::Any> aSeq;
                int nSeqIndex = 0;

                sal_uInt16 const aWhichIds[] { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                               EE_CHAR_FONTINFO_CTL };

                const SfxItemPool& rPool = mpDoc->GetPool();

                for(sal_uInt16 nWhichId : aWhichIds)
                {
                    sal_uInt32 nItems = rPool.GetItemCount2( nWhichId );

                    aSeq.realloc( aSeq.getLength() + nItems*5 + 5 );

                    for (const SfxPoolItem* pItem : rPool.GetItemSurrogates(nWhichId))
                    {
                        const SvxFontItem *pFont = static_cast<const SvxFontItem *>(pItem);

                        aSeq[nSeqIndex++] <<= pFont->GetFamilyName();
                        aSeq[nSeqIndex++] <<= pFont->GetStyleName();
                        aSeq[nSeqIndex++] <<= sal_Int16(pFont->GetFamily());
                        aSeq[nSeqIndex++] <<= sal_Int16(pFont->GetPitch());
                        aSeq[nSeqIndex++] <<= sal_Int16(pFont->GetCharSet());
                    }

                    const SvxFontItem& rFont = static_cast<const SvxFontItem&>(rPool.GetDefaultItem( nWhichId ));

                    aSeq[nSeqIndex++] <<= rFont.GetFamilyName();
                    aSeq[nSeqIndex++] <<= rFont.GetStyleName();
                    aSeq[nSeqIndex++] <<= sal_Int16(rFont.GetFamily());
                    aSeq[nSeqIndex++] <<= sal_Int16(rFont.GetPitch());
                    aSeq[nSeqIndex++] <<= sal_Int16(rFont.GetCharSet());

                }

                aSeq.realloc( nSeqIndex );
                aAny <<= aSeq;
                break;
            }
        case WID_MODEL_INTEROPGRABBAG:
            getGrabBagItem(aAny);
            break;
        default:
            throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    return aAny;
}

void SAL_CALL SdXImpressDocument::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}

// XLinkTargetSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLinks()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xLinks( mxLinks );
    if( !xLinks.is() )
        mxLinks = xLinks = new SdDocLinkTargets( *this );
    return xLinks;
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getStyleFamilies(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xStyles( dynamic_cast< container::XNameAccess* >( mpDoc->GetStyleSheetPool()) );
    return xStyles;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SdXImpressDocument::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

// XRenderable
sal_Int32 SAL_CALL SdXImpressDocument::getRendererCount( const uno::Any& rSelection,
                                                         const uno::Sequence< beans::PropertyValue >&  )
{
    ::SolarMutexGuard aGuard;
    sal_Int32   nRet = 0;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if (mpDocShell)
    {
        uno::Reference< frame::XModel > xModel;

        rSelection >>= xModel;

        if( xModel == mpDocShell->GetModel() )
            nRet = mpDoc->GetSdPageCount( PageKind::Standard );
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

uno::Sequence< beans::PropertyValue > SAL_CALL SdXImpressDocument::getRenderer( sal_Int32 , const uno::Any& ,
                                                                                const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    bool bExportNotesPages = false;
    for( const auto& rOption : rxOptions )
    {
        if ( rOption.Name == "ExportNotesPages" )
            rOption.Value >>= bExportNotesPages;
    }
    uno::Sequence< beans::PropertyValue > aRenderer;
    if (mpDocShell)
    {
        awt::Size aPageSize;
        if ( bExportNotesPages )
        {
            Size aNotesPageSize = mpDoc->GetSdPage( 0, PageKind::Notes )->GetSize();
            aPageSize = awt::Size( aNotesPageSize.Width(), aNotesPageSize.Height() );
        }
        else
        {
            const ::tools::Rectangle aVisArea( mpDocShell->GetVisArea( embed::Aspects::MSOLE_DOCPRINT ) );
            aPageSize = awt::Size( aVisArea.GetWidth(), aVisArea.GetHeight() );
        }
        aRenderer.realloc( 1 );

        aRenderer[ 0 ].Name = "PageSize" ;
        aRenderer[ 0 ].Value <<= aPageSize;
    }
    return aRenderer;
}

namespace {

class ImplRenderPaintProc : public sdr::contact::ViewObjectContactRedirector
{
    const SdrLayerAdmin&    rLayerAdmin;
    SdrPageView*            pSdrPageView;
    vcl::PDFExtOutDevData*  pPDFExtOutDevData;

    vcl::PDFWriter::StructElement ImplBegStructureTag( SdrObject& rObject );

public:
    bool IsVisible  ( const SdrObject* pObj ) const;
    bool IsPrintable( const SdrObject* pObj ) const;

    ImplRenderPaintProc( const SdrLayerAdmin& rLA, SdrPageView* pView, vcl::PDFExtOutDevData* pData );

    // all default implementations just call the same methods at the original. To do something
    // different, override the method and at least do what the method does.
    virtual drawinglayer::primitive2d::Primitive2DContainer createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo) override;
};

}

ImplRenderPaintProc::ImplRenderPaintProc( const SdrLayerAdmin& rLA, SdrPageView* pView, vcl::PDFExtOutDevData* pData )
:   ViewObjectContactRedirector(),
    rLayerAdmin         ( rLA ),
    pSdrPageView        ( pView ),
    pPDFExtOutDevData   ( pData )
{
}

static sal_Int32 ImplPDFGetBookmarkPage( const OUString& rBookmark, SdDrawDocument const & rDoc )
{
    sal_Int32 nPage = -1;

    OUString aBookmark( rBookmark );

    if( rBookmark.startsWith("#") )
        aBookmark = rBookmark.copy( 1 );

    // is the bookmark a page ?
    bool        bIsMasterPage;
    sal_uInt16  nPgNum = rDoc.GetPageByName( aBookmark, bIsMasterPage );

    if ( nPgNum == SDRPAGE_NOTFOUND )
    {
        // is the bookmark an object ?
        SdrObject* pObj = rDoc.GetObj( aBookmark );
        if (pObj)
            nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNum();
    }
    if ( nPgNum != SDRPAGE_NOTFOUND )
        nPage = ( nPgNum - 1 ) / 2;
    return nPage;
}

static void ImplPDFExportComments( const uno::Reference< drawing::XDrawPage >& xPage, vcl::PDFExtOutDevData& rPDFExtOutDevData )
{
    try
    {
        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( xPage, uno::UNO_QUERY_THROW );
        uno::Reference< office::XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );

        while( xAnnotationEnumeration->hasMoreElements() )
        {
            uno::Reference< office::XAnnotation > xAnnotation( xAnnotationEnumeration->nextElement() );

            geometry::RealPoint2D aRealPoint2D( xAnnotation->getPosition() );
            uno::Reference< text::XText > xText( xAnnotation->getTextRange() );

            vcl::PDFNote aNote;
            aNote.Title = xAnnotation->getAuthor();
            aNote.Contents = xText->getString();
            aNote.maModificationDate = xAnnotation->getDateTime();

            rPDFExtOutDevData.CreateNote( ::tools::Rectangle( Point( static_cast< ::tools::Long >( aRealPoint2D.X * 100 ),
                static_cast< ::tools::Long >( aRealPoint2D.Y * 100 ) ), Size( 1000, 1000 ) ), aNote );
        }
    }
    catch (const uno::Exception&)
    {
    }
}

static void ImplPDFExportShapeInteraction( const uno::Reference< drawing::XShape >& xShape, SdDrawDocument& rDoc, vcl::PDFExtOutDevData& rPDFExtOutDevData )
{
    if ( xShape->getShapeType() == "com.sun.star.drawing.GroupShape" )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( xShape, uno::UNO_QUERY );
        if ( xIndexAccess.is() )
        {
            sal_Int32 i, nCount = xIndexAccess->getCount();
            for ( i = 0; i < nCount; i++ )
            {
                uno::Reference< drawing::XShape > xSubShape( xIndexAccess->getByIndex( i ), uno::UNO_QUERY );
                if ( xSubShape.is() )
                    ImplPDFExportShapeInteraction( xSubShape, rDoc, rPDFExtOutDevData );
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xShapePropSet( xShape, uno::UNO_QUERY );
        if( xShapePropSet.is() )
        {
            Size        aPageSize( rDoc.GetSdPage( 0, PageKind::Standard )->GetSize() );
            Point aPoint( 0, 0 );
            ::tools::Rectangle   aPageRect( aPoint, aPageSize );

            awt::Point  aShapePos( xShape->getPosition() );
            awt::Size   aShapeSize( xShape->getSize() );
            ::tools::Rectangle   aLinkRect( Point( aShapePos.X, aShapePos.Y ), Size( aShapeSize.Width, aShapeSize.Height ) );

            // Handle linked videos.
            if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape" || xShape->getShapeType() == "com.sun.star.presentation.MediaShape")
            {
                OUString aMediaURL;
                xShapePropSet->getPropertyValue("MediaURL") >>= aMediaURL;
                if (!aMediaURL.isEmpty())
                {
                    sal_Int32 nScreenId = rPDFExtOutDevData.CreateScreen(aLinkRect, rPDFExtOutDevData.GetCurrentPageNumber());
                    if (aMediaURL.startsWith("vnd.sun.star.Package:"))
                    {
                        OUString aTempFileURL;
                        xShapePropSet->getPropertyValue("PrivateTempFileURL") >>= aTempFileURL;
                        rPDFExtOutDevData.SetScreenStream(nScreenId, aTempFileURL);
                    }
                    else
                        rPDFExtOutDevData.SetScreenURL(nScreenId, aMediaURL);
                }
            }

            presentation::ClickAction eCa;
            uno::Any aAny( xShapePropSet->getPropertyValue( "OnClick" ) );
            if ( aAny >>= eCa )
            {
                switch ( eCa )
                {
                    case presentation::ClickAction_LASTPAGE :
                    {
                        sal_Int32 nCount = rDoc.GetSdPageCount( PageKind::Standard );
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nCount - 1, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_FIRSTPAGE :
                    {
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, 0, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_PREVPAGE :
                    {
                        sal_Int32 nDestPage = rPDFExtOutDevData.GetCurrentPageNumber();
                        if ( nDestPage )
                            nDestPage--;
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_NEXTPAGE :
                    {
                        sal_Int32 nDestPage = rPDFExtOutDevData.GetCurrentPageNumber() + 1;
                        sal_Int32 nLastPage = rDoc.GetSdPageCount( PageKind::Standard ) - 1;
                        if ( nDestPage > nLastPage )
                            nDestPage = nLastPage;
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;

                    case presentation::ClickAction_PROGRAM :
                    case presentation::ClickAction_BOOKMARK :
                    case presentation::ClickAction_DOCUMENT :
                    {
                        OUString aBookmark;
                        xShapePropSet->getPropertyValue( "Bookmark" ) >>= aBookmark;
                        if( !aBookmark.isEmpty() )
                        {
                            switch( eCa )
                            {
                                case presentation::ClickAction_DOCUMENT :
                                case presentation::ClickAction_PROGRAM :
                                {
                                    sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                                    rPDFExtOutDevData.SetLinkURL( nLinkId, aBookmark );
                                }
                                break;
                                case presentation::ClickAction_BOOKMARK :
                                {
                                    sal_Int32 nPage = ImplPDFGetBookmarkPage( aBookmark, rDoc );
                                    if ( nPage != -1 )
                                    {
                                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle );
                                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink( aLinkRect );
                                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                                    }
                                }
                                break;
                                default:
                                    break;
                            }
                        }
                    }
                    break;

                    case presentation::ClickAction_STOPPRESENTATION :
                    case presentation::ClickAction_SOUND :
                    case presentation::ClickAction_INVISIBLE :
                    case presentation::ClickAction_VERB :
                    case presentation::ClickAction_VANISH :
                    case presentation::ClickAction_MACRO :
                    default :
                    break;
                }
            }
        }
    }
}

vcl::PDFWriter::StructElement ImplRenderPaintProc::ImplBegStructureTag( SdrObject& rObject )
{
    vcl::PDFWriter::StructElement eElement(vcl::PDFWriter::NonStructElement);

    if ( pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportTaggedPDF() )
    {
        SdrInventor nInventor   = rObject.GetObjInventor();
        sal_uInt16  nIdentifier = rObject.GetObjIdentifier();
        bool        bIsTextObj  = dynamic_cast< const SdrTextObj *>( &rObject ) !=  nullptr;

        if ( nInventor == SdrInventor::Default )
        {
            if ( nIdentifier == OBJ_GRUP )
                eElement = vcl::PDFWriter::Section;
            else if ( nIdentifier == OBJ_TITLETEXT )
                eElement = vcl::PDFWriter::Heading;
            else if ( nIdentifier == OBJ_OUTLINETEXT )
                eElement = vcl::PDFWriter::Division;
            else if ( !bIsTextObj || !static_cast<SdrTextObj&>(rObject).HasText() )
                eElement = vcl::PDFWriter::Figure;
        }
    }

    return eElement;
}

drawinglayer::primitive2d::Primitive2DContainer ImplRenderPaintProc::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        drawinglayer::primitive2d::Primitive2DContainer xRetval;

        if(pObject->getSdrPageFromSdrObject())
        {
            if(pObject->getSdrPageFromSdrObject()->checkVisibility(rOriginal, rDisplayInfo, false))
            {
                if(IsVisible(pObject) && IsPrintable(pObject))
                {
                    const vcl::PDFWriter::StructElement eElement(ImplBegStructureTag( *pObject ));
                    const bool bTagUsed(vcl::PDFWriter::NonStructElement != eElement);

                    xRetval = sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);

                    if(!xRetval.empty() && bTagUsed)
                    {
                        // embed Primitive2DSequence in a structure tag element for
                        // exactly this purpose (StructureTagPrimitive2D)

                        const SdrPage* pSdrPage(pObject->getSdrPageFromSdrObject());
                        const bool bBackground(nullptr != pSdrPage && pSdrPage->IsMasterPage());
                        const bool bImage(pObject->GetObjIdentifier() == OBJ_GRAF);

                        const drawinglayer::primitive2d::Primitive2DReference xReference(
                            new drawinglayer::primitive2d::StructureTagPrimitive2D(
                                eElement,
                                bBackground,
                                bImage,
                                xRetval));

                        xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };
                    }
                }
            }
        }

        return xRetval;
    }
    else
    {
        // not an object, maybe a page
        return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }
}

bool ImplRenderPaintProc::IsVisible( const SdrObject* pObj ) const
{
    bool bVisible = true;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID( nLayerId );
        if ( pSdrLayer )
        {
            const OUString& aLayerName = pSdrLayer->GetName();
            bVisible = pSdrPageView->IsLayerVisible( aLayerName );
        }
    }
    return bVisible;
}
bool ImplRenderPaintProc::IsPrintable( const SdrObject* pObj ) const
{
    bool bPrintable = true;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID( nLayerId );
        if ( pSdrLayer )
        {
            const OUString& aLayerName = pSdrLayer->GetName();
            bPrintable = pSdrPageView->IsLayerPrintable( aLayerName );
        }
    }
    return bPrintable;

}

namespace
{
    sal_Int16 CalcOutputPageNum(vcl::PDFExtOutDevData const * pPDFExtOutDevData, SdDrawDocument const *pDoc, sal_Int16 nPageNumber)
    {
        //export all pages, simple one to one case
        if (pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportHiddenSlides())
            return nPageNumber-1;
        //check all preceding pages, and only count non-hidden ones
        sal_Int16 nRet = 0;
        for (sal_Int16 i = 0; i < nPageNumber-1; ++i)
        {
           if (!pDoc->GetSdPage(i, PageKind::Standard)->IsExcluded())
                ++nRet;
        }
        return nRet;
    }
}

void SAL_CALL SdXImpressDocument::render( sal_Int32 nRenderer, const uno::Any& rSelection,
                                          const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if (!mpDocShell)
        return;

    uno::Reference< awt::XDevice >  xRenderDevice;
    const sal_Int32                 nPageNumber = nRenderer + 1;
    PageKind                        ePageKind = PageKind::Standard;
    bool                        bExportNotesPages = false;

    for( const auto& rOption : rxOptions )
    {
        if ( rOption.Name == "RenderDevice" )
            rOption.Value >>= xRenderDevice;
        else if ( rOption.Name == "ExportNotesPages" )
        {
            rOption.Value >>= bExportNotesPages;
            if ( bExportNotesPages )
                ePageKind = PageKind::Notes;
        }
    }

    if( !(xRenderDevice.is() && nPageNumber && ( nPageNumber <= mpDoc->GetSdPageCount( ePageKind ) )) )
        return;

    VCLXDevice* pDevice = comphelper::getUnoTunnelImplementation<VCLXDevice>( xRenderDevice );
    VclPtr< OutputDevice> pOut = pDevice ? pDevice->GetOutputDevice() : VclPtr< OutputDevice >();

    if( !pOut )
        return;

    vcl::PDFExtOutDevData* pPDFExtOutDevData = dynamic_cast<vcl::PDFExtOutDevData* >( pOut->GetExtOutDevData() );

    if ( mpDoc->GetSdPage(static_cast<sal_Int16>(nPageNumber)-1, PageKind::Standard)->IsExcluded() &&
        !(pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportHiddenSlides()) )
        return;

    if (pPDFExtOutDevData)
    {
        // Calculate the page number in the PDF output, which may be smaller than the page number in
        // case of hidden slides.
        sal_Int32 nOutputPageNum = CalcOutputPageNum(pPDFExtOutDevData, mpDoc, nPageNumber);
        pPDFExtOutDevData->SetCurrentPageNumber(nOutputPageNum);
    }

    std::unique_ptr<::sd::ClientView> pView( new ::sd::ClientView( mpDocShell, pOut ) );
    ::tools::Rectangle aVisArea( Point(), mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1, ePageKind )->GetSize() );
    vcl::Region                       aRegion( aVisArea );

    ::sd::ViewShell* pOldViewSh = mpDocShell->GetViewShell();
    ::sd::View* pOldSdView = pOldViewSh ? pOldViewSh->GetView() : nullptr;

    if  ( pOldSdView )
        pOldSdView->SdrEndTextEdit();

    pView->SetHlplVisible( false );
    pView->SetGridVisible( false );
    pView->SetBordVisible( false );
    pView->SetPageVisible( false );
    pView->SetGlueVisible( false );

    pOut->SetMapMode(MapMode(MapUnit::Map100thMM));
    pOut->IntersectClipRegion( aVisArea );

    uno::Reference< frame::XModel > xModel;
    rSelection >>= xModel;

    if( xModel == mpDocShell->GetModel() )
    {
        pView->ShowSdrPage( mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1, ePageKind ));
        SdrPageView* pPV = pView->GetSdrPageView();

        if( pOldSdView )
        {
            SdrPageView* pOldPV = pOldSdView->GetSdrPageView();
            if( pPV && pOldPV )
            {
                pPV->SetVisibleLayers( pOldPV->GetVisibleLayers() );
                pPV->SetPrintableLayers( pOldPV->GetPrintableLayers() );
            }
        }

        ImplRenderPaintProc aImplRenderPaintProc( mpDoc->GetLayerAdmin(),
            pPV, pPDFExtOutDevData );

        // background color for outliner :o
        SdPage* pPage = pPV ? static_cast<SdPage*>(pPV->GetPage()) : nullptr;
        if( pPage )
        {
            SdrOutliner& rOutl = mpDoc->GetDrawOutliner();
            bool bScreenDisplay(true);

            // #i75566# printing; suppress AutoColor BackgroundColor generation
            // for visibility reasons by giving GetPageBackgroundColor()
            // the needed hint
            // #i75566# PDF export; suppress AutoColor BackgroundColor generation (see printing)
            if (pOut && ((OUTDEV_PRINTER == pOut->GetOutDevType())
                    || (OUTDEV_PDF == pOut->GetOutDevType())))
                bScreenDisplay = false;

            // #i75566# Name change GetBackgroundColor -> GetPageBackgroundColor and
            // hint value if screen display. Only then the AutoColor mechanisms shall be applied
            rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor( pPV, bScreenDisplay ) );
        }
        pView->SdrPaintView::CompleteRedraw( pOut, aRegion, &aImplRenderPaintProc );

        if ( pPDFExtOutDevData && pPage )
        {
            try
            {
                uno::Any aAny;
                uno::Reference< drawing::XDrawPage > xPage( uno::Reference< drawing::XDrawPage >::query( pPage->getUnoPage() ) );
                if ( xPage.is() )
                {
                    if ( pPDFExtOutDevData->GetIsExportNotes() )
                        ImplPDFExportComments( xPage, *pPDFExtOutDevData );
                    uno::Reference< beans::XPropertySet > xPagePropSet( xPage, uno::UNO_QUERY );
                    if( xPagePropSet.is() )
                    {
                        // exporting object interactions to pdf

                        // if necessary, the master page interactions will be exported first
                        bool bIsBackgroundObjectsVisible = false;   // #i39428# IsBackgroundObjectsVisible not available for Draw
                        if ( mbImpressDoc && xPagePropSet->getPropertySetInfo()->hasPropertyByName( "IsBackgroundObjectsVisible" ) )
                            xPagePropSet->getPropertyValue( "IsBackgroundObjectsVisible" ) >>= bIsBackgroundObjectsVisible;
                        if ( bIsBackgroundObjectsVisible && !pPDFExtOutDevData->GetIsExportNotesPages() )
                        {
                            uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( xPage, uno::UNO_QUERY );
                            if ( xMasterPageTarget.is() )
                            {
                                uno::Reference< drawing::XDrawPage > xMasterPage = xMasterPageTarget->getMasterPage();
                                if ( xMasterPage.is() )
                                {
                                    sal_Int32 i, nCount = xMasterPage->getCount();
                                    for ( i = 0; i < nCount; i++ )
                                    {
                                        aAny = xMasterPage->getByIndex( i );
                                        uno::Reference< drawing::XShape > xShape;
                                        if ( aAny >>= xShape )
                                            ImplPDFExportShapeInteraction( xShape, *mpDoc, *pPDFExtOutDevData );
                                    }
                                }
                            }
                        }

                        // exporting slide page object interactions
                        sal_Int32 i, nCount = xPage->getCount();
                        for ( i = 0; i < nCount; i++ )
                        {
                            aAny = xPage->getByIndex( i );
                            uno::Reference< drawing::XShape > xShape;
                            if ( aAny >>= xShape )
                                ImplPDFExportShapeInteraction( xShape, *mpDoc, *pPDFExtOutDevData );
                        }

                        // exporting transition effects to pdf
                        if ( mbImpressDoc && !pPDFExtOutDevData->GetIsExportNotesPages() && pPDFExtOutDevData->GetIsExportTransitionEffects() )
                        {
                            const OUString sEffect( "Effect" );
                            const OUString sSpeed ( "Speed" );
                            sal_Int32 nTime = 800;
                            presentation::AnimationSpeed aAs;
                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sSpeed ) )
                            {
                                aAny = xPagePropSet->getPropertyValue( sSpeed );
                                if ( aAny >>= aAs )
                                {
                                    switch( aAs )
                                    {
                                        case presentation::AnimationSpeed_SLOW : nTime = 1500; break;
                                        case presentation::AnimationSpeed_FAST : nTime = 300; break;
                                        default:
                                        case presentation::AnimationSpeed_MEDIUM : nTime = 800;
                                    }
                                }
                            }
                            presentation::FadeEffect eFe;
                            vcl::PDFWriter::PageTransition eType = vcl::PDFWriter::PageTransition::Regular;
                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sEffect ) )
                            {
                                aAny = xPagePropSet->getPropertyValue( sEffect );
                                if ( aAny >>= eFe )
                                {
                                    switch( eFe )
                                    {
                                        case presentation::FadeEffect_HORIZONTAL_LINES :
                                        case presentation::FadeEffect_HORIZONTAL_CHECKERBOARD :
                                        case presentation::FadeEffect_HORIZONTAL_STRIPES : eType = vcl::PDFWriter::PageTransition::BlindsHorizontal; break;

                                        case presentation::FadeEffect_VERTICAL_LINES :
                                        case presentation::FadeEffect_VERTICAL_CHECKERBOARD :
                                        case presentation::FadeEffect_VERTICAL_STRIPES : eType = vcl::PDFWriter::PageTransition::BlindsVertical; break;

                                        case presentation::FadeEffect_UNCOVER_TO_RIGHT :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_LEFT :
                                        case presentation::FadeEffect_FADE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_LEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LEFT : eType = vcl::PDFWriter::PageTransition::WipeLeftToRight; break;

                                        case presentation::FadeEffect_UNCOVER_TO_BOTTOM :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_TOP :
                                        case presentation::FadeEffect_FADE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_TOP :
                                        case presentation::FadeEffect_MOVE_FROM_TOP : eType = vcl::PDFWriter::PageTransition::WipeTopToBottom; break;

                                        case presentation::FadeEffect_UNCOVER_TO_LEFT :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_RIGHT :

                                        case presentation::FadeEffect_FADE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_RIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_RIGHT : eType = vcl::PDFWriter::PageTransition::WipeRightToLeft; break;

                                        case presentation::FadeEffect_UNCOVER_TO_TOP :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_BOTTOM :
                                        case presentation::FadeEffect_FADE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_BOTTOM :
                                        case presentation::FadeEffect_MOVE_FROM_BOTTOM : eType = vcl::PDFWriter::PageTransition::WipeBottomToTop; break;

                                        case presentation::FadeEffect_OPEN_VERTICAL : eType = vcl::PDFWriter::PageTransition::SplitHorizontalInward; break;
                                        case presentation::FadeEffect_CLOSE_HORIZONTAL : eType = vcl::PDFWriter::PageTransition::SplitHorizontalOutward; break;

                                        case presentation::FadeEffect_OPEN_HORIZONTAL : eType = vcl::PDFWriter::PageTransition::SplitVerticalInward; break;
                                        case presentation::FadeEffect_CLOSE_VERTICAL : eType = vcl::PDFWriter::PageTransition::SplitVerticalOutward; break;

                                        case presentation::FadeEffect_FADE_TO_CENTER : eType = vcl::PDFWriter::PageTransition::BoxInward; break;
                                        case presentation::FadeEffect_FADE_FROM_CENTER : eType = vcl::PDFWriter::PageTransition::BoxOutward; break;

                                        case presentation::FadeEffect_NONE : eType = vcl::PDFWriter::PageTransition::Regular; break;

                                        case presentation::FadeEffect_RANDOM :
                                        case presentation::FadeEffect_DISSOLVE :
                                        default: eType = vcl::PDFWriter::PageTransition::Dissolve; break;
                                    }
                                }
                            }

                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sEffect ) ||
                                xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sSpeed ) )
                            {
                                pPDFExtOutDevData->SetPageTransition( eType, nTime );
                            }
                        }
                    }
                }

                Size        aPageSize( mpDoc->GetSdPage( 0, PageKind::Standard )->GetSize() );
                Point aPoint( 0, 0 );
                ::tools::Rectangle   aPageRect( aPoint, aPageSize );

                // resolving links found in this page by the method ImpEditEngine::Paint
                std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
                for ( const auto& rBookmark : rBookmarks )
                {
                    sal_Int32 nPage = ImplPDFGetBookmarkPage( rBookmark.aBookmark, *mpDoc );
                    if ( nPage != -1 )
                    {
                        if ( rBookmark.nLinkId != -1 )
                            pPDFExtOutDevData->SetLinkDest( rBookmark.nLinkId, pPDFExtOutDevData->CreateDest( aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle ) );
                        else
                            pPDFExtOutDevData->DescribeRegisteredDest( rBookmark.nDestId, aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle );
                    }
                    else
                        pPDFExtOutDevData->SetLinkURL( rBookmark.nLinkId, rBookmark.aBookmark );
                }
                rBookmarks.clear();
                //---> #i56629, #i40318
                //get the page name, will be used as outline element in PDF bookmark pane
                OUString aPageName = mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1 , PageKind::Standard )->GetName();
                if( !aPageName.isEmpty() )
                {
                    // Destination PageNum
                    const sal_Int32 nDestPageNum = CalcOutputPageNum(pPDFExtOutDevData, mpDoc, nPageNumber);

                    // insert the bookmark to this page into the NamedDestinations
                    if( pPDFExtOutDevData->GetIsExportNamedDestinations() )
                        pPDFExtOutDevData->CreateNamedDest(aPageName, aPageRect, nDestPageNum);

                    // add the name to the outline, (almost) same code as in sc/source/ui/unoobj/docuno.cxx
                    // issue #i40318.

                    if( pPDFExtOutDevData->GetIsExportBookmarks() )
                    {
                        // Destination Export
                        const sal_Int32 nDestId =
                            pPDFExtOutDevData->CreateDest(aPageRect , nDestPageNum);

                        // Create a new outline item:
                        pPDFExtOutDevData->CreateOutlineItem( -1 , aPageName, nDestId );
                    }
                }
                //<--- #i56629, #i40318
            }
            catch (const uno::Exception&)
            {
            }

        }
    }
    else
    {
        uno::Reference< drawing::XShapes > xShapes;
        rSelection >>= xShapes;

        if( xShapes.is() && xShapes->getCount() )
        {
            SdrPageView* pPV = nullptr;

            ImplRenderPaintProc  aImplRenderPaintProc( mpDoc->GetLayerAdmin(),
                                pOldSdView ? pOldSdView->GetSdrPageView() : nullptr, pPDFExtOutDevData );

            for( sal_uInt32 i = 0, nCount = xShapes->getCount(); i < nCount; i++ )
            {
                uno::Reference< drawing::XShape > xShape;
                xShapes->getByIndex( i ) >>= xShape;

                if( xShape.is() )
                {
                    SvxShape* pShape = comphelper::getUnoTunnelImplementation<SvxShape>( xShape );

                    if( pShape )
                    {
                        SdrObject* pObj = pShape->GetSdrObject();
                        if( pObj && pObj->getSdrPageFromSdrObject()
                            && aImplRenderPaintProc.IsVisible( pObj )
                                && aImplRenderPaintProc.IsPrintable( pObj ) )
                        {
                            if( !pPV )
                                pPV = pView->ShowSdrPage( pObj->getSdrPageFromSdrObject() );

                            if( pPV )
                                pView->MarkObj( pObj, pPV );
                        }
                    }
                }
            }
            pView->DrawMarkedObj(*pOut);
        }
    }
}

DrawViewShell* SdXImpressDocument::GetViewShell()
{
    DrawViewShell* pViewSh = dynamic_cast<DrawViewShell*>(mpDocShell->GetViewShell());
    if (!pViewSh)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return nullptr;
    }
    return pViewSh;
}

void SdXImpressDocument::paintTile( VirtualDevice& rDevice,
                            int nOutputWidth, int nOutputHeight,
                            int nTilePosX, int nTilePosY,
                            ::tools::Long nTileWidth, ::tools::Long nTileHeight )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    // Scaling. Must convert from pixels to twips. We know
    // that VirtualDevices use a DPI of 96.
    // We specifically calculate these scales first as we're still
    // in TWIPs, and might as well minimize the number of conversions.
    Fraction scaleX = Fraction( nOutputWidth, 96 ) * Fraction(1440) /
                                Fraction( nTileWidth);
    Fraction scaleY = Fraction( nOutputHeight, 96 ) * Fraction(1440) /
                                Fraction( nTileHeight);

    // svx seems to be the only component that works natively in
    // 100th mm rather than TWIP. It makes most sense just to
    // convert here and in getDocumentSize, and leave the tiled
    // rendering API working in TWIPs.
    ::tools::Long nTileWidthHMM = convertTwipToMm100( nTileWidth );
    ::tools::Long nTileHeightHMM = convertTwipToMm100( nTileHeight );
    int nTilePosXHMM = convertTwipToMm100( nTilePosX );
    int nTilePosYHMM = convertTwipToMm100( nTilePosY );

    MapMode aMapMode = rDevice.GetMapMode();
    aMapMode.SetMapUnit( MapUnit::Map100thMM );
    aMapMode.SetOrigin( Point( -nTilePosXHMM,
                               -nTilePosYHMM) );
    aMapMode.SetScaleX( scaleX );
    aMapMode.SetScaleY( scaleY );

    rDevice.SetMapMode( aMapMode );

    rDevice.SetOutputSizePixel( Size(nOutputWidth, nOutputHeight) );

    Point aPoint(nTilePosXHMM, nTilePosYHMM);
    Size aSize(nTileWidthHMM, nTileHeightHMM);
    ::tools::Rectangle aRect(aPoint, aSize);

    pViewSh->GetView()->CompleteRedraw(&rDevice, vcl::Region(aRect));

    LokChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

void SdXImpressDocument::selectPart(int nPart, int nSelect)
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    pViewSh->SelectPage(nPart, nSelect);
}

void SdXImpressDocument::moveSelectedParts(int nPosition, bool bDuplicate)
{
    // Duplicating is currently unsupported.
    if (!bDuplicate)
        mpDoc->MovePages(nPosition);
}

OUString SdXImpressDocument::getPartInfo(int nPart)
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return OUString();

    const bool bIsVisible = pViewSh->IsVisible(nPart);
    const bool bIsSelected = pViewSh->IsSelected(nPart);

    OUString aPartInfo = "{ \"visible\": \"" +
        OUString::number(static_cast<unsigned int>(bIsVisible)) +
        "\", \"selected\": \"" +
        OUString::number(static_cast<unsigned int>(bIsSelected)) +
        "\" }";
    return aPartInfo;
}

void SdXImpressDocument::setPart( int nPart )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    pViewSh->SwitchPage( nPart );
}

int SdXImpressDocument::getParts()
{
    // TODO: master pages?
    // Read: drviews1.cxx
    if (!mpDoc)
        return 0;

    return mpDoc->GetSdPageCount(PageKind::Standard);
}

int SdXImpressDocument::getPart()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return 0;

    return pViewSh->GetViewShellBase().getPart();
}

OUString SdXImpressDocument::getPartName( int nPart )
{
    SdPage* pPage = mpDoc->GetSdPage( nPart, PageKind::Standard );
    if (!pPage)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return OUString();
    }

    return pPage->GetName();
}

OUString SdXImpressDocument::getPartHash( int nPart )
{
    SdPage* pPage = mpDoc->GetSdPage( nPart, PageKind::Standard );
    if (!pPage)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return OUString();
    }

    return OUString::number(pPage->GetHashCode());
}

VclPtr<vcl::Window> SdXImpressDocument::getDocWindow()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    VclPtr<vcl::Window> pWindow;
    if (pViewShell)
        pWindow = pViewShell->GetActiveWindow();

    LokChartHelper aChartHelper(pViewShell->GetViewShell());
    VclPtr<vcl::Window> pChartWindow = aChartHelper.GetWindow();
    if (pChartWindow)
        pWindow = pChartWindow;

    return pWindow;
}

void SdXImpressDocument::setPartMode( int nPartMode )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    PageKind aPageKind( PageKind::Standard );
    switch ( nPartMode )
    {
    case LOK_PARTMODE_SLIDES:
        break;
    case LOK_PARTMODE_NOTES:
        aPageKind = PageKind::Notes;
        break;
    }
    pViewSh->SetPageKind( aPageKind );
}

Size SdXImpressDocument::getDocumentSize()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return Size();

    SdrView *pSdrView = pViewSh->GetView();
    if (!pSdrView)
        return Size();

    SdrPageView* pCurPageView = pSdrView->GetSdrPageView();
    if (!pCurPageView)
        return Size();

    Size aSize = pCurPageView->GetPageRect().GetSize();
    // Convert the size in 100th mm to TWIP
    // See paintTile above for further info.
    return Size(convertMm100ToTwip(aSize.getWidth()), convertMm100ToTwip(aSize.getHeight()));
}

void SdXImpressDocument::getPostIts(::tools::JsonWriter& rJsonWriter)
{
    auto commentsNode = rJsonWriter.startNode("comments");
    // Return annotations on master pages too ?
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();
    SdPage* pPage;
    for (sal_uInt16 nPage = 0; nPage < nMaxPages; ++nPage)
    {
        pPage = static_cast<SdPage*>(mpDoc->GetPage(nPage));
        const sd::AnnotationVector& aPageAnnotations = pPage->getAnnotations();

        for (const uno::Reference<office::XAnnotation>& xAnnotation : aPageAnnotations)
        {
            sal_uInt32 nID = sd::getAnnotationId(xAnnotation);
            OString nodeName = "comment" + OString::number(nID);
            auto commentNode = rJsonWriter.startNode(nodeName.getStr());
            rJsonWriter.put("id", nID);
            rJsonWriter.put("author", xAnnotation->getAuthor());
            rJsonWriter.put("dateTime", utl::toISO8601(xAnnotation->getDateTime()));
            uno::Reference<text::XText> xText(xAnnotation->getTextRange());
            rJsonWriter.put("text", xText->getString());
            rJsonWriter.put("parthash", pPage->GetHashCode());
            geometry::RealPoint2D const & rPoint = xAnnotation->getPosition();
            geometry::RealSize2D const & rSize = xAnnotation->getSize();
            ::tools::Rectangle aRectangle(Point(rPoint.X * 100.0, rPoint.Y * 100.0), Size(rSize.Width * 100.0, rSize.Height * 100.0));
            aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
            OString sRectangle = aRectangle.toString();
            rJsonWriter.put("rectangle", sRectangle.getStr());
        }
    }
}

void SdXImpressDocument::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    SolarMutexGuard aGuard;

    if (DrawViewShell* pViewShell = GetViewShell())
    {
        DrawView* pDrawView = pViewShell->GetDrawView();
        for (const beans::PropertyValue& rValue : rArguments)
        {
            if (rValue.Name == ".uno:ShowBorderShadow" && rValue.Value.has<bool>())
                pDrawView->SetPageShadowVisible(rValue.Value.get<bool>());
            else if (rValue.Name == ".uno:Author" && rValue.Value.has<OUString>())
                pDrawView->SetAuthor(rValue.Value.get<OUString>());
        }

        // Disable comments if requested
        SdOptions* pOptions = SD_MOD()->GetSdOptions(mpDoc->GetDocumentType());
        pOptions->SetShowComments(comphelper::LibreOfficeKit::isTiledAnnotations());

        pViewShell->SetRuler(false);
        pViewShell->SetScrollBarsVisible(false);

        if (sd::Window* pWindow = pViewShell->GetActiveWindow())
        {
            // get the full page size in pixels
            pWindow->EnableMapMode();
            Size aSize(pWindow->LogicToPixel(pDrawView->GetSdrPageView()->GetPage()->GetSize()));
            // Disable map mode, so that it's possible to send mouse event
            // coordinates in logic units
            pWindow->EnableMapMode(false);

            // arrange UI elements again with new view size
            pViewShell->GetParentWindow()->SetSizePixel(aSize);
            pViewShell->Resize();
        }

        // Forces all images to be swapped in synchronously, this
        // ensures that images are available when paintTile is called
        // (whereas with async loading images start being loaded after
        //  we have painted the tile, resulting in an invalidate, followed
        //  by the tile being rerendered - which is wasteful and ugly).
        pDrawView->SetSwapAsynchron(false);
    }

    // when the "This document may contain formatting or content that cannot
    // be saved..." dialog appears, it is auto-cancelled with tiled rendering,
    // causing 'Save' being disabled; so let's always save to the original
    // format
    SvtSaveOptions().SetWarnAlienFormat(false);

    if (!getenv("LO_TESTNAME"))
        SvtSlideSorterBarOptions().SetVisibleImpressView(true);
}

void SdXImpressDocument::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;
    SfxLokHelper::postKeyEventAsync(getDocWindow(), nType, nCharCode, nKeyCode);
}

void SdXImpressDocument::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    double fScale = 1.0/TWIPS_PER_PIXEL;

    // check if user hit a chart which is being edited by him
    LokChartHelper aChartHelper(pViewShell->GetViewShell());
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

    const Point aPos(Point(convertTwipToMm100(nX), convertTwipToMm100(nY)));
    LokMouseEventData aMouseEventData(nType, aPos, nCount, MouseEventModifiers::SIMPLECLICK,
                                      nButtons, nModifier);
    SfxLokHelper::postMouseEventAsync(pViewShell->GetActiveWindow(), aMouseEventData);
}

void SdXImpressDocument::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    LokChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setTextSelection(nType, nX, nY))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (nType)
    {
    case LOK_SETTEXTSELECTION_START:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/false, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_END:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/true, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_RESET:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/true, /*bClearMark=*/true);
        break;
    default:
        assert(false);
        break;
    }
}

uno::Reference<datatransfer::XTransferable> SdXImpressDocument::getSelection()
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return uno::Reference<datatransfer::XTransferable>();

    return pViewShell->GetSelectionTransferrable();
}

void SdXImpressDocument::setGraphicSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    double fScale = 1.0/TWIPS_PER_PIXEL;

    LokChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setGraphicSelection(nType, nX, nY, fScale, fScale))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (nType)
    {
    case LOK_SETGRAPHICSELECTION_START:
        pViewShell->SetGraphicMm100Position(/*bStart=*/true, aPoint);
        break;
    case LOK_SETGRAPHICSELECTION_END:
        pViewShell->SetGraphicMm100Position(/*bStart=*/false, aPoint);
        break;
    default:
        assert(false);
        break;
    }
}

void SdXImpressDocument::resetSelection()
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    SdrView* pSdrView = pViewShell->GetView();
    if (!pSdrView)
        return;

    if (pSdrView->IsTextEdit())
    {
        // Reset the editeng selection.
        pSdrView->UnmarkAll();
        // Finish editing.
        pSdrView->SdrEndTextEdit();
    }
    // Reset graphic selection.
    pSdrView->UnmarkAll();
}

void SdXImpressDocument::setClipboard(const uno::Reference<datatransfer::clipboard::XClipboard>& xClipboard)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    pViewShell->GetActiveWindow()->SetClipboard(xClipboard);
}

bool SdXImpressDocument::isMimeTypeSupported()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return false;

    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(pViewShell->GetActiveWindow()));
    return EditEngine::HasValidData(aDataHelper.GetTransferable());
}

PointerStyle SdXImpressDocument::getPointer()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return PointerStyle::Arrow;

    Window* pWindow = pViewShell->GetActiveWindow();
    if (!pWindow)
        return PointerStyle::Arrow;

    return pWindow->GetPointer();
}

uno::Reference< i18n::XForbiddenCharacters > SdXImpressDocument::getForbiddenCharsTable()
{
    uno::Reference< i18n::XForbiddenCharacters > xForb(mxForbiddenCharacters);

    if( !xForb.is() )
        mxForbiddenCharacters = xForb = new SdUnoForbiddenCharsTable( mpDoc );

    return xForb;
}

void SdXImpressDocument::initializeDocument()
{
    if( mbClipBoard )
        return;

    switch( mpDoc->GetPageCount() )
    {
    case 1:
    {
        // nasty hack to detect clipboard document
        mbClipBoard = true;
        break;
    }
    case 0:
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        break;
    }
    }
}

SdrModel& SdXImpressDocument::getSdrModelFromUnoModel() const
{
    OSL_ENSURE(GetDoc(), "No SdrModel in draw/Impress, should not happen");
    return *GetDoc(); // TTTT should be reference
}

void SAL_CALL SdXImpressDocument::dispose()
{
    if( mbDisposed )
        return;

    ::SolarMutexGuard aGuard;

    if( mpDoc )
    {
        EndListening( *mpDoc );
        mpDoc = nullptr;
    }

    // Call the base class dispose() before setting the mbDisposed flag
    // to true.  The reason for this is that if close() has not yet been
    // called this is done in SfxBaseModel::dispose().  At the end of
    // that dispose() is called again.  It is important to forward this
    // second dispose() to the base class, too.
    // As a consequence the following code has to be able to be run twice.
    SfxBaseModel::dispose();
    mbDisposed = true;

    uno::Reference< container::XNameAccess > xLinks( mxLinks );
    if( xLinks.is() )
    {
        uno::Reference< lang::XComponent > xComp( xLinks, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();

        xLinks = nullptr;
    }

    uno::Reference< drawing::XDrawPages > xDrawPagesAccess( mxDrawPagesAccess );
    if( xDrawPagesAccess.is() )
    {
        uno::Reference< lang::XComponent > xComp( xDrawPagesAccess, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();

        xDrawPagesAccess = nullptr;
    }

    uno::Reference< drawing::XDrawPages > xMasterPagesAccess( mxMasterPagesAccess );
    if( xDrawPagesAccess.is() )
    {
        uno::Reference< lang::XComponent > xComp( xMasterPagesAccess, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();

        xDrawPagesAccess = nullptr;
    }

    uno::Reference< container::XNameAccess > xLayerManager( mxLayerManager );
    if( xLayerManager.is() )
    {
        uno::Reference< lang::XComponent > xComp( xLayerManager, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();

        xLayerManager = nullptr;
    }

    uno::Reference< container::XNameContainer > xCustomPresentationAccess( mxCustomPresentationAccess );
    if( xCustomPresentationAccess.is() )
    {
        uno::Reference< lang::XComponent > xComp( xCustomPresentationAccess, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();

        xCustomPresentationAccess = nullptr;
    }

    mxDashTable = nullptr;
    mxGradientTable = nullptr;
    mxHatchTable = nullptr;
    mxBitmapTable = nullptr;
    mxTransGradientTable = nullptr;
    mxMarkerTable = nullptr;
    mxDrawingPool = nullptr;
}


SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  throw()
:   mpModel( &rMyModel)
{
}

SdDrawPagesAccess::~SdDrawPagesAccess() throw()
{
}

// XIndexAccess
sal_Int32 SAL_CALL SdDrawPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
}

uno::Any SAL_CALL SdDrawPagesAccess::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->mpDoc->GetSdPageCount( PageKind::Standard ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->mpDoc->GetSdPage( static_cast<sal_uInt16>(Index), PageKind::Standard );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XNameAccess
uno::Any SAL_CALL SdDrawPagesAccess::getByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    if( !aName.isEmpty() )
    {
        const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nCount; nPage++ )
        {
            SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
            if(nullptr == pPage)
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

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getElementNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();

    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
        *pNames++ = SdDrawPage::getPageApiName( pPage );
    }

    return aNames;
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
        if(nullptr == pPage)
            continue;

        if( aName == SdDrawPage::getPageApiName( pPage ) )
            return true;
    }

    return false;
}

// XElementAccess
uno::Type SAL_CALL SdDrawPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages

/**
 * Creates a new page with model at the specified position.
 * @returns corresponding SdDrawPage
 */
uno::Reference< drawing::XDrawPage > SAL_CALL SdDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("insertNewByIndex");

    if( nullptr == mpModel )
        throw lang::DisposedException();

    if( mpModel->mpDoc )
    {
        SdPage* pPage = mpModel->InsertSdPage( static_cast<sal_uInt16>(nIndex), false );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            return xDrawPage;
        }
    }
    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}

/**
 * Removes the specified SdDrawPage from the model and the internal list. It
 * only works, if there is at least one *normal* page in the model after
 * removing this page.
 */
void SAL_CALL SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel || mpModel->mpDoc == nullptr )
        throw lang::DisposedException();

    SdDrawDocument& rDoc = *mpModel->mpDoc;

    sal_uInt16 nPageCount = rDoc.GetSdPageCount( PageKind::Standard );
    if( nPageCount > 1 )
    {
        // get pPage from xPage and determine the Id (nPos ) afterwards
        SdDrawPage* pSvxPage = comphelper::getUnoTunnelImplementation<SdDrawPage>( xPage );
        if( pSvxPage )
        {
            SdPage* pPage = static_cast<SdPage*>(pSvxPage->GetSdrPage());
            if(pPage && ( pPage->GetPageKind() == PageKind::Standard ) )
            {
                sal_uInt16 nPage = pPage->GetPageNum();

                SdPage* pNotesPage = static_cast< SdPage* >( rDoc.GetPage( nPage+1 ) );

                bool bUndo = rDoc.IsUndoEnabled();
                if( bUndo )
                {
                    // Add undo actions and delete the pages.  The order of adding
                    // the undo actions is important.
                    rDoc.BegUndo( SdResId( STR_UNDO_DELETEPAGES ) );
                    rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));
                    rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
                }

                rDoc.RemovePage( nPage ); // the page
                rDoc.RemovePage( nPage ); // the notes page

                if( bUndo )
                {
                    rDoc.EndUndo();
                }
                else
                {
                    delete pNotesPage;
                    delete pPage;
                }
            }
        }
    }

    mpModel->SetModified();
}

// XServiceInfo

OUString SAL_CALL SdDrawPagesAccess::getImplementationName(  )
{
    return "SdDrawPagesAccess";
}

sal_Bool SAL_CALL SdDrawPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getSupportedServiceNames(  )
{
    return { "com.sun.star.drawing.DrawPages" };
}

// XComponent
void SAL_CALL SdDrawPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdDrawPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdDrawPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}


SdMasterPagesAccess::SdMasterPagesAccess( SdXImpressDocument& rMyModel ) throw()
:   mpModel(&rMyModel)
{
}

SdMasterPagesAccess::~SdMasterPagesAccess() throw()
{
}

// XComponent
void SAL_CALL SdMasterPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdMasterPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdMasterPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel->mpDoc )
        throw lang::DisposedException();

    return mpModel->mpDoc->GetMasterSdPageCount(PageKind::Standard);
}

/**
 * Provides a drawing::XDrawPage interface for accessing the Masterpage at the
 * specified position in the model.
 */
uno::Any SAL_CALL SdMasterPagesAccess::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SdMasterPagesAccess::getByIndex");

    if( nullptr == mpModel )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->mpDoc->GetMasterSdPageCount( PageKind::Standard ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->mpDoc->GetMasterSdPage( static_cast<sal_uInt16>(Index), PageKind::Standard );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XElementAccess
uno::Type SAL_CALL SdMasterPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SdMasterPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages
uno::Reference< drawing::XDrawPage > SAL_CALL SdMasterPagesAccess::insertNewByIndex( sal_Int32 nInsertPos )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xDrawPage;

    SdDrawDocument* pDoc = mpModel->mpDoc;
    if( pDoc )
    {
        // calculate internal index and check for range errors
        const sal_Int32 nMPageCount = pDoc->GetMasterPageCount();
        nInsertPos = nInsertPos * 2 + 1;
        if( nInsertPos < 0 || nInsertPos > nMPageCount )
            nInsertPos = nMPageCount;

        // now generate a unique name for the new masterpage
        const OUString aStdPrefix( SdResId(STR_LAYOUT_DEFAULT_NAME) );
        OUString aPrefix( aStdPrefix );

        bool bUnique = true;

        std::vector<OUString> aPageNames;
        for (sal_Int32 nMaster = 1; nMaster < nMPageCount; ++nMaster)
        {
            const SdPage* pPage = static_cast<const SdPage*>(pDoc->GetMasterPage(static_cast<sal_uInt16>(nMaster)));
            if (!pPage)
                continue;
            aPageNames.push_back(pPage->GetName());
            if (aPageNames.back() == aPrefix)
                bUnique = false;
        }

        sal_Int32 i = 0;
        while (!bUnique)
        {
            aPrefix = aStdPrefix + " " + OUString::number(++i);
            bUnique = std::find(aPageNames.begin(), aPageNames.end(), aPrefix) == aPageNames.end();
        }

        OUString aLayoutName = aPrefix + SD_LT_SEPARATOR STR_LAYOUT_OUTLINE;

        // create styles
        static_cast<SdStyleSheetPool*>(pDoc->GetStyleSheetPool())->CreateLayoutStyleSheets( aPrefix );

        // get the first page for initial size and border settings
        SdPage* pPage = mpModel->mpDoc->GetSdPage( sal_uInt16(0), PageKind::Standard );
        SdPage* pRefNotesPage = mpModel->mpDoc->GetSdPage( sal_uInt16(0), PageKind::Notes);

        // create and insert new draw masterpage
        SdPage* pMPage = mpModel->mpDoc->AllocSdPage(true);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLeftBorder(),
                           pPage->GetUpperBorder(),
                           pPage->GetRightBorder(),
                           pPage->GetLowerBorder() );
        pMPage->SetLayoutName( aLayoutName );
        pDoc->InsertMasterPage(pMPage,  static_cast<sal_uInt16>(nInsertPos));

        {
            // ensure default MasterPage fill
            pMPage->EnsureMasterPageDefaultBackground();
        }

        xDrawPage.set( pMPage->getUnoPage(), uno::UNO_QUERY );

        // create and insert new notes masterpage
        SdPage* pMNotesPage = mpModel->mpDoc->AllocSdPage(true);
        pMNotesPage->SetSize( pRefNotesPage->GetSize() );
        pMNotesPage->SetPageKind(PageKind::Notes);
        pMNotesPage->SetBorder( pRefNotesPage->GetLeftBorder(),
                                pRefNotesPage->GetUpperBorder(),
                                pRefNotesPage->GetRightBorder(),
                                pRefNotesPage->GetLowerBorder() );
        pMNotesPage->SetLayoutName( aLayoutName );
        pDoc->InsertMasterPage(pMNotesPage,  static_cast<sal_uInt16>(nInsertPos) + 1);
        pMNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);
        mpModel->SetModified();
    }

    return xDrawPage;
}

/**
 * Removes the specified SdMasterPage from the model and the internal list. It
 * only works, if there is no *normal* page using this page as MasterPage in
 * the model.
 */
void SAL_CALL SdMasterPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel || mpModel->mpDoc == nullptr )
        throw lang::DisposedException();

    SdMasterPage* pSdPage = comphelper::getUnoTunnelImplementation<SdMasterPage>( xPage );
    if(pSdPage == nullptr)
        return;

    SdPage* pPage = dynamic_cast< SdPage* > (pSdPage->GetSdrPage());

    DBG_ASSERT( pPage && pPage->IsMasterPage(), "SdMasterPage is not masterpage?");

    if( !pPage || !pPage->IsMasterPage() || (mpModel->mpDoc->GetMasterPageUserCount(pPage) > 0))
        return; //Todo: this should be excepted

    // only standard pages can be removed directly
    if( pPage->GetPageKind() != PageKind::Standard )
        return;

    sal_uInt16 nPage = pPage->GetPageNum();

    SdDrawDocument& rDoc = *mpModel->mpDoc;

    SdPage* pNotesPage = static_cast< SdPage* >( rDoc.GetMasterPage( nPage+1 ) );

    bool bUndo = rDoc.IsUndoEnabled();
    if( bUndo )
    {
        // Add undo actions and delete the pages.  The order of adding
        // the undo actions is important.
        rDoc.BegUndo( SdResId( STR_UNDO_DELETEPAGES ) );
        rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));
        rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
    }

    // remove both pages
    rDoc.RemoveMasterPage( nPage );
    rDoc.RemoveMasterPage( nPage );

    if( bUndo )
    {
        rDoc.EndUndo();
    }
    else
    {
        delete pNotesPage;
        delete pPage;
    }
}

// XServiceInfo

OUString SAL_CALL SdMasterPagesAccess::getImplementationName(  )
{
    return "SdMasterPagesAccess";
}

sal_Bool SAL_CALL SdMasterPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdMasterPagesAccess::getSupportedServiceNames(  )
{
    return { "com.sun.star.drawing.MasterPages" };
}


SdDocLinkTargets::SdDocLinkTargets( SdXImpressDocument& rMyModel ) throw()
: mpModel( &rMyModel )
{
}

SdDocLinkTargets::~SdDocLinkTargets() throw()
{
}

// XComponent
void SAL_CALL SdDocLinkTargets::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdDocLinkTargets::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdDocLinkTargets::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XNameAccess
uno::Any SAL_CALL SdDocLinkTargets::getByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    SdPage* pPage = FindPage( aName );

    if( pPage == nullptr )
        throw container::NoSuchElementException();

    uno::Any aAny;

    uno::Reference< beans::XPropertySet > xProps( pPage->getUnoPage(), uno::UNO_QUERY );
    if( xProps.is() )
        aAny <<= xProps;

    return aAny;
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getElementNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == nullptr )
    {
        return { };
    }

    if( pDoc->GetDocumentType() == DocumentType::Draw )
    {
        const sal_uInt16 nMaxPages = pDoc->GetSdPageCount( PageKind::Standard );
        const sal_uInt16 nMaxMasterPages = pDoc->GetMasterSdPageCount( PageKind::Standard );

        uno::Sequence< OUString > aSeq( nMaxPages + nMaxMasterPages );
        OUString* pStr = aSeq.getArray();

        sal_uInt16 nPage;
        // standard pages
        for( nPage = 0; nPage < nMaxPages; nPage++ )
            *pStr++ = pDoc->GetSdPage( nPage, PageKind::Standard )->GetName();

        // master pages
        for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
            *pStr++ = pDoc->GetMasterSdPage( nPage, PageKind::Standard )->GetName();
        return aSeq;
    }
    else
    {
        const sal_uInt16 nMaxPages = pDoc->GetPageCount();
        const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

        uno::Sequence< OUString > aSeq( nMaxPages + nMaxMasterPages );
        OUString* pStr = aSeq.getArray();

        sal_uInt16 nPage;
        // standard pages
        for( nPage = 0; nPage < nMaxPages; nPage++ )
            *pStr++ = static_cast<SdPage*>(pDoc->GetPage( nPage ))->GetName();

        // master pages
        for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
            *pStr++ = static_cast<SdPage*>(pDoc->GetMasterPage( nPage ))->GetName();
        return aSeq;
    }
}

sal_Bool SAL_CALL SdDocLinkTargets::hasByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return FindPage( aName ) != nullptr;
}

// container::XElementAccess
uno::Type SAL_CALL SdDocLinkTargets::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL SdDocLinkTargets::hasElements()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->GetDoc() != nullptr;
}

SdPage* SdDocLinkTargets::FindPage( const OUString& rName ) const
{
    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == nullptr )
        return nullptr;

    const sal_uInt16 nMaxPages = pDoc->GetPageCount();
    const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

    sal_uInt16 nPage;
    SdPage* pPage;

    const bool bDraw = pDoc->GetDocumentType() == DocumentType::Draw;

    // standard pages
    for( nPage = 0; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*>(pDoc->GetPage( nPage ));
        if( (pPage->GetName() == rName) && (!bDraw || (pPage->GetPageKind() == PageKind::Standard)) )
            return pPage;
    }

    // master pages
    for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
    {
        pPage = static_cast<SdPage*>(pDoc->GetMasterPage( nPage ));
        if( (pPage->GetName() == rName) && (!bDraw || (pPage->GetPageKind() == PageKind::Standard)) )
            return pPage;
    }

    return nullptr;
}

// XServiceInfo
OUString SAL_CALL SdDocLinkTargets::getImplementationName()
{
    return "SdDocLinkTargets";
}

sal_Bool SAL_CALL SdDocLinkTargets::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getSupportedServiceNames()
{
    return { "com.sun.star.document.LinkTargets" };
}

rtl::Reference< SdXImpressDocument > SdXImpressDocument::GetModel( SdDrawDocument const & rDocument )
{
    rtl::Reference< SdXImpressDocument > xRet;
    ::sd::DrawDocShell* pDocShell(rDocument.GetDocSh());
    if( pDocShell )
    {
        uno::Reference<frame::XModel> xModel(pDocShell->GetModel());

        xRet.set( dynamic_cast< SdXImpressDocument* >( xModel.get() ) );
    }

    return xRet;
}

void NotifyDocumentEvent( SdDrawDocument const & rDocument, const OUString& rEventName )
{
    rtl::Reference< SdXImpressDocument > xModel( SdXImpressDocument::GetModel( rDocument ) );

    if( xModel.is() )
    {
        uno::Reference< uno::XInterface > xSource( static_cast<uno::XWeak*>( xModel.get() ) );
        css::document::EventObject aEvent( xSource, rEventName );
        xModel->notifyEvent(aEvent );
    }
}

void NotifyDocumentEvent( SdDrawDocument const & rDocument, const OUString& rEventName, const uno::Reference< uno::XInterface >& xSource )
{
    rtl::Reference< SdXImpressDocument > xModel( SdXImpressDocument::GetModel( rDocument ) );

    if( xModel.is() )
    {
        css::document::EventObject aEvent( xSource, rEventName );
        xModel->notifyEvent(aEvent );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
