/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <svx/fmpage.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>


#include <svx/fmmodel.hxx>

#ifndef SVX_LIGHT
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#endif

#ifndef SVX_LIGHT
#ifndef _SVX_FMRESIDS_HRC
#include "svx/fmresids.hrc"
#endif
#endif
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>

#ifndef SVX_LIGHT
#ifndef _SVX_FMPGEIMP_HXX
#include "fmpgeimp.hxx"
#endif
#endif

#ifndef SVX_LIGHT
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#endif
#include "svx/svditer.hxx"
#include <svx/svdview.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>


#ifndef SVX_LIGHT
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#include "fmundo.hxx"
#include "svx/fmtools.hxx"
using namespace ::svxform;
#endif
#include <comphelper/property.hxx>
#include <rtl/logfile.hxx>

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::container::XChild;
using com::sun::star::container::XNameContainer;

TYPEINIT1(FmFormPage, SdrPage);

//------------------------------------------------------------------
FmFormPage::FmFormPage(FmFormModel& rModel, StarBASIC* _pBasic, FASTBOOL bMasterPage)
           :SdrPage(rModel, bMasterPage)
#ifndef SVX_LIGHT
           ,m_pImpl( new FmFormPageImpl( *this ) )
#else
           ,m_pImpl(NULL)
#endif
           ,m_pBasic(_pBasic)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::FmFormPage" );
}

//------------------------------------------------------------------
FmFormPage::FmFormPage(const FmFormPage& rPage)
           :SdrPage(rPage)
#ifndef SVX_LIGHT
           ,m_pImpl(new FmFormPageImpl( *this ) )
#else
           ,m_pImpl(NULL)
#endif
           ,m_pBasic(0)
{
#ifndef SVX_LIGHT
    m_pImpl->initFrom( rPage.GetImpl() );
#endif
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::FmFormPage" );
    m_sPageName = rPage.m_sPageName;
}

//------------------------------------------------------------------
FmFormPage::~FmFormPage()
{
#ifndef SVX_LIGHT
    delete m_pImpl;
#endif
}

//------------------------------------------------------------------
void FmFormPage::SetModel(SdrModel* pNewModel)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::SetModel" );
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
            Reference< XNameContainer > xForms( m_pImpl->getForms( false ) );
            if ( xForms.is() )
            {
                // we want to keep the current collection, just reset the model
                // with which it's associated.
                Reference< XChild > xAsChild( xForms, UNO_QUERY );
                if ( xAsChild.is() )
                {
                    FmFormModel* pDrawModel = (FmFormModel*) GetModel();
                    SfxObjectShell* pObjShell = pDrawModel->GetObjectShell();
                    if ( pObjShell )
                        xAsChild->setParent( pObjShell->GetModel() );
                }
            }
        }
        catch( ::com::sun::star::uno::Exception ex )
        {
            OSL_ENSURE( sal_False, "UNO Exception caught resetting model for m_pImpl (FmFormPageImpl) in FmFormPage::SetModel" );
        }
    }
}

//------------------------------------------------------------------
SdrPage* FmFormPage::Clone() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::Clone" );
    return new FmFormPage(*this);
    // hier fehlt noch ein kopieren der Objekte
}

//------------------------------------------------------------------
void FmFormPage::InsertObject(SdrObject* pObj, sal_uLong nPos,
                              const SdrInsertReason* pReason)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::InsertObject" );
    SdrPage::InsertObject( pObj, nPos, pReason );
#ifndef SVX_LIGHT
    if (GetModel() && (!pReason || pReason->GetReason() != SDRREASON_STREAMING))
        ((FmFormModel*)GetModel())->GetUndoEnv().Inserted(pObj);
#endif
}

//------------------------------------------------------------------
const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & FmFormPage::GetForms( bool _bForceCreate ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::GetForms" );
#ifndef SVX_LIGHT
    const SdrPage& rMasterPage( *this );
    const FmFormPage* pFormPage = dynamic_cast< const FmFormPage* >( &rMasterPage );
    OSL_ENSURE( pFormPage, "FmFormPage::GetForms: referenced page is no FmFormPage - is this allowed?!" );
    if ( !pFormPage )
        pFormPage = this;

    return pFormPage->m_pImpl->getForms( _bForceCreate );
#else
    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  aRef;
    return aRef;
#endif
}

//------------------------------------------------------------------
sal_Bool FmFormPage::RequestHelp( Window* pWindow, SdrView* pView,
                              const HelpEvent& rEvt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::RequestHelp" );
#ifndef SVX_LIGHT
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

    UniString aHelpText;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet( pFormObject->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY );
    if (xSet.is())
    {
        if (::comphelper::hasProperty(FM_PROP_HELPTEXT, xSet))
            aHelpText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_HELPTEXT)).getStr();

        if (!aHelpText.Len() && ::comphelper::hasProperty(FM_PROP_TARGET_URL, xSet))
        {
            ::rtl::OUString aText = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_TARGET_URL));
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
    if ( aHelpText.Len() != 0 )
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
#endif
    return sal_True;
}

//------------------------------------------------------------------
SdrObject* FmFormPage::RemoveObject(sal_uLong nObjNum)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "FmFormPage::RemoveObject" );
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
#ifndef SVX_LIGHT
    if (pObj && GetModel())
        ((FmFormModel*)GetModel())->GetUndoEnv().Removed(pObj);
#endif
    return pObj;
}
