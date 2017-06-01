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

#include <sal/macros.h>

#include <svx/fmpage.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <svx/fmmodel.hxx>

#include "fmobj.hxx"

#include <svx/dialmgr.hxx>

#include "fmpgeimp.hxx"

#include <sfx2/objsh.hxx>
#include <svx/svditer.hxx>
#include <svx/svdview.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>


#include <svx/fmglob.hxx>
#include "fmprop.hrc"
#include "fmundo.hxx"
#include "svx/fmtools.hxx"
using namespace ::svxform;
#include <comphelper/property.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;


FmFormPage::FmFormPage(FmFormModel& rModel, bool bMasterPage)
           :SdrPage(rModel, bMasterPage)
           ,m_pImpl( new FmFormPageImpl( *this ) )
{
}


FmFormPage::FmFormPage(const FmFormPage& rPage)
           :SdrPage(rPage)
           ,m_pImpl(new FmFormPageImpl( *this ) )
{
}

void FmFormPage::lateInit(const FmFormPage& rPage, FmFormModel* const pNewModel)
{
    SdrPage::lateInit( rPage, pNewModel );

    m_pImpl->initFrom( rPage.GetImpl() );
    m_sPageName = rPage.m_sPageName;
}


FmFormPage::~FmFormPage()
{
}


void FmFormPage::SetModel(SdrModel* pNewModel)
{
    /* #35055# */
    // we want to call the super's "SetModel" method even if the model is the
    // same, in case code somewhere in the system depends on it.  But our code
    // doesn't, so get the old model to do a check.
    SdrModel *pOldModel = GetModel();

    SdrPage::SetModel( pNewModel );

    /* #35055# */
    if ( ( pOldModel != pNewModel ) && m_pImpl )
    {
        try
        {
            Reference< css::form::XForms > xForms( m_pImpl->getForms( false ) );
            if ( xForms.is() )
            {
                // we want to keep the current collection, just reset the model
                // with which it's associated.
                FmFormModel* pDrawModel = static_cast<FmFormModel*>( GetModel() );
                SfxObjectShell* pObjShell = pDrawModel->GetObjectShell();
                if ( pObjShell )
                    xForms->setParent( pObjShell->GetModel() );
            }
        }
        catch( css::uno::Exception const& )
        {
            OSL_FAIL( "UNO Exception caught resetting model for m_pImpl (FmFormPageImpl) in FmFormPage::SetModel" );
        }
    }
}


SdrPage* FmFormPage::Clone() const
{
    return Clone(nullptr);
}

SdrPage* FmFormPage::Clone(SdrModel* const pNewModel) const
{
    FmFormPage* const pNewPage = new FmFormPage(*this);
    FmFormModel* pFormModel = nullptr;
    if (pNewModel)
    {
        pFormModel = dynamic_cast<FmFormModel*>(pNewModel);
        assert(pFormModel);
    }
    pNewPage->lateInit(*this, pFormModel);
    return pNewPage;
}


void FmFormPage::InsertObject(SdrObject* pObj, size_t nPos)
{
    SdrPage::InsertObject( pObj, nPos );
    if (GetModel())
        static_cast<FmFormModel*>(GetModel())->GetUndoEnv().Inserted(pObj);
}


const Reference< css::form::XForms > & FmFormPage::GetForms( bool _bForceCreate ) const
{
    const SdrPage& rMasterPage( *this );
    const FmFormPage* pFormPage = dynamic_cast< const FmFormPage* >( &rMasterPage );
    OSL_ENSURE( pFormPage, "FmFormPage::GetForms: referenced page is no FmFormPage - is this allowed?!" );
    if ( !pFormPage )
        pFormPage = this;

    return pFormPage->m_pImpl->getForms( _bForceCreate );
}


bool FmFormPage::RequestHelp( vcl::Window* pWindow, SdrView* pView,
                              const HelpEvent& rEvt )
{
    if( pView->IsAction() )
        return false;

    Point aPos = rEvt.GetMousePosPixel();
    aPos = pWindow->ScreenToOutputPixel( aPos );
    aPos = pWindow->PixelToLogic( aPos );

    SdrPageView* pPV = nullptr;
    SdrObject* pObj = pView->PickObj(aPos, 0, pPV, SdrSearchOptions::DEEP);
    if (!pObj)
        return false;

    FmFormObj* pFormObject = FmFormObj::GetFormObject( pObj );
    if ( !pFormObject )
        return false;

    OUString aHelpText;
    css::uno::Reference< css::beans::XPropertySet >  xSet( pFormObject->GetUnoControlModel(), css::uno::UNO_QUERY );
    if (xSet.is())
    {
        if (::comphelper::hasProperty(FM_PROP_HELPTEXT, xSet))
            aHelpText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_HELPTEXT)).getStr();

        if (aHelpText.isEmpty() && ::comphelper::hasProperty(FM_PROP_TARGET_URL, xSet))
        {
            OUString aText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_TARGET_URL));
            INetURLObject aUrl(aText);

            // test if it is a protocol type that I want to display
            INetProtocol aProtocol = aUrl.GetProtocol();
            static const INetProtocol s_aQuickHelpSupported[] =
                {   INetProtocol::Ftp, INetProtocol::Http, INetProtocol::File, INetProtocol::Mailto,
                    INetProtocol::Https, INetProtocol::Javascript,
                    INetProtocol::Ldap
                };
            for (const INetProtocol& i : s_aQuickHelpSupported)
                if (i == aProtocol)
                {
                    aHelpText = INetURLObject::decode(aUrl.GetURLNoPass(), INetURLObject::DecodeMechanism::Unambiguous);
                    break;
                }
        }
    }
    if ( !aHelpText.isEmpty() )
    {
        // display the help
        tools::Rectangle aItemRect = pObj->GetCurrentBoundRect();
        aItemRect = pWindow->LogicToPixel( aItemRect );
        Point aPt = pWindow->OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = pWindow->OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();
        if( rEvt.GetMode() == HelpEventMode::BALLOON )
            Help::ShowBalloon( pWindow, aItemRect.Center(), aItemRect, aHelpText);
        else
            Help::ShowQuickHelp( pWindow, aItemRect, aHelpText );
    }
    return true;
}


SdrObject* FmFormPage::RemoveObject(size_t nObjNum)
{
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
    if (pObj && GetModel())
        static_cast<FmFormModel*>(GetModel())->GetUndoEnv().Removed(pObj);
    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
