/*************************************************************************
 *
 *  $RCSfile: sdpage_animations.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:44:13 $
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

#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif

#ifndef _SD_CUSTOMANIMATIONCLONER_HXX
#include "CustomAnimationCloner.hxx"
#endif

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif

#ifndef _SD_CUSTOMANIMATIONPRESET_HXX
#include <CustomAnimationPreset.hxx>
#endif

#ifndef _SD_TRANSITIONPRESET_HXX
#include <TransitionPreset.hxx>
#endif

#ifndef _SD_UNDO_ANIM_HXX
#include "undoanim.hxx"
#endif

#ifndef _SD_EFFECT_MIGRATION_HXX
#include "EffectMigration.hxx"
#endif

using namespace ::vos;
using namespace ::rtl;
using namespace ::sd;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::drawing::XShape;

/** returns a helper class to manipulate effects inside the main sequence */
sd::MainSequencePtr SdPage::getMainSequence()
{
    if( 0 == mpMainSequence.get() )
        mpMainSequence.reset( new sd::MainSequence( getAnimationNode() ) );

    return mpMainSequence;
}

/** returns the main animation node */
Reference< XAnimationNode > SdPage::getAnimationNode() throw (RuntimeException)
{
    if( !mxAnimationNode.is() )
    {
        mxAnimationNode = Reference< XAnimationNode >::query(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.animations.ParallelTimeContainer"))));
        if( mxAnimationNode.is() )
        {
            Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
            aUserData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "node-type" ) );
            aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT;
            mxAnimationNode->setUserData( aUserData );
        }
    }

    return mxAnimationNode;
}

void SdPage::setAnimationNode( Reference< XAnimationNode >& xNode ) throw (RuntimeException)
{
    mxAnimationNode = xNode;
    if( mpMainSequence.get() )
        mpMainSequence->reset( xNode );
}

/** removes all custom animations for the given shape */
void SdPage::removeAnimations( const SdrObject* pObj )
{
    if( mxAnimationNode.is() )
    {
        getMainSequence();

        Reference< XShape > xShape( const_cast<SdrObject*>(pObj)->getUnoShape(), UNO_QUERY );

        if( mpMainSequence->hasEffect( xShape ) )
        {
            SdDrawDocument* pDoc = static_cast<SdDrawDocument*>(GetModel());

            // since this is also called from a redo action, make sure we
            // are recording undo actions anyway
            SdrUndoGroup* pGroup = pDoc ? const_cast< SdrUndoGroup*  >( pDoc->GetAktUndoGroup() ) : 0;

            if( pGroup )
            {
                bool bAdd = true;
                if( pGroup->GetActionCount() == 0 )
                {
                    bAdd = true;
                }
                else
                {
                    UndoAnimation* pAnim = dynamic_cast< UndoAnimation* >( pGroup->GetAction( 0 ) );
                    bAdd = pAnim == 0;
                }

                if( bAdd )
                    pGroup->push_front( new UndoAnimation( pDoc, this ) );
            }

            mpMainSequence->disposeShape( xShape );
        }
    }
}

void SdPage::SetFadeEffect(::com::sun::star::presentation::FadeEffect eNewEffect)
{
    EffectMigration::SetFadeEffect( this, eNewEffect );
}

FadeEffect SdPage::GetFadeEffect() const
{
    return EffectMigration::GetFadeEffect( this );
}

/** callback from the sd::View when a new paragraph for one object on this page is created */
void SdPage::onParagraphInserted( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj )
{
    if( mxAnimationNode.is() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape = Reference< XShape >( pObj->getUnoShape(), UNO_QUERY );
        aTarget.Paragraph = (sal_Int16)pOutliner->GetAbsPos( pPara );

        getMainSequence()->insertTextRange( makeAny( aTarget ) );
    }
}

/** callback from the sd::View when a paragraph from one object on this page is removed */
void SdPage::onParagraphRemoving( ::Outliner* pOutliner, Paragraph* pPara, SdrObject* pObj )
{
    if( mxAnimationNode.is() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape = Reference< XShape >( pObj->getUnoShape(), UNO_QUERY );
        aTarget.Paragraph = (sal_Int16)pOutliner->GetAbsPos( pPara );

        getMainSequence()->disposeTextRange( makeAny( aTarget ) );
    }
}

/** callback from the sd::View when an object just left text edit mode */
void SdPage::onEndTextEdit( SdrObject* pObj )
{
    if( pObj && mxAnimationNode.is() )
    {
        Reference< XShape > xObj( pObj->getUnoShape(), UNO_QUERY );
        getMainSequence()->onTextChanged( xObj );
    }
}

void SdPage::cloneAnimations( SdPage& rTargetPage ) const
{
    if( mxAnimationNode.is() )
    {
        Reference< XAnimationNode > xClonedNode(
            ::sd::Clone( mxAnimationNode, this, &rTargetPage ) );

        if( xClonedNode.is() )
            rTargetPage.setAnimationNode( xClonedNode );
    }
}
