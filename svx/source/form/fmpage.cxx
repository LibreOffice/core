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
#include <com/sun/star/container/XNameContainer.hpp>


#include <svx/fmmodel.hxx>

#include "fmobj.hxx"

#include <svx/fmresids.hrc>
#include <tools/shl.hxx>
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
using com::sun::star::container::XChild;
using com::sun::star::container::XNameContainer;

TYPEINIT1(FmFormPage, SdrPage);

//------------------------------------------------------------------
FmFormPage::FmFormPage(FmFormModel& rModel, StarBASIC* _pBasic, bool bMasterPage)
           :SdrPage(rModel, bMasterPage)
           ,m_pImpl( new FmFormPageImpl( *this ) )
           ,m_pBasic(_pBasic)
{
}

//------------------------------------------------------------------
FmFormPage::FmFormPage(const FmFormPage& rPage)
           :SdrPage(rPage)
           ,m_pImpl(new FmFormPageImpl( *this ) )
           ,m_pBasic(0)
{
    m_pImpl->initFrom( rPage.GetImpl() );
    m_sPageName = rPage.m_sPageName;
}

//------------------------------------------------------------------
FmFormPage::~FmFormPage()
{
    delete m_pImpl;
}

//------------------------------------------------------------------
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
                FmFormModel* pDrawModel = (FmFormModel*) GetModel();
                SfxObjectShell* pObjShell = pDrawModel->GetObjectShell();
                if ( pObjShell )
                    xForms->setParent( pObjShell->GetModel() );
            }
        }
        catch( ::com::sun::star::uno::Exception const& )
        {
            OSL_FAIL( "UNO Exception caught resetting model for m_pImpl (FmFormPageImpl) in FmFormPage::SetModel" );
        }
    }
}

//------------------------------------------------------------------
SdrPage* FmFormPage::Clone() const
{
    return new FmFormPage(*this);
    // hier fehlt noch ein kopieren der Objekte
}

//------------------------------------------------------------------
void FmFormPage::InsertObject(SdrObject* pObj, sal_uLong nPos,
                              const SdrInsertReason* pReason)
{
    SdrPage::InsertObject( pObj, nPos, pReason );
    if (GetModel() && (!pReason || pReason->GetReason() != SDRREASON_STREAMING))
        ((FmFormModel*)GetModel())->GetUndoEnv().Inserted(pObj);
}

//------------------------------------------------------------------
const Reference< css::form::XForms > & FmFormPage::GetForms( bool _bForceCreate ) const
{
    const SdrPage& rMasterPage( *this );
    const FmFormPage* pFormPage = dynamic_cast< const FmFormPage* >( &rMasterPage );
    OSL_ENSURE( pFormPage, "FmFormPage::GetForms: referenced page is no FmFormPage - is this allowed?!" );
    if ( !pFormPage )
        pFormPage = this;

    return pFormPage->m_pImpl->getForms( _bForceCreate );
}

//------------------------------------------------------------------
sal_Bool FmFormPage::RequestHelp( Window* pWindow, SdrView* pView,
                              const HelpEvent& rEvt )
{
    if( pView->IsAction() )
        return sal_False;

    Point aPos = rEvt.GetMousePosPixel();
    aPos = pWindow->ScreenToOutputPixel( aPos );
    aPos = pWindow->PixelToLogic( aPos );

    SdrObject* pObj = NULL;
    SdrPageView* pPV = NULL;
    if ( !pView->PickObj( aPos, 0, pObj, pPV, SDRSEARCH_DEEP ) )
        return sal_False;

    FmFormObj* pFormObject = FmFormObj::GetFormObject( pObj );
    if ( !pFormObject )
        return sal_False;

    OUString aHelpText;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet( pFormObject->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY );
    if (xSet.is())
    {
        if (::comphelper::hasProperty(FM_PROP_HELPTEXT, xSet))
            aHelpText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_HELPTEXT)).getStr();

        if (aHelpText.isEmpty() && ::comphelper::hasProperty(FM_PROP_TARGET_URL, xSet))
        {
            OUString aText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_TARGET_URL));
            INetURLObject aUrl(aText);

            // testen, ob es ein Protokoll-Typ ist, den ich anzeigen will
            INetProtocol aProtocol = aUrl.GetProtocol();
            static const INetProtocol s_aQuickHelpSupported[] =
                {   INET_PROT_FTP, INET_PROT_HTTP, INET_PROT_FILE, INET_PROT_MAILTO, INET_PROT_NEWS,
                    INET_PROT_HTTPS, INET_PROT_JAVASCRIPT, INET_PROT_IMAP, INET_PROT_POP3,
                    INET_PROT_VIM, INET_PROT_LDAP
                };
            for (sal_uInt16 i=0; i<sizeof(s_aQuickHelpSupported)/sizeof(s_aQuickHelpSupported[0]); ++i)
                if (s_aQuickHelpSupported[i] == aProtocol)
                {
                    aHelpText = INetURLObject::decode(aUrl.GetURLNoPass(), '%', INetURLObject::DECODE_UNAMBIGUOUS);
                    break;
                }
        }
    }
    if ( !aHelpText.isEmpty() )
    {
        // Hilfe anzeigen
        Rectangle aItemRect = pObj->GetCurrentBoundRect();
        aItemRect = pWindow->LogicToPixel( aItemRect );
        Point aPt = pWindow->OutputToScreenPixel( aItemRect.TopLeft() );
        aItemRect.Left()   = aPt.X();
        aItemRect.Top()    = aPt.Y();
        aPt = pWindow->OutputToScreenPixel( aItemRect.BottomRight() );
        aItemRect.Right()  = aPt.X();
        aItemRect.Bottom() = aPt.Y();
        if( rEvt.GetMode() == HELPMODE_BALLOON )
            Help::ShowBalloon( pWindow, aItemRect.Center(), aItemRect, aHelpText);
        else
            Help::ShowQuickHelp( pWindow, aItemRect, aHelpText );
    }
    return sal_True;
}

//------------------------------------------------------------------
SdrObject* FmFormPage::RemoveObject(sal_uLong nObjNum)
{
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
    if (pObj && GetModel())
        ((FmFormModel*)GetModel())->GetUndoEnv().Removed(pObj);
    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
