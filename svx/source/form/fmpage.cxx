/*************************************************************************
 *
 *  $RCSfile: fmpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:18:56 $
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

#pragma hdrstop

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

#ifndef _SVDIO_HXX //autogen
#include "svdio.hxx"
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
#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

TYPEINIT1(FmFormPage, SdrPage);

//------------------------------------------------------------------
FmFormPage::FmFormPage(FmFormModel& rModel, StarBASIC* _pBasic, FASTBOOL bMasterPage)
           :SdrPage(rModel, bMasterPage)
           ,pBasic(_pBasic)
#ifndef SVX_LIGHT
           ,pImpl(new FmFormPageImpl(this))
#else
           ,pImpl(NULL)
#endif
{
}

//------------------------------------------------------------------
FmFormPage::FmFormPage(const FmFormPage& rPage)
           :SdrPage(rPage)
           ,pBasic(0)
#ifndef SVX_LIGHT
           ,pImpl(new FmFormPageImpl(this, *rPage.GetImpl()))
#else
           ,pImpl(NULL)
#endif
{
    aPageName = rPage.aPageName;
}

//------------------------------------------------------------------
FmFormPage::~FmFormPage()
{
#ifndef SVX_LIGHT
    delete pImpl;
#endif
}

//------------------------------------------------------------------
void FmFormPage::WriteData(SvStream& rOut) const
{
#ifndef SVX_LIGHT
    {
        {
            SdrDownCompat aVCCompat1( rOut, STREAM_WRITE );
            sal_uInt16 n = 0;
            rOut << n;
        }
        SdrPage::WriteData( rOut );
        SdrDownCompat aVCCompat2( rOut, STREAM_WRITE );

        rOut << ByteString(aPageName, gsl_getSystemTextEncoding());
        rOut << (sal_uInt32)0x11051967;
        rOut << (sal_uInt32)0x19670511;
        sal_uInt16 nVer = 1;
        rOut << nVer;
        {
            SdrDownCompat aVCCompat3( rOut, STREAM_WRITE);
            sal_uInt32 nFormCount = 0;
            rOut << nFormCount;
        }
    }

    // dont use the flag in that way: if (rOut.GetVersion() >= SOFFICE_FILEFORMAT_40)
    if (rOut.GetVersion() >= 3830)
    {
        SdrDownCompat aCompat(rOut, STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
        pImpl->WriteData(rOut);
    }
#endif
}

//------------------------------------------------------------------
void FmFormPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    {
        {
            SdrDownCompat aVCCompat1( rIn, STREAM_READ );
        }
        SdrPage::ReadData( rHead, rIn );
        {
            SdrDownCompat aVCCompat2( rIn, STREAM_READ );
            ByteString aByteStringName;
            rIn >> aByteStringName;
            aPageName = String(aByteStringName, gsl_getSystemTextEncoding());
        }
    }

    // dont use the flag in that way: if (rIn.GetVersion() >= SOFFICE_FILEFORMAT_40)
    if (rIn.GetVersion() >= 3830 && rHead.GetVersion() >=14)
    {
        SdrDownCompat aCompat(rIn, STREAM_READ);    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifndef SVX_LIGHT
        pImpl->ReadData(rHead, rIn);
#endif
    }
}

//------------------------------------------------------------------
void FmFormPage::SetModel(SdrModel* pNewModel)
{
    SdrPage::SetModel( pNewModel );
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

#ifndef SVX_LIGHT
//------------------------------------------------------------------
void FmFormPage::InsertFormObjectEnsureEnv(const SdrObject* _pSourceObject, SdrObject* _pClone, sal_Bool _bTryPreserveName, sal_uInt32 _nPos,
    const SdrInsertReason* _pReason)
{
    DBG_ERROR("FmFormPage::InsertFormObjectEnsureEnv : obsolete method used (maybe I should write a changes mail ?) !");
    InsertObject(_pClone, _nPos, _pReason);
    return;
}

//------------------------------------------------------------------
sal_Bool FmFormPage::EnsureFormObjectEnv(const SdrObject* _pObj)
{
    DBG_ERROR("FmFormPage::EnsureFormObjectEnv : obsolete method used (maybe I should write a changes mail ?) !");
    return sal_False;
}
#endif

//------------------------------------------------------------------
const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & FmFormPage::GetForms() const
{
#ifndef SVX_LIGHT
    return pImpl->getForms();
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
                    Rectangle aItemRect = pObj->GetBoundRect();
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


//------------------------------------------------------------------
void FmFormPage::SetBasic( StarBASIC* pBas )
{
}

void FmFormPage::RequestBasic()
{
}

XubString FmFormPage::GetLinkData( const XubString& rLinkName )
{
    return XubString();
}

void FmFormPage::SetLinkData( const XubString& rLinkName, const XubString& rLinkData )
{
}

void FmFormPage::UpdateLinkData( const XubString& rLinkName, const XubString& rLinkData )
{
}

SfxJSArray* FmFormPage::GetFormsArray() const
{
    return 0;
}




