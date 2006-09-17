/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmpage.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:07:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif


#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef SVX_LIGHT
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#endif

#ifndef SVX_LIGHT
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

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

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svdview.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif


#ifndef SVX_LIGHT
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
using namespace ::svxform;
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::container::XChild;

TYPEINIT1(FmFormPage, SdrPage);

//------------------------------------------------------------------
FmFormPage::FmFormPage(FmFormModel& rModel, StarBASIC* _pBasic, FASTBOOL bMasterPage)
           :SdrPage(rModel, bMasterPage)
#ifndef SVX_LIGHT
           ,m_pImpl(new FmFormPageImpl(this))
#else
           ,m_pImpl(NULL)
#endif
           ,m_pBasic(_pBasic)
{
}

//------------------------------------------------------------------
FmFormPage::FmFormPage(const FmFormPage& rPage)
           :SdrPage(rPage)
#ifndef SVX_LIGHT
           ,m_pImpl(new FmFormPageImpl(this, *rPage.GetImpl()))
#else
           ,m_pImpl(NULL)
#endif
           ,m_pBasic(0)
{
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
            if ( m_pImpl->m_xForms.is() )
            {
                // we want to keep the current collection, just reset the model
                // with which it's associated.
                Reference< XChild > xAsChild( m_pImpl->m_xForms, UNO_QUERY );
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
    return new FmFormPage(*this);
    // hier fehlt noch ein kopieren der Objekte
}

//------------------------------------------------------------------
void FmFormPage::NbcInsertObject(SdrObject* pObj,
                                 sal_uInt32 nPos,
                                 const SdrInsertReason* pReason)
{
    SdrPage::NbcInsertObject(pObj, nPos, pReason);
}

//------------------------------------------------------------------
void FmFormPage::InsertObject(SdrObject* pObj, sal_uInt32 nPos,
                              const SdrInsertReason* pReason)
{
    SdrPage::InsertObject( pObj, nPos, pReason );
#ifndef SVX_LIGHT
    if (GetModel() && (!pReason || pReason->GetReason() != SDRREASON_STREAMING))
        ((FmFormModel*)GetModel())->GetUndoEnv().Inserted(pObj);
#endif
}

//------------------------------------------------------------------
const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & FmFormPage::GetForms( bool _bForceCreate ) const
{
#ifndef SVX_LIGHT
    return m_pImpl->getForms( _bForceCreate );
#else
    static ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >  aRef;
    return aRef;
#endif
}

//------------------------------------------------------------------
sal_Bool FmFormPage::RequestHelp( Window* pWindow, SdrView* pView,
                              const HelpEvent& rEvt )
{
#ifndef SVX_LIGHT
    if( !pView->IsAction() )
    {
        Point aPos = rEvt.GetMousePosPixel();
        aPos = pWindow->ScreenToOutputPixel( aPos );
        aPos = pWindow->PixelToLogic( aPos );

        SdrObject* pObj = NULL;
        SdrPageView* pPV = NULL;
        if( pView->PickObj( aPos, 0, pObj, pPV, SDRSEARCH_DEEP ) )
        {
            // Ein Object getroffen
            if( pObj->ISA(FmFormObj) )
            {
                UniString aHelpText;
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(((FmFormObj*)pObj)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
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
                return sal_True;
            }
        }
    }
#endif
    return sal_False;
}

//------------------------------------------------------------------
SdrObject* FmFormPage::NbcRemoveObject(sal_uInt32 nObjNum)
{
    return SdrPage::NbcRemoveObject(nObjNum);
}

//------------------------------------------------------------------
SdrObject* FmFormPage::RemoveObject(sal_uInt32 nObjNum)
{
    SdrObject* pObj = SdrPage::RemoveObject(nObjNum);
#ifndef SVX_LIGHT
    if (pObj && GetModel())
        ((FmFormModel*)GetModel())->GetUndoEnv().Removed(pObj);
#endif
    return pObj;
}

//------------------------------------------------------------------
SdrObject* FmFormPage::NbcReplaceObject(SdrObject* pNewObj, sal_uInt32 nObjNum)
{
    return SdrPage::NbcReplaceObject(pNewObj, nObjNum);
}

//------------------------------------------------------------------
SdrObject* FmFormPage::ReplaceObject(SdrObject* pNewObj, sal_uInt32 nObjNum)
{
    return SdrPage::ReplaceObject(pNewObj, nObjNum);
}

