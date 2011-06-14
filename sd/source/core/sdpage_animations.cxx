/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <editeng/outliner.hxx>
#include "CustomAnimationCloner.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <CustomAnimationPreset.hxx>
#include <TransitionPreset.hxx>
#include "undoanim.hxx"
#include "EffectMigration.hxx"

using ::rtl::OUString;
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
            mpMainSequence->disposeShape( xShape );
    }
}

bool SdPage::hasAnimationNode() const
{
    return mxAnimationNode.is();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
