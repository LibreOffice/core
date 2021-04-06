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

#include <svx/fmpage.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <svx/fmmodel.hxx>

#include <fmobj.hxx>

#include <fmpgeimp.hxx>

#include <svx/svdview.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <vcl/window.hxx>


#include <fmprop.hxx>
#include <fmundo.hxx>
using namespace ::svxform;
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;


FmFormPage::FmFormPage(FmFormModel& rModel, bool bMasterPage)
:   SdrPage(rModel, bMasterPage)
    ,m_pImpl( new FmFormPageImpl( *this ) )
{
}

void FmFormPage::lateInit(const FmFormPage& rPage)
{
    // call parent
    SdrPage::lateInit( rPage );

    // copy local variables (former stuff from copy constructor)
    m_pImpl->initFrom( rPage.GetImpl() );
    m_sPageName = rPage.m_sPageName;
}


FmFormPage::~FmFormPage()
{
}

rtl::Reference<SdrPage> FmFormPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    FmFormModel& rFmFormModel(static_cast< FmFormModel& >(rTargetModel));
    rtl::Reference<FmFormPage> pClonedFmFormPage =
        new FmFormPage(
            rFmFormModel,
            IsMasterPage());
    pClonedFmFormPage->lateInit(*this);
    return pClonedFmFormPage;
}


void FmFormPage::InsertObject(SdrObject* pObj, size_t nPos)
{
    SdrPage::InsertObject( pObj, nPos );
    static_cast< FmFormModel& >(getSdrModelFromSdrPage()).GetUndoEnv().Inserted(pObj);
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


bool FmFormPage::RequestHelp( vcl::Window* pWindow, SdrView const * pView,
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
            aHelpText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_HELPTEXT));

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
                    aHelpText = aUrl.GetURLNoPass(INetURLObject::DecodeMechanism::Unambiguous);
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
        aItemRect.SetLeft( aPt.X() );
        aItemRect.SetTop( aPt.Y() );
        aPt = pWindow->OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.SetRight( aPt.X() );
        aItemRect.SetBottom( aPt.Y() );
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
    if (pObj)
        static_cast< FmFormModel& >(getSdrModelFromSdrPage()).GetUndoEnv().Removed(pObj);
    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
