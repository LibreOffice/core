/*************************************************************************
 *
 *  $RCSfile: unoshtxt.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: cl $ $Date: 2001-08-05 15:33:50 $
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//#include <tools/debug.hxx>
//#include <svx/editeng.hxx>

#include <unoshtxt.hxx>

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svdoutl.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _SVDETC_HXX
#include <svdetc.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <outliner.hxx>
#endif
#ifndef _SVX_UNOFOROU_HXX
#include <unoforou.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include "outlobj.hxx"
#endif
#include "svdotext.hxx"
#include "svdpage.hxx"
#include "editeng.hxx"

#include "unotext.hxx"

using namespace ::osl;
using namespace ::vos;
using namespace ::rtl;

namespace css = ::com::sun::star;

//------------------------------------------------------------------------
// SvxTextEditSourceImpl
//------------------------------------------------------------------------

class SvxTextEditSourceImpl : public SfxListener
{
private:
    oslInterlockedCount maRefCount;

    SdrObject*              mpObject;
    Outliner*               mpOutliner;
    SvxTextForwarder*       mpTextForwarder;
    BOOL                    mbDataValid;
    BOOL                    mbDestroyed;
    BOOL                    mbIsLocked;
    BOOL                    mbNeedsUpdate;
    BOOL                    mbOldUndoMode;

public:
    SvxTextEditSourceImpl( SdrObject* pObject );
    ~SvxTextEditSourceImpl();

    void SAL_CALL acquire();
    void SAL_CALL release();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    SvxEditSource*      Clone() const;
    SvxTextForwarder*   GetTextForwarder();
    void                UpdateData();

    SdrObject* GetSdrObject() const { return mpObject; }

    void lock();
    void unlock();
};

//------------------------------------------------------------------------

SvxTextEditSourceImpl::SvxTextEditSourceImpl( SdrObject* pObject )
:   mpObject        ( pObject ),
    mpOutliner      ( NULL ),
    mpTextForwarder ( NULL ),
    mbDataValid     ( FALSE ),
    mbDestroyed     ( FALSE ),
    mbIsLocked      ( FALSE ),
    mbNeedsUpdate   ( FALSE ),
    mbOldUndoMode   ( FALSE ),
    maRefCount      ( 0 )
{
    DBG_ASSERT( mpObject, "invalid pObject!" );

    if( mpObject && mpObject->GetModel() )
        StartListening( *mpObject->GetModel() );
}

//------------------------------------------------------------------------

SvxTextEditSourceImpl::~SvxTextEditSourceImpl()
{
    DBG_ASSERT( mbIsLocked == sal_False, "text edit source was not unlocked before dispose!" );

    if( mpObject && mpObject->GetModel() )
        EndListening( *mpObject->GetModel() );

    delete mpTextForwarder;
    delete mpOutliner;
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::acquire() throw()
{
    osl_incrementInterlockedCount( &maRefCount );
}

//------------------------------------------------------------------------

void SAL_CALL SvxTextEditSourceImpl::release() throw()
{
    if( ! osl_decrementInterlockedCount( &maRefCount ) )
        delete this;
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    if( pSdrHint )
    {
        if( pSdrHint->GetKind() == HINT_OBJCHG )
            mbDataValid = FALSE;                        // Text muss neu geholt werden
        if( pSdrHint->GetKind() == HINT_OBJREMOVED )
        {
            if( mpObject == pSdrHint->GetObject() )
            {
                mpObject = NULL;
                mbDestroyed = TRUE;
            }
        }
        else if( pSdrHint->GetKind() == HINT_MODELCLEARED ||
                 pSdrHint->GetKind() == HINT_OBJLISTCLEARED )
        {
            if( mpObject )
                EndListening( *mpObject->GetModel() );
            mpObject = NULL;
            mbDestroyed = TRUE;
        }
    }

    if( mbDestroyed )
    {
        if( mpTextForwarder )
        {
            delete mpTextForwarder;
            mpTextForwarder = NULL;
        }

        if( mpOutliner )
        {
            delete mpOutliner;
            mpOutliner = NULL;
        }
    }
}

//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

SvxTextForwarder* SvxTextEditSourceImpl::GetTextForwarder()
{
    if( mbDestroyed || mpObject == NULL )
        return NULL;

    if (!mpTextForwarder)
    {
        if( mpOutliner == NULL )
        {
            SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
            USHORT nOutlMode = OUTLINERMODE_TEXTOBJECT;
            if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_OUTLINETEXT )
                nOutlMode = OUTLINERMODE_OUTLINEOBJECT;
            SdrModel* pModel = mpObject->GetModel();
            mpOutliner = SdrMakeOutliner( nOutlMode, pModel );
            Outliner& aDrawOutliner = pModel->GetDrawOutliner();
            mpOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );

            if( mbIsLocked )
            {
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
                mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
                ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
            }

// -
            css::uno::Reference< css::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            css::uno::Reference< css::linguistic2::XLinguServiceManager > xLinguServiceManager(
                xMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.linguistic2.LinguServiceManager" ))), css::uno::UNO_QUERY );

            if ( xLinguServiceManager.is() )
            {
                css::uno::Reference< css::linguistic2::XHyphenator > xHyphenator( xLinguServiceManager->getHyphenator(), css::uno::UNO_QUERY );
                if( xHyphenator.is() )
                    mpOutliner->SetHyphenator( xHyphenator );
            }
// -
        }

        mpTextForwarder = new SvxOutlinerForwarder( *mpOutliner );
    }

    if( mpObject && !mbDataValid )
    {
        OutlinerParaObject* mpOutlinerParaObject = NULL;
        BOOL bTextEditActive = FALSE;
        SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, mpObject );
        if( pTextObj )
            mpOutlinerParaObject = pTextObj->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

        if( mpOutlinerParaObject )
            bTextEditActive = TRUE; // text edit active
        else
            mpOutlinerParaObject = mpObject->GetOutlinerParaObject();

        if( mpOutlinerParaObject && ( bTextEditActive || !mpObject->IsEmptyPresObj() || mpObject->GetPage()->IsMasterPage() ) )
        {
            mpOutliner->SetText( *mpOutlinerParaObject );

//          if( mpObject->IsEmptyPresObj() )
//              mpObject->SetEmptyPresObj( FALSE );
        }
        else
        {
            sal_Bool bVertical = mpOutlinerParaObject ? mpOutlinerParaObject->IsVertical() : sal_False;

            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = (SfxStyleSheetPool*)mpObject->GetModel()->GetStyleSheetPool();
            if( pPool )
                mpOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = mpObject->GetPage()->GetTextStyleSheetForObject( mpObject );
            if( pStyleSheet )
                mpOutliner->SetStyleSheet( 0, pStyleSheet );

            if( bVertical )
                mpOutliner->SetVertical( sal_True );
        }

        // evtually we have to set the border attributes
        if (mpOutliner->GetParagraphCount()==1)
        {
            // if we only have one paragraph we check if it is empty
            XubString aStr( mpOutliner->GetText( mpOutliner->GetParagraph( 0 ) ) );

            if(!aStr.Len())
            {
                // its empty, so we have to force the outliner to initialise itself
                mpOutliner->SetText( String(), mpOutliner->GetParagraph( 0 ) );

                if(mpObject->GetStyleSheet())
                    mpOutliner->SetStyleSheet( 0, mpObject->GetStyleSheet());

                // Beim setzen der harten Attribute an den ersten Absatz muss
                // der Parent pOutlAttr (=die Vorlage) temporaer entfernt
                // werden, da sonst bei SetParaAttribs() auch alle in diesem
                // Parent enthaltenen Items hart am Absatz attributiert werden.
                // -> BugID 22467
                const SfxItemSet& rSet = mpObject->GetItemSet();
                SdrOutlinerSetItem aOutlSetItem(rSet.GetPool());
                aOutlSetItem.GetItemSet().Put(rSet);
                const SfxItemSet* pTmpSet = &aOutlSetItem.GetItemSet();
                const SfxItemSet* pParentMerk = pTmpSet->GetParent();
                ((SfxItemSet*)pTmpSet)->SetParent(NULL);
                mpOutliner->SetParaAttribs(0,*pTmpSet);
                ((SfxItemSet*)pTmpSet)->SetParent(pParentMerk);
            }
        }

        mbDataValid = TRUE;
    }

    return mpTextForwarder;
}

//------------------------------------------------------------------------

void SvxTextEditSourceImpl::UpdateData()
{
    if( mbIsLocked )
    {
        mbNeedsUpdate = sal_True;
    }
    else
    {
        if( mpOutliner && mpObject && !mbDestroyed )
        {
            if( mpOutliner->GetParagraphCount() != 1 || mpOutliner->GetEditEngine().GetTextLen( 0 ) )
                mpObject->SetOutlinerParaObject( mpOutliner->CreateParaObject() );
            else
                mpObject->SetOutlinerParaObject( NULL );

            if( mpObject->IsEmptyPresObj() )
                mpObject->SetEmptyPresObj(sal_False);
        }
    }
}

void SvxTextEditSourceImpl::lock()
{
    mbIsLocked = sal_True;
    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_False );
        mbOldUndoMode = ((EditEngine*)&(mpOutliner->GetEditEngine()))->IsUndoEnabled();
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( sal_False );
    }
}

void SvxTextEditSourceImpl::unlock()
{
    mbIsLocked = sal_False;

    if( mbNeedsUpdate )
    {
        UpdateData();
        mbNeedsUpdate = sal_False;
    }

    if( mpOutliner )
    {
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->SetUpdateMode( sal_True );
        ((EditEngine*)&(mpOutliner->GetEditEngine()))->EnableUndo( mbOldUndoMode );
    }
}

//------------------------------------------------------------------------

// --------------------------------------------------------------------
// SvxTextEditSource
// --------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject )
{
    mpImpl = new SvxTextEditSourceImpl( pObject );
    mpImpl->acquire();
}

// --------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SvxTextEditSourceImpl* pImpl )
{
    mpImpl = pImpl;
    mpImpl->acquire();
}

//------------------------------------------------------------------------
SvxTextEditSource::~SvxTextEditSource()
{
    OGuard aGuard( Application::GetSolarMutex() );

    mpImpl->release();
}

//------------------------------------------------------------------------
SvxEditSource* SvxTextEditSource::Clone() const
{
    return new SvxTextEditSource( mpImpl );
}

//------------------------------------------------------------------------
SvxTextForwarder* SvxTextEditSource::GetTextForwarder()
{
    if( mpImpl )
        return mpImpl->GetTextForwarder();
    else
        return NULL;
}

//------------------------------------------------------------------------
void SvxTextEditSource::UpdateData()
{
    if( mpImpl )
        mpImpl->UpdateData();
}

SdrObject* SvxTextEditSource::GetSdrObject() const
{
    if( mpImpl )
        return mpImpl->GetSdrObject();
    else
        return NULL;
}

void SvxTextEditSource::lock()
{
    if( mpImpl )
        mpImpl->lock();
}

void SvxTextEditSource::unlock()
{
    if( mpImpl )
        mpImpl->unlock();
}

