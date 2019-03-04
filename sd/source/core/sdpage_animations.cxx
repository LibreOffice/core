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

#include <com/sun/star/animations/ParallelTimeContainer.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/outliner.hxx>
#include <CustomAnimationCloner.hxx>
#include <CustomAnimationEffect.hxx>
#include <sdpage.hxx>
#include <EffectMigration.hxx>

using namespace ::sd;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

using ::com::sun::star::drawing::XShape;

/** returns a helper class to manipulate effects inside the main sequence */
std::shared_ptr< sd::MainSequence >  const & SdPage::getMainSequence()
{
    if (nullptr == mpMainSequence)
        mpMainSequence.reset( new sd::MainSequence( getAnimationNode() ) );

    return mpMainSequence;
}

/** returns the main animation node */
Reference< XAnimationNode > const & SdPage::getAnimationNode()
{
    if( !mxAnimationNode.is() )
    {
        mxAnimationNode.set( ParallelTimeContainer::create( ::comphelper::getProcessComponentContext() ), UNO_QUERY_THROW );
        Sequence< css::beans::NamedValue > aUserData
            { { "node-type", css::uno::makeAny(css::presentation::EffectNodeType::TIMING_ROOT) } };
        mxAnimationNode->setUserData( aUserData );
    }

    return mxAnimationNode;
}

void SdPage::setAnimationNode( Reference< XAnimationNode > const & xNode )
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

/** Notify that the object has been renamed and the animation effect has to update. */
void SdPage::notifyObjectRenamed(const SdrObject* pObj)
{
    if (pObj && hasAnimationNode())
    {
        Reference<XShape> xShape(const_cast<SdrObject*>(pObj)->getUnoShape(), UNO_QUERY);

        if (xShape.is() && getMainSequence()->hasEffect(xShape))
            getMainSequence()->notify_change();
    }
}

bool SdPage::hasAnimationNode() const
{
    return mxAnimationNode.is();
}

void SdPage::SetFadeEffect(css::presentation::FadeEffect eNewEffect)
{
    EffectMigration::SetFadeEffect( this, eNewEffect );
}

FadeEffect SdPage::GetFadeEffect() const
{
    return EffectMigration::GetFadeEffect( this );
}

/** callback from the sd::View when a new paragraph for one object on this page is created */
void SdPage::onParagraphInserted( ::Outliner* pOutliner, Paragraph const * pPara, SdrObject* pObj )
{
    if( mxAnimationNode.is() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape.set( pObj->getUnoShape(), UNO_QUERY );
        /* FIXME: Paragraph should be sal_Int32, though more than 64k
         * paragrapsh at a shape are unlikely.. */
        aTarget.Paragraph = static_cast<sal_Int16>(pOutliner->GetAbsPos( pPara ));

        getMainSequence()->insertTextRange( makeAny( aTarget ) );
    }
}

/** callback from the sd::View when a paragraph from one object on this page is removed */
void SdPage::onParagraphRemoving( ::Outliner* pOutliner, Paragraph const * pPara, SdrObject* pObj )
{
    if( mxAnimationNode.is() )
    {
        ParagraphTarget aTarget;
        aTarget.Shape.set( pObj->getUnoShape(), UNO_QUERY );
        /* FIXME: Paragraph should be sal_Int32, though more than 64k
         * paragrapsh at a shape are unlikely.. */
        aTarget.Paragraph = static_cast<sal_Int16>(pOutliner->GetAbsPos( pPara ));

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
