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

#include "viscrs.hxx"
#include <sfx2/frame.hxx>
#include <sfx2/printer.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include <docsh.hxx>
#include <rubylist.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <unotxvw.hxx>
#include <unodispatch.hxx>
#include <unomap.hxx>
#include <unostyle.hxx>
#include <unoprnms.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <unomod.hxx>
#include <unoframe.hxx>
#include <unocrsr.hxx>
#include <wrtsh.hxx>
#include <unotbl.hxx>
#include <svx/fmshell.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/pbinitem.hxx>
#include <pagedesc.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <frmatr.hxx>
#include <osl/mutex.hxx>
#include <IMark.hxx>
#include <unotxdoc.hxx>
#include <unodraw.hxx>
#include <svx/unoshape.hxx>
#include <svx/svdpagv.hxx>
#include <swerror.h>
#include <shellio.hxx>
#include <ndtxt.hxx>
#include <SwStyleNameMapper.hxx>
#include <crsskip.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/ShapeCollection.hpp>
#include <editeng/outliner.hxx>
#include <editeng/editview.hxx>
#include <unoparagraph.hxx>
#include <unocrsrhelper.hxx>
#include <unotextrange.hxx>
#include <sfx2/docfile.hxx>
#include <calbck.hxx>
#include "swdtflvr.hxx"
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::frame;

using ::com::sun::star::util::URL;
using comphelper::HelperBaseNoState;

SwXTextView::SwXTextView(SwView* pSwView) :
    SfxBaseController(pSwView),
    m_SelChangedListeners(m_aMutex),
    m_pView(pSwView),
    m_pPropSet( aSwMapProvider.GetPropertySet( PROPERTY_MAP_TEXT_VIEW ) ),
    mxViewSettings(),
    mxTextViewCursor()
{

}

SwXTextView::~SwXTextView()
{
    Invalidate();
}

void SwXTextView::Invalidate()
{
    if(mxViewSettings.is())
    {
        HelperBaseNoState *pSettings = static_cast < HelperBaseNoState * > ( mxViewSettings.get() );
        static_cast < SwXViewSettings* > ( pSettings )->Invalidate();
        mxViewSettings.clear();
    }
    if(mxTextViewCursor.is())
    {
        text::XTextViewCursor* pCursor = mxTextViewCursor.get();
        static_cast<SwXTextViewCursor*>(pCursor)->Invalidate();
        mxTextViewCursor.clear();
    }

    m_refCount++; //prevent second d'tor call

    {
        uno::Reference<uno::XInterface> const xInt(static_cast<
                cppu::OWeakObject*>(static_cast<SfxBaseController*>(this)));
        lang::EventObject aEvent(xInt);
        m_SelChangedListeners.disposeAndClear(aEvent);
    }

    m_refCount--;
    m_pView = nullptr;
}

Sequence< uno::Type > SAL_CALL SwXTextView::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aBaseTypes = SfxBaseController::getTypes();

    long nIndex = aBaseTypes.getLength();
    aBaseTypes.realloc(
        aBaseTypes.getLength() + 8 );

    uno::Type* pBaseTypes = aBaseTypes.getArray();
    pBaseTypes[nIndex++] = cppu::UnoType<XSelectionSupplier>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XServiceInfo>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XFormLayerAccess>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XTextViewCursorSupplier>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XViewSettingsSupplier>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XRubySelection>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<XPropertySet>::get();
    pBaseTypes[nIndex++] = cppu::UnoType<datatransfer::XTransferableSupplier>::get();
    return aBaseTypes;
}

Sequence< sal_Int8 > SAL_CALL SwXTextView::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SwXTextView::acquire(  )throw()
{
    SfxBaseController::acquire();
}

void SAL_CALL SwXTextView::release(  )throw()
{
    SfxBaseController::release();
}

uno::Any SAL_CALL SwXTextView::queryInterface( const uno::Type& aType )
    throw (RuntimeException, std::exception)
{
    uno::Any aRet;
    if(aType == cppu::UnoType<view::XSelectionSupplier>::get())
    {
        uno::Reference<view::XSelectionSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<lang::XServiceInfo>::get())
    {
        uno::Reference<lang::XServiceInfo> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<view::XControlAccess>::get())
    {
        uno::Reference<view::XControlAccess> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<view::XFormLayerAccess>::get())
    {
        uno::Reference<view::XFormLayerAccess> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<text::XTextViewCursorSupplier>::get())
    {
        uno::Reference<text::XTextViewCursorSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<view::XViewSettingsSupplier>::get())
    {
        uno::Reference<view::XViewSettingsSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<XRubySelection>::get())
    {
        uno::Reference<XRubySelection> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<XPropertySet>::get())
    {
        uno::Reference<XPropertySet> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else if(aType == cppu::UnoType<datatransfer::XTransferableSupplier>::get())
    {
        uno::Reference<datatransfer::XTransferableSupplier> xRet = this;
        aRet.setValue(&xRet, aType);
    }
    else
        aRet = SfxBaseController::queryInterface(aType);
    return aRet;
}

sal_Bool SwXTextView::select(const uno::Any& aInterface)
    throw (lang::IllegalArgumentException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    uno::Reference< uno::XInterface >  xInterface;
    if (!GetView() || !(aInterface >>= xInterface))
    {
        return sal_False;
    }

    SwWrtShell& rSh = GetView()->GetWrtShell();
    SwDoc* pDoc = GetView()->GetDocShell()->GetDoc();
    std::vector<SdrObject *> sdrObjects;
    uno::Reference<awt::XControlModel> const xCtrlModel(xInterface,
            UNO_QUERY);
    if (xCtrlModel.is())
    {
        uno::Reference<awt::XControl> xControl;
        SdrObject *const pSdrObject = GetControl(xCtrlModel, xControl);
        if (pSdrObject) // hmm... needs view to verify it's in right doc...
        {
            sdrObjects.push_back(pSdrObject);
        }
    }
    else
    {
        SwPaM * pPaM(nullptr);
        std::pair<OUString, FlyCntType> frame;
        OUString tableName;
        SwUnoTableCursor const* pTableCursor(nullptr);
        ::sw::mark::IMark const* pMark(nullptr);
        SwUnoCursorHelper::GetSelectableFromAny(xInterface, *pDoc,
                pPaM, frame, tableName, pTableCursor, pMark, sdrObjects);
        if (pPaM)
        {
            rSh.EnterStdMode();
            rSh.SetSelection(*pPaM);
            // the pPaM has been copied - delete it
            while (pPaM->GetNext() != pPaM)
                delete pPaM->GetNext();
            delete pPaM;
            return sal_True;
        }
        else if (!frame.first.isEmpty())
        {
            bool const bSuccess(rSh.GotoFly(frame.first, frame.second));
            if (bSuccess)
            {
                rSh.HideCursor();
                rSh.EnterSelFrameMode();
            }
            return sal_True;
        }
        else if (!tableName.isEmpty())
        {
            rSh.EnterStdMode();
            rSh.GotoTable(tableName);
            return sal_True;
        }
        else if (pTableCursor)
        {
            UnoActionRemoveContext const aContext(*pTableCursor);
            rSh.EnterStdMode();
            rSh.SetSelection(*pTableCursor);
            return sal_True;
        }
        else if (pMark)
        {
            rSh.EnterStdMode();
            rSh.GotoMark(pMark);
            return sal_True;
        }
        // sdrObjects handled below
    }
    bool bRet(false);
    if (sdrObjects.size())
    {

        SdrView *const pDrawView = rSh.GetDrawView();
        SdrPageView *const pPV = pDrawView->GetSdrPageView();

        pDrawView->SdrEndTextEdit();
        pDrawView->UnmarkAll();

        for (size_t i = 0; i < sdrObjects.size(); ++i)
        {
            SdrObject *const pSdrObject(sdrObjects[i]);
            // GetSelectableFromAny did not check pSdrObject is in right doc!
            if (pPV && pSdrObject->GetPage() == pPV->GetPage())
            {
                pDrawView->MarkObj(pSdrObject, pPV);
                bRet = true;
            }
        }
    }
    return bRet;
}

uno::Any SwXTextView::getSelection()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  aRef;
    if(GetView())
    {
        //force immediat shell update
        m_pView->StopShellTimer();
        //Generating an interface from the current selection.
        SwWrtShell& rSh = m_pView->GetWrtShell();
        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            case SHELL_MODE_TABLE_TEXT      :
            {
                if(rSh.GetTableCursor())
                {
                    OSL_ENSURE(rSh.GetTableFormat(), "not a table format?");
                    uno::Reference< text::XTextTableCursor >  xCursor = new SwXTextTableCursor(*rSh.GetTableFormat(),
                                                    rSh.GetTableCursor());
                    aRef.set(xCursor, uno::UNO_QUERY);
                    break;
                }
                SAL_FALLTHROUGH;
                    // without a table selection the text will be delivered
            }
            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT:
            case SHELL_MODE_TEXT            :
            {
                uno::Reference< container::XIndexAccess > xPos = SwXTextRanges::Create(rSh.GetCursor());
                aRef.set(xPos, uno::UNO_QUERY);
            }
            break;
            case SHELL_MODE_FRAME           :
            {
                SwFrameFormat *const pFormat = rSh.GetFlyFrameFormat();
                if (pFormat)
                {
                    aRef = SwXTextFrame::CreateXTextFrame(
                            *pFormat->GetDoc(), pFormat);
                }
            }
            break;
            case SHELL_MODE_GRAPHIC         :
            {
                SwFrameFormat *const pFormat = rSh.GetFlyFrameFormat();
                if (pFormat)
                {
                    aRef = SwXTextGraphicObject::CreateXTextGraphicObject(
                            *pFormat->GetDoc(), pFormat);
                }
            }
            break;
            case SHELL_MODE_OBJECT          :
            {
                SwFrameFormat *const pFormat = rSh.GetFlyFrameFormat();
                if (pFormat)
                {
                    aRef = SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                            *pFormat->GetDoc(), pFormat);
                }
            }
            break;
            case SHELL_MODE_DRAW            :
            case SHELL_MODE_DRAW_CTRL       :
            case SHELL_MODE_DRAW_FORM       :
            case SHELL_MODE_DRAWTEXT        :
            case SHELL_MODE_BEZIER          :
            {
                uno::Reference< drawing::XDrawPageSupplier >  xPageSupp;
                uno::Reference< frame::XModel > xModel = m_pView->GetDocShell()->GetBaseModel();
                uno::Reference< lang::XUnoTunnel > xModelTunnel(xModel, uno::UNO_QUERY);

                uno::Reference< drawing::XShapes >  xShCol = drawing::ShapeCollection::create(
                        comphelper::getProcessComponentContext());

                const SdrMarkList& rMarkList = rSh.GetDrawView()->GetMarkedObjectList();
                for(size_t i = 0; i < rMarkList.GetMarkCount(); ++i)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                    uno::Reference< uno::XInterface >  xInt = SwFmDrawPage::GetInterface( pObj );
                    uno::Reference< drawing::XShape >  xShape(xInt, uno::UNO_QUERY);
                    xShCol->add(xShape);
                }
                aRef.set(xShCol, uno::UNO_QUERY);
            }
            break;
            default:;//prevent warning
        }
    }
    uno::Any aRet(&aRef, cppu::UnoType<uno::XInterface>::get());
    return aRet;
}

void SwXTextView::addSelectionChangeListener(
                                    const uno::Reference< view::XSelectionChangeListener > & rxListener)
                                    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    m_SelChangedListeners.addInterface(rxListener);
}

void SwXTextView::removeSelectionChangeListener(
                                        const uno::Reference< view::XSelectionChangeListener > & rxListener)
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    m_SelChangedListeners.removeInterface(rxListener);
}

SdrObject* SwXTextView::GetControl(
        const uno::Reference< awt::XControlModel > & xModel,
        uno::Reference< awt::XControl >& xToFill  )
{
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : nullptr;
    SdrView* pDrawView = pView2 ? pView2->GetDrawView() : nullptr;
    vcl::Window* pWindow = pView2 ? pView2->GetWrtShell().GetWin() : nullptr;

    OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::GetControl: how could I?" );

    SdrObject* pControl = nullptr;
    if ( pFormShell && pDrawView && pWindow )
        pControl = pFormShell->GetFormControl( xModel, *pDrawView, *pWindow, xToFill );
    return pControl;
}

uno::Reference< awt::XControl >  SwXTextView::getControl(const uno::Reference< awt::XControlModel > & xModel)
        throw( container::NoSuchElementException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< awt::XControl >  xRet;
    GetControl(xModel, xRet);
    return xRet;
}

uno::Reference< form::runtime::XFormController > SAL_CALL SwXTextView::getFormController( const uno::Reference< form::XForm >& _Form ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : nullptr;
    SdrView* pDrawView = pView2 ? pView2->GetDrawView() : nullptr;
    vcl::Window* pWindow = pView2 ? pView2->GetWrtShell().GetWin() : nullptr;
    OSL_ENSURE( pFormShell && pDrawView && pWindow, "SwXTextView::getFormController: how could I?" );

    uno::Reference< form::runtime::XFormController > xController;
    if ( pFormShell && pDrawView && pWindow )
        xController = FmFormShell::GetFormController( _Form, *pDrawView, *pWindow );
    return xController;
}

sal_Bool SAL_CALL SwXTextView::isFormDesignMode(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : nullptr;
    return !pFormShell || pFormShell->IsDesignMode();
}

void SAL_CALL SwXTextView::setFormDesignMode( sal_Bool _DesignMode ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwView* pView2 = GetView();
    FmFormShell* pFormShell = pView2 ? pView2->GetFormShell() : nullptr;
    if ( pFormShell )
        pFormShell->SetDesignMode( _DesignMode );
}

uno::Reference< text::XTextViewCursor >  SwXTextView::getViewCursor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(GetView())
    {
        if(!mxTextViewCursor.is())
        {
            mxTextViewCursor = new SwXTextViewCursor(GetView());
        }
        return mxTextViewCursor;
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySet >  SwXTextView::getViewSettings() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if(!mxViewSettings.is())
        {
            mxViewSettings = static_cast < HelperBaseNoState * > ( new SwXViewSettings( false, m_pView ) );
        }
    }
    else
        throw uno::RuntimeException();
    return mxViewSettings;
}

Sequence< Sequence< PropertyValue > > SwXTextView::getRubyList( sal_Bool /*bAutomatic*/ )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!GetView())
        throw RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    ShellModes  eSelMode = m_pView->GetShellMode();
    if (eSelMode != SHELL_MODE_LIST_TEXT      &&
        eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
        eSelMode != SHELL_MODE_TABLE_TEXT      &&
        eSelMode != SHELL_MODE_TEXT           )
        return Sequence< Sequence< PropertyValue > > ();

    SwRubyList aList;

    const sal_uInt16 nCount = SwDoc::FillRubyList( *rSh.GetCursor(), aList, 0 );
    Sequence< Sequence< PropertyValue > > aRet(nCount);
    Sequence< PropertyValue >* pRet = aRet.getArray();
    OUString aString;
    for(sal_uInt16 n = 0; n < nCount; n++)
    {
        const SwRubyListEntry* pEntry = aList[n].get();

        const OUString& rEntryText = pEntry->GetText();
        const SwFormatRuby& rAttr = pEntry->GetRubyAttr();

        pRet[n].realloc(5);
        PropertyValue* pValues = pRet[n].getArray();
        pValues[0].Name = UNO_NAME_RUBY_BASE_TEXT;
        pValues[0].Value <<= rEntryText;
        pValues[1].Name = UNO_NAME_RUBY_TEXT;
        pValues[1].Value <<= rAttr.GetText();
        pValues[2].Name = UNO_NAME_RUBY_CHAR_STYLE_NAME;
        SwStyleNameMapper::FillProgName(rAttr.GetCharFormatName(), aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
        pValues[2].Value <<= aString;
        pValues[3].Name = UNO_NAME_RUBY_ADJUST;
        pValues[3].Value <<= (sal_Int16)rAttr.GetAdjustment();
        pValues[4].Name = UNO_NAME_RUBY_IS_ABOVE;
        sal_Bool bVal = !rAttr.GetPosition();
        pValues[4].Value.setValue(&bVal, cppu::UnoType<bool>::get());
    }
    return aRet;
}

void SAL_CALL SwXTextView::setRubyList(
    const Sequence< Sequence< PropertyValue > >& rRubyList, sal_Bool /*bAutomatic*/ )
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if(!GetView() || !rRubyList.getLength())
        throw RuntimeException();
    SwWrtShell& rSh = m_pView->GetWrtShell();
    ShellModes  eSelMode = m_pView->GetShellMode();
    if (eSelMode != SHELL_MODE_LIST_TEXT      &&
        eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
        eSelMode != SHELL_MODE_TABLE_TEXT      &&
        eSelMode != SHELL_MODE_TEXT           )
        throw RuntimeException();

    SwRubyList aList;

    const Sequence<PropertyValue>* pRubyList = rRubyList.getConstArray();
    for(sal_Int32 nPos = 0; nPos < rRubyList.getLength(); nPos++)
    {
        std::unique_ptr<SwRubyListEntry> pEntry(new SwRubyListEntry);
        const PropertyValue* pProperties = pRubyList[nPos].getConstArray();
        OUString sTmp;
        for(sal_Int32 nProp = 0; nProp < pRubyList[nPos].getLength(); nProp++)
        {
            if(pProperties[nProp].Name == UNO_NAME_RUBY_BASE_TEXT)
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->SetText(sTmp);
            }
            else if(pProperties[nProp].Name == UNO_NAME_RUBY_TEXT)
            {
                pProperties[nProp].Value >>= sTmp;
                pEntry->GetRubyAttr().SetText(sTmp);
            }
            else if(pProperties[nProp].Name == UNO_NAME_RUBY_CHAR_STYLE_NAME)
            {
                if((pProperties[nProp].Value >>= sTmp))
                {
                    OUString sName;
                    SwStyleNameMapper::FillUIName(sTmp, sName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, true );
                    const sal_uInt16 nPoolId = sName.isEmpty() ? 0
                        : SwStyleNameMapper::GetPoolIdFromUIName(sName,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );

                    pEntry->GetRubyAttr().SetCharFormatName( sName );
                    pEntry->GetRubyAttr().SetCharFormatId( nPoolId );
                }
            }
            else if(pProperties[nProp].Name == UNO_NAME_RUBY_ADJUST)
            {
                sal_Int16 nTmp = 0;
                if((pProperties[nProp].Value >>= nTmp))
                    pEntry->GetRubyAttr().SetAdjustment(nTmp);
            }
            else if(pProperties[nProp].Name == UNO_NAME_RUBY_IS_ABOVE)
            {
                bool bValue = !pProperties[nProp].Value.hasValue() ||
                    *static_cast<sal_Bool const *>(pProperties[nProp].Value.getValue());
                pEntry->GetRubyAttr().SetPosition(bValue ? 0 : 1);
            }
        }
        aList.insert(aList.begin() + nPos, std::move(pEntry));
    }
    SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
    pDoc->SetRubyList( *rSh.GetCursor(), aList, 0 );
}

SfxObjectShellLock SwXTextView::BuildTmpSelectionDoc()
{
    SwWrtShell& rOldSh = m_pView->GetWrtShell();
    SfxPrinter *pPrt = rOldSh.getIDocumentDeviceAccess().getPrinter( false );
    SwDocShell* pDocSh;
    SfxObjectShellLock xDocSh( pDocSh = new SwDocShell( /*pPrtDoc, */SfxObjectCreateMode::STANDARD ) );
    xDocSh->DoInitNew();
    SwDoc *const pTempDoc( pDocSh->GetDoc() );
    // #i103634#, #i112425#: do not expand numbering and fields on PDF export
    pTempDoc->SetClipBoard(true);
    rOldSh.FillPrtDoc(pTempDoc,  pPrt);
    SfxViewFrame* pDocFrame = SfxViewFrame::LoadHiddenDocument( *xDocSh, 0 );
    SwView* pDocView = static_cast<SwView*>( pDocFrame->GetViewShell() );
    pDocView->AttrChangedNotify( &pDocView->GetWrtShell() );//So that SelectShell is called.
    SwWrtShell* pSh = pDocView->GetWrtShellPtr();

    IDocumentDeviceAccess& rIDDA = pSh->getIDocumentDeviceAccess();
    SfxPrinter* pTempPrinter = rIDDA.getPrinter( true );

    const SwPageDesc& rCurPageDesc = rOldSh.GetPageDesc(rOldSh.GetCurPageDesc());

    IDocumentDeviceAccess& rIDDA_old = rOldSh.getIDocumentDeviceAccess();

    if( rIDDA_old.getPrinter( false ) )
    {
        rIDDA.setJobsetup( *rIDDA_old.getJobsetup() );
        //#69563# if it isn't the same printer then the pointer has been invalidated!
        pTempPrinter = rIDDA.getPrinter( true );
    }

    pTempPrinter->SetPaperBin(rCurPageDesc.GetMaster().GetPaperBin().GetValue());

    return xDocSh;
}

void SwXTextView::NotifySelChanged()
{
    OSL_ENSURE( m_pView, "view is missing" );

    uno::Reference<uno::XInterface> const xInt(
        static_cast<cppu::OWeakObject*>(static_cast<SfxBaseController*>(this)));

    lang::EventObject const aEvent(xInt);
    m_SelChangedListeners.notifyEach(
            &view::XSelectionChangeListener::selectionChanged, aEvent);
}

namespace {
    struct DispatchListener
    {
        URL const & m_rURL;
        Sequence<PropertyValue> const& m_rSeq;
        explicit DispatchListener(URL const& rURL,
                Sequence<PropertyValue> const& rSeq)
            : m_rURL(rURL), m_rSeq(rSeq) { }
        void operator()(uno::Reference<XDispatch> const & xListener) const
        {
            xListener->dispatch(m_rURL, m_rSeq);
        }
    };
}

void SwXTextView::NotifyDBChanged()
{
    URL aURL;
    aURL.Complete = OUString::createFromAscii(SwXDispatch::GetDBChangeURL());

    m_SelChangedListeners.forEach<XDispatch>(
            DispatchListener(aURL, Sequence<PropertyValue>(0)));
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL SwXTextView::getPropertySetInfo(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference< XPropertySetInfo > aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL SwXTextView::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException();
    else if (pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException();
    else
    {
        switch (pEntry->nWID)
        {
            case WID_IS_HIDE_SPELL_MARKS :
                // deprecated #i91949
            break;
            case WID_IS_CONSTANT_SPELLCHECK :
            {
                bool bVal = false;
                const SwViewOption *pOpt = m_pView->GetWrtShell().GetViewOptions();
                if (!pOpt || !(rValue >>= bVal))
                    throw RuntimeException();
                SwViewOption aNewOpt( *pOpt );
                if (pEntry->nWID == WID_IS_CONSTANT_SPELLCHECK)
                    aNewOpt.SetOnlineSpell(bVal);
                m_pView->GetWrtShell().ApplyViewOptions( aNewOpt );
            }
            break;
            default :
                OSL_FAIL("unknown WID");
        }
    }
}

uno::Any SAL_CALL SwXTextView::getPropertyValue(
        const OUString& rPropertyName )
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;

    Any aRet;

    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException();
    else
    {
        sal_Int16 nWID = pEntry->nWID;
        switch (nWID)
        {
            case WID_PAGE_COUNT :
            case WID_LINE_COUNT :
            {
                // format document completely in order to get meaningful
                // values for page count and line count
                m_pView->GetWrtShell().CalcLayout();

                sal_Int32 nCount = -1;
                if (nWID == WID_PAGE_COUNT)
                    nCount = m_pView->GetWrtShell().GetPageCount();
                else // WID_LINE_COUNT
                    nCount = m_pView->GetWrtShell().GetLineCount( false /*of whole document*/ );
                aRet <<= nCount;
            }
            break;
            case WID_IS_HIDE_SPELL_MARKS :
                // deprecated #i91949
            break;
            case WID_IS_CONSTANT_SPELLCHECK :
            {
                const SwViewOption *pOpt = m_pView->GetWrtShell().GetViewOptions();
                if (!pOpt)
                    throw RuntimeException();
                sal_uInt32 nFlag = VIEWOPT_1_ONLINESPELL;
                bool bVal = 0 != (pOpt->GetCoreOptions() & nFlag);
                aRet <<= bVal;
            }
            break;
            default :
                OSL_FAIL("unknown WID");
        }
    }

    return aRet;
}

void SAL_CALL SwXTextView::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL SwXTextView::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener >& /*rxListener*/ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

OUString SwXTextView::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextView");
}

sal_Bool SwXTextView::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextView::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextDocumentView";
    pArray[1] = "com.sun.star.view.OfficeDocumentView";
    return aRet;
}

SwXTextViewCursor::SwXTextViewCursor(SwView* pVw) :
    m_pView(pVw),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR))
{
}

SwXTextViewCursor::~SwXTextViewCursor()
{
}

// used to determine if there is a text selection or not.
// If there is no text selection the functions that need a working
// cursor will be disabled (throw RuntimeException). This will be the case
// for the following interfaces:
// - XViewCursor
// - XTextCursor
// - XTextRange
// - XLineCursor
bool SwXTextViewCursor::IsTextSelection( bool bAllowTables ) const
{

    bool bRes = false;
    OSL_ENSURE(m_pView, "m_pView is NULL ???");
    if(m_pView)
    {
        //! m_pView->GetShellMode() will only work after the shell
        //! has already changed and thus can not be used here!
        SelectionType eSelType = m_pView->GetWrtShell().GetSelectionType();
        bRes =  ( (nsSelectionType::SEL_TXT & eSelType) ||
                  (nsSelectionType::SEL_NUM & eSelType) )  &&
                (!(nsSelectionType::SEL_TBL_CELLS & eSelType) || bAllowTables);
    }
    return bRes;
}

sal_Bool SwXTextViewCursor::isVisible() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OSL_FAIL("not implemented");
    return sal_True;
}

void SwXTextViewCursor::setVisible(sal_Bool /*bVisible*/) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OSL_FAIL("not implemented");
}

awt::Point SwXTextViewCursor::getPosition() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    awt::Point aRet;
    if(m_pView)
    {
        const SwWrtShell& rSh = m_pView->GetWrtShell();
        const SwRect aCharRect(rSh.GetCharRect());

        const SwFrameFormat& rMaster = rSh.GetPageDesc( rSh.GetCurPageDesc() ).GetMaster();

        const SvxULSpaceItem& rUL = rMaster.GetULSpace();
        const long nY = aCharRect.Top() - (rUL.GetUpper() + DOCUMENTBORDER);
        aRet.Y = convertTwipToMm100(nY);

        const SvxLRSpaceItem& rLR = rMaster.GetLRSpace();
        const long nX = aCharRect.Left() - (rLR.GetLeft() + DOCUMENTBORDER);
        aRet.X = convertTwipToMm100(nX);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextViewCursor::collapseToStart()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        if(rSh.HasSelection())
        {
            SwPaM* pShellCursor = rSh.GetCursor();
            if(*pShellCursor->GetPoint() > *pShellCursor->GetMark())
                pShellCursor->Exchange();
            pShellCursor->DeleteMark();
            rSh.EnterStdMode();
            rSh.SetSelection(*pShellCursor);
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::collapseToEnd()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        if(rSh.HasSelection())
        {
            SwPaM* pShellCursor = rSh.GetCursor();
            if(*pShellCursor->GetPoint() < *pShellCursor->GetMark())
                pShellCursor->Exchange();
            pShellCursor->DeleteMark();
            rSh.EnterStdMode();
            rSh.SetSelection(*pShellCursor);
        }
    }
    else
        throw uno::RuntimeException();
}

sal_Bool SwXTextViewCursor::isCollapsed()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        const SwWrtShell& rSh = m_pView->GetWrtShell();
        bRet = !rSh.HasSelection();
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

sal_Bool SwXTextViewCursor::goLeft(sal_Int16 nCount, sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().Left( CRSR_SKIP_CHARS, bExpand, nCount, true );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::goRight(sal_Int16 nCount, sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().Right( CRSR_SKIP_CHARS, bExpand, nCount, true );
    }
    else
        throw uno::RuntimeException();
    return bRet;

}

void SwXTextViewCursor::gotoRange(
    const uno::Reference< text::XTextRange > & xRange,
    sal_Bool bExpand)
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView && xRange.is())
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwUnoInternalPaM rDestPam(*m_pView->GetDocShell()->GetDoc());
        if (!::sw::XTextRangeToSwPaM(rDestPam, xRange))
        {
            throw uno::RuntimeException();
        }

        ShellModes  eSelMode = m_pView->GetShellMode();
        SwWrtShell& rSh = m_pView->GetWrtShell();
        // call EnterStdMode in non-text selections only
        if(!bExpand ||
           (eSelMode != SHELL_MODE_TABLE_TEXT &&
            eSelMode != SHELL_MODE_LIST_TEXT &&
            eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
            eSelMode != SHELL_MODE_TEXT ))
                rSh.EnterStdMode();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwPaM aOwnPaM(*pShellCursor->GetPoint());
        if(pShellCursor->HasMark())
        {
            aOwnPaM.SetMark();
            *aOwnPaM.GetMark() = *pShellCursor->GetMark();
        }

        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xRange, uno::UNO_QUERY);
        SwXTextRange* pRange = nullptr;
        SwXParagraph* pPara = nullptr;
        OTextCursorHelper* pCursor = nullptr;
        if(xRangeTunnel.is())
        {
            pRange = reinterpret_cast<SwXTextRange*>(xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId()));
            pCursor = reinterpret_cast<OTextCursorHelper*>(xRangeTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
            pPara = reinterpret_cast<SwXParagraph*>(xRangeTunnel->getSomething(
                                    SwXParagraph::getUnoTunnelId()));
        }

        const FrameTypeFlags nFrameType = rSh.GetFrameType(nullptr,true);

        SwStartNodeType eSearchNodeType = SwNormalStartNode;
        if(nFrameType & FrameTypeFlags::FLY_ANY)
            eSearchNodeType = SwFlyStartNode;
        else if(nFrameType &FrameTypeFlags::HEADER)
            eSearchNodeType = SwHeaderStartNode;
        else if(nFrameType & FrameTypeFlags::FOOTER)
            eSearchNodeType = SwFooterStartNode;
        else if(nFrameType & FrameTypeFlags::TABLE)
            eSearchNodeType = SwTableBoxStartNode;
        else if(nFrameType & FrameTypeFlags::FOOTNOTE)
            eSearchNodeType = SwFootnoteStartNode;

        const SwStartNode* pOwnStartNode = aOwnPaM.GetNode().
                                                FindSttNodeByType(eSearchNodeType);

        const SwNode* pSrcNode = nullptr;
        if(pCursor && pCursor->GetPaM())
        {
            pSrcNode = &pCursor->GetPaM()->GetNode();
        }
        else if (pRange)
        {
            SwPaM aPam(pRange->GetDoc().GetNodes());
            if (pRange->GetPositions(aPam))
            {
                pSrcNode = &aPam.GetNode();
            }
        }
        else if (pPara && pPara->GetTextNode())
        {
            pSrcNode = pPara->GetTextNode();
        }
        const SwStartNode* pTmp = pSrcNode ? pSrcNode->FindSttNodeByType(eSearchNodeType) : nullptr;

        //Skip SectionNodes
        while(pTmp && pTmp->IsSectionNode())
        {
            pTmp = pTmp->StartOfSectionNode();
        }
        while(pOwnStartNode && pOwnStartNode->IsSectionNode())
        {
            pOwnStartNode = pOwnStartNode->StartOfSectionNode();
        }
        //Without Expand it is allowed to jump out with the ViewCursor everywhere,
        //with Expand only in the same environment
        if(bExpand &&
            (pOwnStartNode != pTmp ||
            (eSelMode != SHELL_MODE_TABLE_TEXT &&
                eSelMode != SHELL_MODE_LIST_TEXT &&
                eSelMode != SHELL_MODE_TABLE_LIST_TEXT &&
                eSelMode != SHELL_MODE_TEXT)))
            throw uno::RuntimeException();

        //Now, the selection must be expanded.
        if(bExpand)
        {
            // The cursor should include everything that has been included
            // by him and the transferred Range.
            SwPosition aOwnLeft(*aOwnPaM.Start());
            SwPosition aOwnRight(*aOwnPaM.End());
            SwPosition* pParamLeft = rDestPam.Start();
            SwPosition* pParamRight = rDestPam.End();
            // Now four SwPositions are there, two of them are needed, but which?
            if(aOwnRight > *pParamRight)
                *aOwnPaM.GetPoint() = aOwnRight;
            else
                *aOwnPaM.GetPoint() = *pParamRight;
            aOwnPaM.SetMark();
            if(aOwnLeft < *pParamLeft)
                *aOwnPaM.GetMark() = aOwnLeft;
            else
                *aOwnPaM.GetMark() = *pParamLeft;
        }
        else
        {
            //The cursor shall match the passed range.
            *aOwnPaM.GetPoint() = *rDestPam.GetPoint();
            if(rDestPam.HasMark())
            {
                aOwnPaM.SetMark();
                *aOwnPaM.GetMark() = *rDestPam.GetMark();
            }
            else
                aOwnPaM.DeleteMark();
        }
        rSh.SetSelection(aOwnPaM);
    }
    else
        throw uno::RuntimeException();

}

void SwXTextViewCursor::gotoStart(sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().SttDoc( bExpand );
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::gotoEnd(sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().EndDoc( bExpand );
    }
    else
        throw uno::RuntimeException();
}

sal_Bool SwXTextViewCursor::jumpToFirstPage()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        if (rSh.IsSelFrameMode())
        {
            rSh.UnSelectFrame();
            rSh.LeaveSelFrameMode();
        }
        rSh.EnterStdMode();
        bRet = rSh.SttEndDoc(true);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToLastPage()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        if (rSh.IsSelFrameMode())
        {
            rSh.UnSelectFrame();
            rSh.LeaveSelFrameMode();
        }
        rSh.EnterStdMode();
        bRet = rSh.SttEndDoc(false);
        rSh.SttPg();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToPage(sal_Int16 nPage) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
        bRet = m_pView->GetWrtShell().GotoPage(nPage, true);
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToNextPage() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
        bRet = m_pView->GetWrtShell().SttNxtPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToPreviousPage() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
        bRet = m_pView->GetWrtShell().EndPrvPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToEndOfPage() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
        bRet = m_pView->GetWrtShell().EndPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::jumpToStartOfPage() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
        bRet = m_pView->GetWrtShell().SttPg();
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Int16 SwXTextViewCursor::getPage()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    sal_Int16 nRet = 0;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        nRet = static_cast<sal_Int16>(pShellCursor->GetPageNum());
    }
    else
        throw uno::RuntimeException();
    return nRet;
}

sal_Bool SwXTextViewCursor::screenDown()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        SfxRequest aReq(FN_PAGEDOWN, SfxCallMode::SLOT, m_pView->GetPool());
        m_pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && static_cast<const SfxBoolItem*>(pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::screenUp()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        SfxRequest aReq(FN_PAGEUP, SfxCallMode::SLOT, m_pView->GetPool());
        m_pView->Execute(aReq);
        const SfxPoolItem* pRet = aReq.GetReturnValue();
        bRet = pRet && static_cast<const SfxBoolItem*>(pRet)->GetValue();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

uno::Reference< text::XText >  SwXTextViewCursor::getText()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XText >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = ::sw::CreateParentXText(*pDoc, *pShellCursor->Start());
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextViewCursor::getStart()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateXTextRange(*pDoc, *pShellCursor->Start(), nullptr);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< text::XTextRange >  SwXTextViewCursor::getEnd()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  xRet;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwDoc* pDoc = m_pView->GetDocShell()->GetDoc();
        xRet = SwXTextRange::CreateXTextRange(*pDoc, *pShellCursor->End(), nullptr);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

OUString SwXTextViewCursor::getString()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString uRet;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            //! since setString for SEL_TABLE_TEXT (with possible
            //! multi selection of cells) would not work properly we
            //! will ignore this case for both
            //! functions (setString AND getString) because of symmetrie.

            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT:
            case SHELL_MODE_TEXT            :
            {
                SwWrtShell& rSh = m_pView->GetWrtShell();
                SwPaM* pShellCursor = rSh.GetCursor();
                SwUnoCursorHelper::GetTextFromPam(*pShellCursor, uRet);
            }
            default:;//prevent warning
        }
    }
    return uRet;
}

void SwXTextViewCursor::setString(const OUString& aString)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        ShellModes  eSelMode = m_pView->GetShellMode();
        switch(eSelMode)
        {
            //! since setString for SEL_TABLE_TEXT (with possible
            //! multi selection of cells) would not work properly we
            //! will ignore this case for both
            //! functions (setString AND getString) because of symmetrie.

            case SHELL_MODE_LIST_TEXT       :
            case SHELL_MODE_TABLE_LIST_TEXT :
            case SHELL_MODE_TEXT            :
            {
                SwWrtShell& rSh = m_pView->GetWrtShell();
                SwCursor* pShellCursor = rSh.GetSwCursor();
                SwUnoCursorHelper::SetString(*pShellCursor, aString);
            }
            default:;//prevent warning
        }
    }
}

uno::Reference< XPropertySetInfo >  SwXTextViewCursor::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    static uno::Reference< XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void  SwXTextViewCursor::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
    throw (UnknownPropertyException, PropertyVetoException,
           IllegalArgumentException, WrappedTargetException,
           RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwNode& rNode = pShellCursor->GetNode();
        if (rNode.IsTextNode())
        {
            SwUnoCursorHelper::SetPropertyValue(
                *pShellCursor, *m_pPropSet, rPropertyName, aValue );
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
}

Any  SwXTextViewCursor::getPropertyValue( const OUString& rPropertyName )
    throw (UnknownPropertyException, WrappedTargetException,
           RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        aRet = SwUnoCursorHelper::GetPropertyValue(
                *pShellCursor, *m_pPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return aRet;
}

void  SwXTextViewCursor::addPropertyChangeListener(
    const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}

void  SwXTextViewCursor::removePropertyChangeListener(
    const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}

void  SwXTextViewCursor::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}

void  SwXTextViewCursor::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}

PropertyState  SwXTextViewCursor::getPropertyState( const OUString& rPropertyName )
    throw (UnknownPropertyException, RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    PropertyState eState;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        eState = SwUnoCursorHelper::GetPropertyState(
                *pShellCursor, *m_pPropSet, rPropertyName);
    }
    else
        throw RuntimeException();
    return eState;
}

Sequence< PropertyState >  SwXTextViewCursor::getPropertyStates(
    const Sequence< OUString >& rPropertyNames )
        throw (UnknownPropertyException, RuntimeException,
               std::exception)
{
    SolarMutexGuard aGuard;
    Sequence< PropertyState >  aRet;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        aRet = SwUnoCursorHelper::GetPropertyStates(
                *pShellCursor, *m_pPropSet,  rPropertyNames);
    }
    return aRet;
}

void  SwXTextViewCursor::setPropertyToDefault( const OUString& rPropertyName )
    throw (UnknownPropertyException, RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        SwUnoCursorHelper::SetPropertyToDefault(
                *pShellCursor, *m_pPropSet, rPropertyName);
    }
}

Any  SwXTextViewCursor::getPropertyDefault( const OUString& rPropertyName )
    throw (UnknownPropertyException, WrappedTargetException,
           RuntimeException, std::exception)
{
    Any aRet;
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        SwWrtShell& rSh = m_pView->GetWrtShell();
        SwPaM* pShellCursor = rSh.GetCursor();
        aRet = SwUnoCursorHelper::GetPropertyDefault(
                *pShellCursor, *m_pPropSet, rPropertyName);
    }
    return aRet;
}

sal_Bool SwXTextViewCursor::goDown(sal_Int16 nCount, sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().Down( bExpand, nCount, true );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::goUp(sal_Int16 nCount, sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection())
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().Up( bExpand, nCount, true );
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::isAtStartOfLine()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().IsAtLeftMargin();
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

sal_Bool SwXTextViewCursor::isAtEndOfLine()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        bRet = m_pView->GetWrtShell().IsAtRightMargin(true);
    }
    else
        throw uno::RuntimeException();
    return bRet;
}

void SwXTextViewCursor::gotoEndOfLine(sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().RightMargin(bExpand, true);
    }
    else
        throw uno::RuntimeException();
}

void SwXTextViewCursor::gotoStartOfLine(sal_Bool bExpand)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_pView)
    {
        if (!IsTextSelection( false ))
            throw  uno::RuntimeException("no text selection", static_cast < cppu::OWeakObject * > ( this ) );

        m_pView->GetWrtShell().LeftMargin(bExpand, true);
    }
    else
        throw uno::RuntimeException();
}

OUString SwXTextViewCursor::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("SwXTextViewCursor");
}

sal_Bool SwXTextViewCursor::supportsService(const OUString& rServiceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SwXTextViewCursor::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextViewCursor";
    pArray[1] = "com.sun.star.style.CharacterProperties";
    pArray[2] = "com.sun.star.style.CharacterPropertiesAsian";
    pArray[3] = "com.sun.star.style.CharacterPropertiesComplex";
    pArray[4] = "com.sun.star.style.ParagraphProperties";
    pArray[5] = "com.sun.star.style.ParagraphPropertiesAsian";
    pArray[6] = "com.sun.star.style.ParagraphPropertiesComplex";
    return aRet;
}

namespace
{
    class theSwXTextViewCursorUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextViewCursorUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextViewCursor::getUnoTunnelId()
{
    return theSwXTextViewCursorUnoTunnelId::get().getSeq();
}

//XUnoTunnel
sal_Int64 SAL_CALL SwXTextViewCursor::getSomething(
    const uno::Sequence< sal_Int8 >& rId )
        throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
                return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
        }
    return OTextCursorHelper::getSomething(rId);
}

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextViewCursor,SwXTextViewCursor_Base,OTextCursorHelper)
const SwDoc*        SwXTextViewCursor::GetDoc() const
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return   rSh.GetCursor() ? rSh.GetCursor()->GetDoc() : nullptr;
}

SwDoc*  SwXTextViewCursor::GetDoc()
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return   rSh.GetCursor() ? rSh.GetCursor()->GetDoc() : nullptr;
}

const SwPaM*    SwXTextViewCursor::GetPaM() const
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return rSh.GetCursor();
}

SwPaM*  SwXTextViewCursor::GetPaM()
{
    SwWrtShell& rSh = m_pView->GetWrtShell();
    return rSh.GetCursor();
}

uno::Reference< datatransfer::XTransferable > SAL_CALL SwXTextView::getTransferable()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //force immediat shell update
    GetView()->StopShellTimer();
    SwWrtShell& rSh = GetView()->GetWrtShell();
    if ( GetView()->GetShellMode() == SHELL_MODE_DRAWTEXT )
    {
        SdrView *pSdrView = rSh.GetDrawView();
        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
        return pOLV->GetEditView().GetTransferable();
    }
    else
    {
        SwTransferable* pTransfer = new SwTransferable( rSh );
        const bool bLockedView = rSh.IsViewLocked();
        rSh.LockView( true );    //lock visible section
        pTransfer->PrepareForCopy();
        rSh.LockView( bLockedView );
        return uno::Reference< datatransfer::XTransferable >( pTransfer );
    }
}

void SAL_CALL SwXTextView::insertTransferable( const uno::Reference< datatransfer::XTransferable >& xTrans ) throw (datatransfer::UnsupportedFlavorException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    //force immediat shell update
    GetView()->StopShellTimer();
    SwWrtShell& rSh = GetView()->GetWrtShell();
    if ( GetView()->GetShellMode() == SHELL_MODE_DRAWTEXT )
    {
        SdrView *pSdrView = rSh.GetDrawView();
        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
        pOLV->GetEditView().InsertText( xTrans, GetView()->GetDocShell()->GetMedium()->GetBaseURL(), false );
    }
    else
    {
        TransferableDataHelper aDataHelper( xTrans );
        if ( SwTransferable::IsPaste( rSh, aDataHelper ) )
        {
            SwTransferable::Paste( rSh, aDataHelper );
            if( rSh.IsFrameSelected() || rSh.IsObjSelected() )
                rSh.EnterSelFrameMode();
            GetView()->AttrChangedNotify( &rSh );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
