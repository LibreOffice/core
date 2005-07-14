/*************************************************************************
 *
 *  $RCSfile: undoanim.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:44:25 $
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

#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _SD_CUSTOMANIMATIONCLONER_HXX
#include "CustomAnimationCloner.hxx"
#endif

#include "undoanim.hxx"
#include "glob.hrc"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "CustomAnimationEffect.hxx"
#include "drawdoc.hxx"

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::util::XCloneable;
using namespace ::com::sun::star::animations;


namespace sd
{

struct UndoAnimationImpl
{
    SdPage*         mpPage;
    Reference< XAnimationNode > mxOldNode;
    Reference< XAnimationNode > mxNewNode;
    bool            mbNewNodeSet;
};

UndoAnimation::UndoAnimation( SdDrawDocument* pDoc, SdPage* pThePage )
: SdrUndoAction( *pDoc ), mpImpl( new UndoAnimationImpl )
{
    mpImpl->mpPage = pThePage;
    mpImpl->mbNewNodeSet = false;

    try
    {
        if( pThePage->mxAnimationNode.is() )
            mpImpl->mxOldNode = sd::Clone( pThePage->getAnimationNode() );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::UndoAnimation::UndoAnimation(), exception caught!");
    }
}

UndoAnimation::~UndoAnimation()
{
    delete mpImpl;
}

void UndoAnimation::Undo()
{
    try
    {
        if( !mpImpl->mbNewNodeSet )
        {
            if( mpImpl->mpPage->mxAnimationNode.is() )
                mpImpl->mxNewNode.set( sd::Clone( mpImpl->mpPage->mxAnimationNode ) );
            mpImpl->mbNewNodeSet = true;
        }

        Reference< XAnimationNode > xOldNode;
        if( mpImpl->mxOldNode.is() )
            xOldNode = sd::Clone( mpImpl->mxOldNode );

        mpImpl->mpPage->setAnimationNode( xOldNode );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::UndoAnimation::Undo(), exception caught!");
    }
}

void UndoAnimation::Redo()
{
    try
    {
        Reference< XAnimationNode > xNewNode;
        if( mpImpl->mxNewNode.is() )
            xNewNode = sd::Clone( mpImpl->mxNewNode );
        mpImpl->mpPage->setAnimationNode( xNewNode );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("sd::UndoAnimation::Redo(), exception caught!");
    }
}

void UndoAnimation::Repeat()
{
}

String UndoAnimation::GetComment() const
{
    return String(SdResId(STR_UNDO_ANIMATION));
}

BOOL UndoAnimation::CanRepeat(SfxRepeatTarget&) const
{
    return FALSE;
}


struct UndoTransitionImpl
{
    SdPage*         mpPage;

    sal_Int16 mnNewTransitionType;
    sal_Int16 mnNewTransitionSubtype;
    sal_Bool mbNewTransitionDirection;
    sal_Int32 mnNewTransitionFadeColor;
    double mfNewTransitionDuration;

    sal_Int16 mnOldTransitionType;
    sal_Int16 mnOldTransitionSubtype;
    sal_Bool mbOldTransitionDirection;
    sal_Int32 mnOldTransitionFadeColor;
    double mfOldTransitionDuration;
};

UndoTransition::UndoTransition( SdDrawDocument* pDoc, SdPage* pThePage )
: SdUndoAction( pDoc ), mpImpl( new UndoTransitionImpl )
{
    mpImpl->mpPage = pThePage;

    mpImpl->mnNewTransitionType = -1;
    mpImpl->mnOldTransitionType = pThePage->mnTransitionType;
    mpImpl->mnOldTransitionSubtype = pThePage->mnTransitionSubtype;
    mpImpl->mbOldTransitionDirection = pThePage->mbTransitionDirection;
    mpImpl->mnOldTransitionFadeColor = pThePage->mnTransitionFadeColor;
    mpImpl->mfOldTransitionDuration = pThePage->mfTransitionDuration;
}

UndoTransition::~UndoTransition()
{
    delete mpImpl;
}

void UndoTransition::Undo()
{
    if( mpImpl->mnNewTransitionType == -1 )
    {
        mpImpl->mnNewTransitionType = mpImpl->mpPage->mnTransitionType;
        mpImpl->mnNewTransitionSubtype = mpImpl->mpPage->mnTransitionSubtype;
        mpImpl->mbNewTransitionDirection = mpImpl->mpPage->mbTransitionDirection;
        mpImpl->mnNewTransitionFadeColor = mpImpl->mpPage->mnTransitionFadeColor;
        mpImpl->mfNewTransitionDuration = mpImpl->mpPage->mfTransitionDuration;
    }

    mpImpl->mpPage->mnTransitionType = mpImpl->mnOldTransitionType;
    mpImpl->mpPage->mnTransitionSubtype = mpImpl->mnOldTransitionSubtype;
    mpImpl->mpPage->mbTransitionDirection = mpImpl->mbOldTransitionDirection;
    mpImpl->mpPage->mnTransitionFadeColor = mpImpl->mnOldTransitionFadeColor;
    mpImpl->mpPage->mfTransitionDuration = mpImpl->mfOldTransitionDuration;
}

void UndoTransition::Redo()
{
    mpImpl->mpPage->mnTransitionType = mpImpl->mnNewTransitionType;
    mpImpl->mpPage->mnTransitionSubtype = mpImpl->mnNewTransitionSubtype;
    mpImpl->mpPage->mbTransitionDirection = mpImpl->mbNewTransitionDirection;
    mpImpl->mpPage->mnTransitionFadeColor = mpImpl->mnNewTransitionFadeColor;
    mpImpl->mpPage->mfTransitionDuration = mpImpl->mfNewTransitionDuration;
}

void UndoTransition::Repeat()
{
}

String UndoTransition::GetComment() const
{
    return String(SdResId(STR_UNDO_SLIDE_PARAMS));
}

BOOL UndoTransition::CanRepeat(SfxRepeatTarget&) const
{
    return FALSE;
}

}
