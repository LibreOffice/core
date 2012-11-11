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

#include <tools/debug.hxx>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <algorithm>

#include <cppuhelper/implbase1.hxx>

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include "CustomAnimationEffect.hxx"
#include <CustomAnimationPreset.hxx>
#include "animations.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::animations;

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XChild;
using ::com::sun::star::container::XElementAccess;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::lang::XInitialization;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::util::XChangesNotifier;
using ::com::sun::star::util::XChangesListener;

namespace sd
{
class MainSequenceChangeGuard
{
public:
    MainSequenceChangeGuard( EffectSequenceHelper* pSequence )
    {
        mpMainSequence = dynamic_cast< MainSequence* >( pSequence );
        if( mpMainSequence == 0 )
        {
            InteractiveSequence* pI = dynamic_cast< InteractiveSequence* >( pSequence );
            if( pI )
                mpMainSequence = pI->mpMainSequence;
        }
        DBG_ASSERT( mpMainSequence, "sd::MainSequenceChangeGuard::MainSequenceChangeGuard(), no main sequence to guard!" );

        if( mpMainSequence )
            mpMainSequence->mbIgnoreChanges++;
    }

    ~MainSequenceChangeGuard()
    {
        if( mpMainSequence )
            mpMainSequence->mbIgnoreChanges++;
    }

private:
    MainSequence* mpMainSequence;
};

CustomAnimationEffect::CustomAnimationEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
:   mnNodeType(-1),
    mnPresetClass(-1),
    mfBegin(-1.0),
    mfDuration(-1.0),
    mfAbsoluteDuration(-1.0),
    mnGroupId(-1),
    mnIterateType(0),
    mfIterateInterval(0.0),
    mnParaDepth( -1 ),
    mbHasText(sal_False),
    mfAcceleration( 1.0 ),
    mfDecelerate( 1.0 ),
    mbAutoReverse(false),
    mnTargetSubItem(0),
    mnCommand(0),
    mpEffectSequence( 0 ),
    mbHasAfterEffect(false),
    mbAfterEffectOnNextEffect(false)
{
    setNode( xNode );
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    mxNode = xNode;
    mxAudio.clear();

    Sequence< NamedValue > aUserData( mxNode->getUserData() );
    sal_Int32 nLength = aUserData.getLength();
    const NamedValue* p = aUserData.getConstArray();

    while( nLength-- )
    {
        if ( p->Name == "node-type" )
        {
            p->Value >>= mnNodeType;
        }
        else if ( p->Name == "preset-id" )
        {
            p->Value >>= maPresetId;
        }
        else if ( p->Name == "preset-sub-type" )
        {
            p->Value >>= maPresetSubType;
        }
        else if ( p->Name == "preset-class" )
        {
            p->Value >>= mnPresetClass;
        }
        else if ( p->Name == "preset-property" )
        {
            p->Value >>= maProperty;
        }
        else if ( p->Name == "group-id" )
        {
            p->Value >>= mnGroupId;
        }

        p++;
    }

    // get effect start time
    mxNode->getBegin() >>= mfBegin;

    mfAcceleration = mxNode->getAcceleration();
    mfDecelerate = mxNode->getDecelerate();
    mbAutoReverse = mxNode->getAutoReverse();

    // get iteration data
    Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );
    if( xIter.is() )
    {
        mfIterateInterval = xIter->getIterateInterval();
        mnIterateType = xIter->getIterateType();
        maTarget = xIter->getTarget();
        mnTargetSubItem = xIter->getSubItem();
    }
    else
    {
        mfIterateInterval = 0.0f;
        mnIterateType = 0;
    }

    // calculate effect duration and get target shape
    Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
    if( xEnumerationAccess.is() )
    {
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
        if( xEnumeration.is() )
        {
            while( xEnumeration->hasMoreElements() )
            {
                Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                if( !xChildNode.is() )
                    continue;

                if( xChildNode->getType() == AnimationNodeType::AUDIO )
                {
                    mxAudio.set( xChildNode, UNO_QUERY );
                }
                else if( xChildNode->getType() == AnimationNodeType::COMMAND )
                {
                    Reference< XCommand > xCommand( xChildNode, UNO_QUERY );
                    if( xCommand.is() )
                    {
                        mnCommand = xCommand->getCommand();
                        if( !maTarget.hasValue() )
                            maTarget = xCommand->getTarget();
                    }
                }
                else
                {
                    double fBegin = 0.0;
                    double fDuration = 0.0;
                    xChildNode->getBegin() >>= fBegin;
                    xChildNode->getDuration() >>= fDuration;

                    fDuration += fBegin;
                    if( fDuration > mfDuration )
                        mfDuration = fDuration;

                    // no target shape yet?
                    if( !maTarget.hasValue() )
                    {
                        // go get it boys!
                        Reference< XAnimate > xAnimate( xChildNode, UNO_QUERY );
                        if( xAnimate.is() )
                        {
                            maTarget = xAnimate->getTarget();
                            mnTargetSubItem = xAnimate->getSubItem();
                        }
                    }
                }
            }
        }
    }

    mfAbsoluteDuration = mfDuration;
    double fRepeatCount = 1.0;
    if( (mxNode->getRepeatCount()) >>= fRepeatCount )
        mfAbsoluteDuration *= fRepeatCount;

    checkForText();
}

// --------------------------------------------------------------------

sal_Int32 CustomAnimationEffect::getNumberOfSubitems( const Any& aTarget, sal_Int16 nIterateType )
{
    sal_Int32 nSubItems = 0;

    try
    {
        // first get target text
        sal_Int32 nOnlyPara = -1;

        Reference< XText > xShape;
        aTarget >>= xShape;
        if( !xShape.is() )
        {
            ParagraphTarget aParaTarget;
            if( aTarget >>= aParaTarget )
            {
                xShape.set( aParaTarget.Shape, UNO_QUERY );
                nOnlyPara = aParaTarget.Paragraph;
            }
        }

        // now use the break iterator to iterate over the given text
        // and count the sub items

        if( xShape.is() )
        {
            // TODO/LATER: Optimize this, don't create a break iterator each time
            Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            Reference < i18n::XBreakIterator > xBI = i18n::BreakIterator::create(xContext);

            Reference< XEnumerationAccess > xEA( xShape, UNO_QUERY_THROW );
            Reference< XEnumeration > xEnumeration( xEA->createEnumeration(), UNO_QUERY_THROW );
            Locale aLocale;
            const OUString aStrLocaleName( "CharLocale" );
            Reference< XTextRange > xParagraph;

            sal_Int32 nPara = 0;
            while( xEnumeration->hasMoreElements() )
            {
                xEnumeration->nextElement() >>= xParagraph;

                // skip this if its not the only paragraph we want to count
                if( (nOnlyPara != -1) && (nOnlyPara != nPara ) )
                    continue;

                if( nIterateType == TextAnimationType::BY_PARAGRAPH )
                {
                    nSubItems++;
                }
                else
                {
                    const OUString aText( xParagraph->getString() );
                    Reference< XPropertySet > xSet( xParagraph, UNO_QUERY_THROW );
                    xSet->getPropertyValue( aStrLocaleName ) >>= aLocale;

                    sal_Int32 nPos;
                    const sal_Int32 nEndPos = aText.getLength();

                    if( nIterateType == TextAnimationType::BY_WORD )
                    {
                        for( nPos = 0; nPos < nEndPos; nPos++ )
                        {
                            nPos = xBI->getWordBoundary(aText, nPos, aLocale, i18n::WordType::ANY_WORD, sal_True).endPos;
                            nSubItems++;
                        }
                        break;
                    }
                    else
                    {
                        sal_Int32 nDone;
                        for( nPos = 0; nPos < nEndPos; nPos++ )
                        {
                            nPos = xBI->nextCharacters(aText, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 0, nDone);
                            nSubItems++;
                        }
                    }
                }

                if( nPara == nOnlyPara )
                    break;

                nPara++;
            }
        }
    }
    catch( Exception& )
    {
        nSubItems = 0;
        OSL_FAIL( "sd::CustomAnimationEffect::getNumberOfSubitems(), exception cought!" );
    }

    return nSubItems;
}

// --------------------------------------------------------------------

CustomAnimationEffect::~CustomAnimationEffect()
{
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr CustomAnimationEffect::clone() const
{
    Reference< XCloneable > xCloneable( mxNode, UNO_QUERY_THROW );
    Reference< XAnimationNode > xNode( xCloneable->createClone(), UNO_QUERY_THROW );
    CustomAnimationEffectPtr pEffect( new CustomAnimationEffect( xNode ) );
    pEffect->setEffectSequence( getEffectSequence() );
    return pEffect;
}

// --------------------------------------------------------------------

sal_Int32 CustomAnimationEffect::get_node_type( const Reference< XAnimationNode >& xNode )
{
    sal_Int16 nNodeType = -1;

    if( xNode.is() )
    {
        Sequence< NamedValue > aUserData( xNode->getUserData() );
        sal_Int32 nLength = aUserData.getLength();
        if( nLength )
        {
            const NamedValue* p = aUserData.getConstArray();
            while( nLength-- )
            {
                if ( p->Name == "node-type" )
                {
                    p->Value >>= nNodeType;
                    break;
                }
                p++;
            }
        }
    }

    return nNodeType;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setPresetClass( sal_Int16 nPresetClass )
{
    if( mnPresetClass != nPresetClass )
    {
        mnPresetClass = nPresetClass;
        if( mxNode.is() )
        {
            // first try to find a "preset-class" entry in the user data
            // and change it
            Sequence< NamedValue > aUserData( mxNode->getUserData() );
            sal_Int32 nLength = aUserData.getLength();
            bool bFound = false;
            if( nLength )
            {
                NamedValue* p = aUserData.getArray();
                while( nLength-- )
                {
                    if ( p->Name == "preset-class" )
                    {
                        p->Value <<= mnPresetClass;
                        bFound = true;
                        break;
                    }
                    p++;
                }
            }

            // no "node-type" entry inside user data, so add it
            if( !bFound )
            {
                nLength = aUserData.getLength();
                aUserData.realloc( nLength + 1);
                aUserData[nLength].Name = "preset-class";
                aUserData[nLength].Value <<= mnPresetClass;
            }

            mxNode->setUserData( aUserData );
        }
    }
}

void CustomAnimationEffect::setNodeType( sal_Int16 nNodeType )
{
    if( mnNodeType != nNodeType )
    {
        mnNodeType = nNodeType;
        if( mxNode.is() )
        {
            // first try to find a "node-type" entry in the user data
            // and change it
            Sequence< NamedValue > aUserData( mxNode->getUserData() );
            sal_Int32 nLength = aUserData.getLength();
            bool bFound = false;
            if( nLength )
            {
                NamedValue* p = aUserData.getArray();
                while( nLength-- )
                {
                    if ( p->Name == "node-type" )
                    {
                        p->Value <<= mnNodeType;
                        bFound = true;
                        break;
                    }
                    p++;
                }
            }

            // no "node-type" entry inside user data, so add it
            if( !bFound )
            {
                nLength = aUserData.getLength();
                aUserData.realloc( nLength + 1);
                aUserData[nLength].Name = "node-type";
                aUserData[nLength].Value <<= mnNodeType;
            }

            mxNode->setUserData( aUserData );
        }
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setGroupId( sal_Int32 nGroupId )
{
    mnGroupId = nGroupId;
    if( mxNode.is() )
    {
        // first try to find a "group-id" entry in the user data
        // and change it
        Sequence< NamedValue > aUserData( mxNode->getUserData() );
        sal_Int32 nLength = aUserData.getLength();
        bool bFound = false;
        if( nLength )
        {
            NamedValue* p = aUserData.getArray();
            while( nLength-- )
            {
                if ( p->Name == "group-id" )
                {
                    p->Value <<= mnGroupId;
                    bFound = true;
                    break;
                }
                p++;
            }
        }

        // no "node-type" entry inside user data, so add it
        if( !bFound )
        {
            nLength = aUserData.getLength();
            aUserData.realloc( nLength + 1);
            aUserData[nLength].Name = "group-id";
            aUserData[nLength].Value <<= mnGroupId;
        }

        mxNode->setUserData( aUserData );
    }
}

// --------------------------------------------------------------------

/** checks if the text for this effect has changed and updates internal flags.
    returns true if something changed.
*/
bool CustomAnimationEffect::checkForText()
{
    bool bChange = false;

    Reference< XText > xText;

    if( maTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
    {
        // calc para depth
        ParagraphTarget aParaTarget;
        maTarget >>= aParaTarget;

        xText = Reference< XText >::query( aParaTarget.Shape );

        // get paragraph
        if( xText.is() )
        {
            Reference< XEnumerationAccess > xEA( xText, UNO_QUERY );
            if( xEA.is() )
            {
                Reference< XEnumeration > xEnumeration( xEA->createEnumeration(), UNO_QUERY );
                if( xEnumeration.is() )
                {
                    sal_Bool bHasText = xEnumeration->hasMoreElements();
                    bChange |= bHasText != mbHasText;
                    mbHasText = bHasText;

                    sal_Int32 nPara = aParaTarget.Paragraph;

                    while( xEnumeration->hasMoreElements() && nPara-- )
                        xEnumeration->nextElement();

                    if( xEnumeration->hasMoreElements() )
                    {
                        Reference< XPropertySet > xParaSet;
                        xEnumeration->nextElement() >>= xParaSet;
                        if( xParaSet.is() )
                        {
                            sal_Int32 nParaDepth = 0;
                            const OUString strNumberingLevel( "NumberingLevel" );
                            xParaSet->getPropertyValue( strNumberingLevel ) >>= nParaDepth;
                            bChange |= nParaDepth != mnParaDepth;
                            mnParaDepth = nParaDepth;
                        }
                    }
                }
            }
        }
    }
    else
    {
        maTarget >>= xText;
        sal_Bool bHasText = xText.is() && !xText->getString().isEmpty();
        bChange |= bHasText != mbHasText;
        mbHasText = bHasText;
    }

    bChange |= calculateIterateDuration();
    return bChange;
}

bool CustomAnimationEffect::calculateIterateDuration()
{
    bool bChange = false;

    // if we have an iteration, we must also calculate the
    // 'true' container duration, that is
    // ( ( is form animated ) ? [contained effects duration] : 0 ) +
    // ( [number of animated children] - 1 ) * [interval-delay] + [contained effects duration]
    Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );
    if( xIter.is() )
    {
        double fDuration = mfDuration;
        const double fSubEffectDuration = mfDuration;

        if( mnTargetSubItem != ShapeAnimationSubType::ONLY_BACKGROUND ) // does not make sense for iterate container but better check
        {
            const sal_Int32 nSubItems = getNumberOfSubitems( maTarget, mnIterateType );
            if( nSubItems )
            {
                const double f = (nSubItems-1) * mfIterateInterval;
                fDuration += f;
            }
        }

        // if we also animate the form first, we have to add the
        // sub effect duration to the whole effect duration
        if( mnTargetSubItem == ShapeAnimationSubType::AS_WHOLE )
            fDuration += fSubEffectDuration;

        bChange |= fDuration != mfAbsoluteDuration;
        mfAbsoluteDuration = fDuration;
    }

    return bChange;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setTarget( const ::com::sun::star::uno::Any& rTarget )
{
    try
    {
        maTarget = rTarget;

        // first, check special case for random node
        Reference< XInitialization > xInit( mxNode, UNO_QUERY );
        if( xInit.is() )
        {
            const Sequence< Any > aArgs( &maTarget, 1 );
            xInit->initialize( aArgs );
        }
        else
        {
            Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );
            if( xIter.is() )
            {
                xIter->setTarget(maTarget);
            }
            else
            {
                Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
                if( xEnumerationAccess.is() )
                {
                    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
                    if( xEnumeration.is() )
                    {
                        while( xEnumeration->hasMoreElements() )
                        {
                            const Any aElem( xEnumeration->nextElement() );
                            Reference< XAnimate > xAnimate( aElem, UNO_QUERY );
                            if( xAnimate.is() )
                                xAnimate->setTarget( rTarget );
                            else
                            {
                                Reference< XCommand > xCommand( aElem, UNO_QUERY );
                                if( xCommand.is() )
                                    xCommand->setTarget( rTarget );
                            }
                        }
                    }
                }
            }
        }
        checkForText();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setTarget(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setTargetSubItem( sal_Int16 nSubItem )
{
    try
    {
        mnTargetSubItem = nSubItem;

        Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );
        if( xIter.is() )
        {
            xIter->setSubItem(mnTargetSubItem);
        }
        else
        {
            Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
            if( xEnumerationAccess.is() )
            {
                Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
                if( xEnumeration.is() )
                {
                    while( xEnumeration->hasMoreElements() )
                    {
                        Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                        if( xAnimate.is() )
                            xAnimate->setSubItem( mnTargetSubItem );
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setTargetSubItem(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setDuration( double fDuration )
{
    if( (mfDuration != -1.0) && (mfDuration != fDuration) ) try
    {
        double fScale = fDuration / mfDuration;
        mfDuration = fDuration;
        double fRepeatCount = 1.0;
        getRepeatCount() >>= fRepeatCount;
        mfAbsoluteDuration = mfDuration * fRepeatCount;

        // calculate effect duration and get target shape
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() )
                {
                    Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xChildNode.is() )
                        continue;


                    double fChildBegin = 0.0;
                    xChildNode->getBegin() >>= fChildBegin;
                    if(  fChildBegin != 0.0 )
                    {
                        fChildBegin *= fScale;
                        xChildNode->setBegin( makeAny( fChildBegin ) );
                    }

                    double fChildDuration = 0.0;
                    xChildNode->getDuration() >>= fChildDuration;
                    if( fChildDuration != 0.0 )
                    {
                        fChildDuration *= fScale;
                        xChildNode->setDuration( makeAny( fChildDuration ) );
                    }
                }
            }
        }
        calculateIterateDuration();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setDuration(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setBegin( double fBegin )
{
    if( mxNode.is() ) try
    {
        mfBegin = fBegin;
        mxNode->setBegin( makeAny( fBegin ) );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setBegin(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setAcceleration( double fAcceleration )
{
    if( mxNode.is() ) try
    {
        mfAcceleration = fAcceleration;
        mxNode->setAcceleration( fAcceleration );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setAcceleration(), exception cought!" );
    }
}
// --------------------------------------------------------------------

void CustomAnimationEffect::setDecelerate( double fDecelerate )
{
    if( mxNode.is() ) try
    {
        mfDecelerate = fDecelerate;
        mxNode->setDecelerate( fDecelerate );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setDecelerate(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setAutoReverse( sal_Bool bAutoReverse )
{
    if( mxNode.is() ) try
    {
        mbAutoReverse = bAutoReverse;
        mxNode->setAutoReverse( bAutoReverse );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setAutoReverse(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::replaceNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    sal_Int16 nNodeType = mnNodeType;
    Any aTarget = maTarget;

    double fBegin = mfBegin;
    double fDuration = mfDuration;
    double fAcceleration = mfAcceleration;
    double fDecelerate = mfDecelerate ;
    sal_Bool bAutoReverse = mbAutoReverse;
    Reference< XAudio > xAudio( mxAudio );
    sal_Int16 nIterateType = mnIterateType;
    double fIterateInterval = mfIterateInterval;
    sal_Int16 nSubItem = mnTargetSubItem;

    setNode( xNode );

    setAudio( xAudio );
    setNodeType( nNodeType );
    setTarget( aTarget );
    setTargetSubItem( nSubItem );
    setDuration( fDuration );
    setBegin( fBegin );

    setAcceleration( fAcceleration );
    setDecelerate( fDecelerate );
    setAutoReverse( bAutoReverse );

    if( nIterateType != mnIterateType )
        setIterateType( nIterateType );

    if( mnIterateType && ( fIterateInterval != mfIterateInterval ) )
        setIterateInterval( fIterateInterval );
}

// --------------------------------------------------------------------

Reference< XShape > CustomAnimationEffect::getTargetShape() const
{
    Reference< XShape > xShape;
    maTarget >>= xShape;
    if( !xShape.is() )
    {
        ParagraphTarget aParaTarget;
        if( maTarget >>= aParaTarget )
            xShape = aParaTarget.Shape;
    }

    return xShape;
}

// --------------------------------------------------------------------

Any CustomAnimationEffect::getRepeatCount() const
{
    if( mxNode.is() )
    {
        return mxNode->getRepeatCount();
    }
    else
    {
        Any aAny;
        return aAny;
    }
}

// --------------------------------------------------------------------

Any CustomAnimationEffect::getEnd() const
{
    if( mxNode.is() )
    {
        return mxNode->getEnd();
    }
    else
    {
        Any aAny;
        return aAny;
    }
}

// --------------------------------------------------------------------

sal_Int16 CustomAnimationEffect::getFill() const
{
    if( mxNode.is() )
        return mxNode->getFill();
    else
        return 0;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setRepeatCount( const Any& rRepeatCount )
{
    if( mxNode.is() )
    {
        mxNode->setRepeatCount( rRepeatCount );
        double fRepeatCount = 1.0;
        rRepeatCount >>= fRepeatCount;
        mfAbsoluteDuration = mfDuration * fRepeatCount;
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setEnd( const Any& rEnd )
{
    if( mxNode.is() )
        mxNode->setEnd( rEnd );
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setFill( sal_Int16 nFill )
{
    if( mxNode.is() )
        mxNode->setFill( nFill );
}

// --------------------------------------------------------------------

Reference< XAnimationNode > CustomAnimationEffect::createAfterEffectNode() const throw (Exception)
{
    DBG_ASSERT( mbHasAfterEffect, "sd::CustomAnimationEffect::createAfterEffectNode(), this node has no after effect!" );

    Reference< XMultiServiceFactory > xMsf( ::comphelper::getProcessServiceFactory() );

    const char* pServiceName = maDimColor.hasValue() ?
        "com.sun.star.animations.AnimateColor" : "com.sun.star.animations.AnimateSet";

    Reference< XAnimate > xAnimate( xMsf->createInstance(OUString::createFromAscii(pServiceName) ), UNO_QUERY_THROW );

    Any aTo;
    OUString aAttributeName;

    if( maDimColor.hasValue() )
    {
        aTo = maDimColor;
        aAttributeName = "DimColor";
    }
    else
    {
        aTo = makeAny( (sal_Bool)sal_False );
        aAttributeName = "Visibility";
    }

    Any aBegin;
    if( !mbAfterEffectOnNextEffect ) // sameClick
    {
        Event aEvent;

        aEvent.Source <<= getNode();
        aEvent.Trigger = EventTrigger::END_EVENT;
        aEvent.Repeat = 0;

        aBegin <<= aEvent;
    }
    else
    {
        aBegin <<= (double)0.0;
    }

    xAnimate->setBegin( aBegin );
    xAnimate->setTo( aTo );
    xAnimate->setAttributeName( aAttributeName );

    xAnimate->setDuration( makeAny( (double)0.001 ) );
    xAnimate->setFill( AnimationFill::HOLD );
    xAnimate->setTarget( maTarget );

    return Reference< XAnimationNode >( xAnimate, UNO_QUERY_THROW );
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setIterateType( sal_Int16 nIterateType )
{
    if( mnIterateType != nIterateType ) try
    {
        // do we need to exchange the container node?
        if( (mnIterateType == 0) || (nIterateType == 0) )
        {
            sal_Int16 nTargetSubItem = mnTargetSubItem;

            Reference< XMultiServiceFactory > xMsf( ::comphelper::getProcessServiceFactory() );
            const char * pServiceName =
                nIterateType ? "com.sun.star.animations.IterateContainer" : "com.sun.star.animations.ParallelTimeContainer";
            Reference< XTimeContainer > xNewContainer(
                xMsf->createInstance( OUString::createFromAscii(pServiceName) ), UNO_QUERY_THROW );

            Reference< XTimeContainer > xOldContainer( mxNode, UNO_QUERY_THROW );
            Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
            while( xEnumeration->hasMoreElements() )
            {
                Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
                xOldContainer->removeChild( xChildNode );
                xNewContainer->appendChild( xChildNode );
            }

            Reference< XAnimationNode > xNewNode( xNewContainer, UNO_QUERY_THROW );

            xNewNode->setBegin( mxNode->getBegin() );
            xNewNode->setDuration( mxNode->getDuration() );
            xNewNode->setEnd( mxNode->getEnd() );
            xNewNode->setEndSync( mxNode->getEndSync() );
            xNewNode->setRepeatCount( mxNode->getRepeatCount() );
            xNewNode->setFill( mxNode->getFill() );
            xNewNode->setFillDefault( mxNode->getFillDefault() );
            xNewNode->setRestart( mxNode->getRestart() );
            xNewNode->setRestartDefault( mxNode->getRestartDefault() );
            xNewNode->setAcceleration( mxNode->getAcceleration() );
            xNewNode->setDecelerate( mxNode->getDecelerate() );
            xNewNode->setAutoReverse( mxNode->getAutoReverse() );
            xNewNode->setRepeatDuration( mxNode->getRepeatDuration() );
            xNewNode->setEndSync( mxNode->getEndSync() );
            xNewNode->setRepeatCount( mxNode->getRepeatCount() );
            xNewNode->setUserData( mxNode->getUserData() );

            mxNode = xNewNode;

            Any aTarget;
            if( nIterateType )
            {
                Reference< XIterateContainer > xIter( mxNode, UNO_QUERY_THROW );
                xIter->setTarget(maTarget);
                xIter->setSubItem( nTargetSubItem );
            }
            else
            {
                aTarget = maTarget;
            }

            Reference< XEnumerationAccess > xEA( mxNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
            while( xE->hasMoreElements() )
            {
                Reference< XAnimate > xAnimate( xE->nextElement(), UNO_QUERY );
                if( xAnimate.is() )
                {
                    xAnimate->setTarget( aTarget );
                    xAnimate->setSubItem( nTargetSubItem );
                }
            }
        }

        mnIterateType = nIterateType;

        // if we have an iteration container, we must set its type
        if( mnIterateType )
        {
            Reference< XIterateContainer > xIter( mxNode, UNO_QUERY_THROW );
            xIter->setIterateType( nIterateType );
        }

        checkForText();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationEffect::setIterateType(), Exception cought!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setIterateInterval( double fIterateInterval )
{
    if( mfIterateInterval != fIterateInterval )
    {
        Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );

        DBG_ASSERT( xIter.is(), "sd::CustomAnimationEffect::setIterateInterval(), not an iteration node" );
        if( xIter.is() )
        {
            mfIterateInterval = fIterateInterval;
            xIter->setIterateInterval( fIterateInterval );
        }

        calculateIterateDuration();
    }
}

// --------------------------------------------------------------------

::rtl::OUString CustomAnimationEffect::getPath() const
{
    ::rtl::OUString aPath;

    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimateMotion > xMotion( xEnumeration->nextElement(), UNO_QUERY );
            if( xMotion.is() )
            {
                xMotion->getPath() >>= aPath;
                break;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::getPath(), exception cought!" );
    }

    return aPath;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setPath( const ::rtl::OUString& rPath )
{
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimateMotion > xMotion( xEnumeration->nextElement(), UNO_QUERY );
            if( xMotion.is() )
            {

                MainSequenceChangeGuard aGuard( mpEffectSequence );
                xMotion->setPath( Any( rPath ) );
                break;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setPath(), exception cought!" );
    }
}

// --------------------------------------------------------------------

Any CustomAnimationEffect::getProperty( sal_Int32 nNodeType, const OUString& rAttributeName, EValue eValue )
{
    Any aProperty;
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() && !aProperty.hasValue() )
                {
                    Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xAnimate.is() )
                        continue;

                    if( xAnimate->getType() == nNodeType )
                    {
                        if( xAnimate->getAttributeName() == rAttributeName )
                        {
                            switch( eValue )
                            {
                            case VALUE_FROM: aProperty = xAnimate->getFrom(); break;
                            case VALUE_TO:   aProperty = xAnimate->getTo(); break;
                            case VALUE_BY:   aProperty = xAnimate->getBy(); break;
                            case VALUE_FIRST:
                            case VALUE_LAST:
                                {
                                    Sequence<Any> aValues( xAnimate->getValues() );
                                    if( aValues.hasElements() )
                                        aProperty =  aValues[ eValue == VALUE_FIRST ? 0 : aValues.getLength() - 1 ];
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::getProperty(), exception cought!" );
    }

    return aProperty;
}

// --------------------------------------------------------------------

bool CustomAnimationEffect::setProperty( sal_Int32 nNodeType, const OUString& rAttributeName, EValue eValue, const Any& rValue )
{
    bool bChanged = false;
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() )
                {
                    Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xAnimate.is() )
                        continue;

                    if( xAnimate->getType() == nNodeType )
                    {
                        if( xAnimate->getAttributeName() == rAttributeName )
                        {
                            switch( eValue )
                            {
                            case VALUE_FROM:
                                if( xAnimate->getFrom() != rValue )
                                {
                                    xAnimate->setFrom( rValue );
                                    bChanged = true;
                                }
                                break;
                            case VALUE_TO:
                                if( xAnimate->getTo() != rValue )
                                {
                                    xAnimate->setTo( rValue );
                                    bChanged = true;
                                }
                                break;
                            case VALUE_BY:
                                if( xAnimate->getTo() != rValue )
                                {
                                    xAnimate->setBy( rValue );
                                    bChanged = true;
                                }
                                break;
                            case VALUE_FIRST:
                            case VALUE_LAST:
                                {
                                    Sequence<Any> aValues( xAnimate->getValues() );
                                    if( !aValues.hasElements() )
                                        aValues.realloc(1);

                                    sal_Int32 nIndex = eValue == VALUE_FIRST ? 0 : aValues.getLength() - 1;

                                    if( aValues[ nIndex ] != rValue )
                                    {
                                        aValues[ nIndex ] = rValue;
                                        xAnimate->setValues( aValues );
                                        bChanged = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setProperty(), exception cought!" );
    }

    return bChanged;
}

// --------------------------------------------------------------------

static bool implIsColorAttribute( const OUString& rAttributeName )
{
    return rAttributeName == "FillColor" || rAttributeName == "LineColor" || rAttributeName == "CharColor";
}

// --------------------------------------------------------------------

Any CustomAnimationEffect::getColor( sal_Int32 nIndex )
{
    Any aColor;
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() && !aColor.hasValue() )
                {
                    Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xAnimate.is() )
                        continue;

                    switch( xAnimate->getType() )
                    {
                    case AnimationNodeType::SET:
                    case AnimationNodeType::ANIMATE:
                        if( !implIsColorAttribute( xAnimate->getAttributeName() ) )
                            break;
                    case AnimationNodeType::ANIMATECOLOR:
                        Sequence<Any> aValues( xAnimate->getValues() );
                        if( aValues.hasElements() )
                        {
                            if( aValues.getLength() > nIndex )
                                aColor = aValues[nIndex];
                        }
                        else if( nIndex == 0 )
                            aColor = xAnimate->getFrom();
                        else
                            aColor = xAnimate->getTo();
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::getColor(), exception cought!" );
    }

    return aColor;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setColor( sal_Int32 nIndex, const Any& rColor )
{
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() )
                {
                    Reference< XAnimate > xAnimate( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xAnimate.is() )
                        continue;

                    switch( xAnimate->getType() )
                    {
                    case AnimationNodeType::SET:
                    case AnimationNodeType::ANIMATE:
                        if( !implIsColorAttribute( xAnimate->getAttributeName() ) )
                            break;
                    case AnimationNodeType::ANIMATECOLOR:
                    {
                        Sequence<Any> aValues( xAnimate->getValues() );
                        if( aValues.hasElements() )
                        {
                            if( aValues.getLength() > nIndex )
                            {
                                aValues[nIndex] = rColor;
                                xAnimate->setValues( aValues );
                            }
                        }
                        else if( (nIndex == 0) && xAnimate->getFrom().hasValue() )
                            xAnimate->setFrom(rColor);
                        else if( (nIndex == 1) && xAnimate->getTo().hasValue() )
                            xAnimate->setTo(rColor);
                    }
                    break;

                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setColor(), exception cought!" );
    }
}

// --------------------------------------------------------------------

Any CustomAnimationEffect::getTransformationProperty( sal_Int32 nTransformType, EValue eValue )
{
    Any aProperty;
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() && !aProperty.hasValue() )
                {
                    Reference< XAnimateTransform > xTransform( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xTransform.is() )
                        continue;

                    if( xTransform->getTransformType() == nTransformType )
                    {
                        switch( eValue )
                        {
                        case VALUE_FROM: aProperty = xTransform->getFrom(); break;
                        case VALUE_TO:   aProperty = xTransform->getTo(); break;
                        case VALUE_BY:   aProperty = xTransform->getBy(); break;
                        case VALUE_FIRST:
                        case VALUE_LAST:
                            {
                                Sequence<Any> aValues( xTransform->getValues() );
                                if( aValues.hasElements() )
                                    aProperty =  aValues[ eValue == VALUE_FIRST ? 0 : aValues.getLength() - 1 ];
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::getTransformationProperty(), exception cought!" );
    }

    return aProperty;
}

// --------------------------------------------------------------------

bool CustomAnimationEffect::setTransformationProperty( sal_Int32 nTransformType, EValue eValue, const Any& rValue )
{
    bool bChanged = false;
    if( mxNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxNode, UNO_QUERY );
        if( xEnumerationAccess.is() )
        {
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
            if( xEnumeration.is() )
            {
                while( xEnumeration->hasMoreElements() )
                {
                    Reference< XAnimateTransform > xTransform( xEnumeration->nextElement(), UNO_QUERY );
                    if( !xTransform.is() )
                        continue;

                    if( xTransform->getTransformType() == nTransformType )
                    {
                        switch( eValue )
                        {
                        case VALUE_FROM:
                            if( xTransform->getFrom() != rValue )
                            {
                                xTransform->setFrom( rValue );
                                bChanged = true;
                            }
                            break;
                        case VALUE_TO:
                            if( xTransform->getTo() != rValue )
                            {
                                xTransform->setTo( rValue );
                                bChanged = true;
                            }
                            break;
                        case VALUE_BY:
                            if( xTransform->getBy() != rValue )
                            {
                                xTransform->setBy( rValue );
                                bChanged = true;
                            }
                            break;
                        case VALUE_FIRST:
                        case VALUE_LAST:
                            {
                                Sequence<Any> aValues( xTransform->getValues() );
                                if( !aValues.hasElements() )
                                    aValues.realloc(1);

                                sal_Int32 nIndex = eValue == VALUE_FIRST ? 0 : aValues.getLength() - 1;
                                if( aValues[nIndex] != rValue )
                                {
                                    aValues[nIndex] = rValue;
                                    xTransform->setValues( aValues );
                                    bChanged = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setTransformationProperty(), exception cought!" );
    }

    return bChanged;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::createAudio( const ::com::sun::star::uno::Any& rSource, double fVolume /* = 1.0 */ )
{
    DBG_ASSERT( !mxAudio.is(), "sd::CustomAnimationEffect::createAudio(), node already has an audio!" );

    if( !mxAudio.is() ) try
    {
        Reference< XMultiServiceFactory > xMsf( ::comphelper::getProcessServiceFactory() );
        Reference< XAudio > xAudio( xMsf->createInstance( "com.sun.star.animations.Audio" ), UNO_QUERY_THROW );
        xAudio->setSource( rSource );
        xAudio->setVolume( fVolume );
        setAudio( xAudio );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::createAudio(), exception cought!" );
    }
}

// --------------------------------------------------------------------

static Reference< XCommand > findCommandNode( const Reference< XAnimationNode >& xRootNode )
{
    Reference< XCommand > xCommand;

    if( xRootNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( !xCommand.is() && xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xNode( xEnumeration->nextElement(), UNO_QUERY );
            if( xNode.is() && (xNode->getType() == AnimationNodeType::COMMAND) )
                xCommand.set( xNode, UNO_QUERY_THROW );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::findCommandNode(), exception caught!" );
    }

    return xCommand;
}

void CustomAnimationEffect::removeAudio()
{
    try
    {
        Reference< XAnimationNode > xChild;

        if( mxAudio.is() )
        {
            xChild.set( mxAudio, UNO_QUERY );
            mxAudio.clear();
        }
        else if( mnCommand == EffectCommands::STOPAUDIO )
        {
            xChild.set( findCommandNode( mxNode ), UNO_QUERY );
            mnCommand = 0;
        }

        if( xChild.is() )
        {
            Reference< XTimeContainer > xContainer( mxNode, UNO_QUERY );
            if( xContainer.is() )
                xContainer->removeChild( xChild );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::removeAudio(), exception caught!" );
    }

}

// --------------------------------------------------------------------

void CustomAnimationEffect::setAudio( const Reference< ::com::sun::star::animations::XAudio >& xAudio )
{
    if( mxAudio != xAudio ) try
    {
        removeAudio();
        mxAudio = xAudio;
        Reference< XTimeContainer > xContainer( mxNode, UNO_QUERY );
        Reference< XAnimationNode > xChild( mxAudio, UNO_QUERY );
        if( xContainer.is() && xChild.is() )
            xContainer->appendChild( xChild );
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setAudio(), exception caught!" );
    }
}

// --------------------------------------------------------------------

void CustomAnimationEffect::setStopAudio()
{
    if( mnCommand != EffectCommands::STOPAUDIO ) try
    {
        if( mxAudio.is() )
            removeAudio();

        Reference< XMultiServiceFactory > xMsf( ::comphelper::getProcessServiceFactory() );
        Reference< XCommand > xCommand( xMsf->createInstance( "com.sun.star.animations.Command" ), UNO_QUERY_THROW );

        xCommand->setCommand( EffectCommands::STOPAUDIO );

        Reference< XTimeContainer > xContainer( mxNode, UNO_QUERY_THROW );
        Reference< XAnimationNode > xChild( xCommand, UNO_QUERY_THROW );
        xContainer->appendChild( xChild );

        mnCommand = EffectCommands::STOPAUDIO;
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::CustomAnimationEffect::setStopAudio(), exception caught!" );
    }
}

// --------------------------------------------------------------------

bool CustomAnimationEffect::getStopAudio() const
{
    return mnCommand == EffectCommands::STOPAUDIO;
}

// --------------------------------------------------------------------

SdrPathObj* CustomAnimationEffect::createSdrPathObjFromPath()
{
    SdrPathObj * pPathObj = new SdrPathObj( OBJ_PATHLINE );
    updateSdrPathObjFromPath( *pPathObj );
    return pPathObj;
}

// --------------------------------------------------------------------

void CustomAnimationEffect::updateSdrPathObjFromPath( SdrPathObj& rPathObj )
{
    ::basegfx::B2DPolyPolygon xPolyPoly;
    if( ::basegfx::tools::importFromSvgD( xPolyPoly, getPath() ) )
    {
        SdrObject* pObj = GetSdrObjectFromXShape( getTargetShape() );
        if( pObj )
        {
            SdrPage* pPage = pObj->GetPage();
            if( pPage )
            {
                const Size aPageSize( pPage->GetSize() );
                xPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix((double)aPageSize.Width(), (double)aPageSize.Height()));
            }

            const Rectangle aBoundRect( pObj->GetCurrentBoundRect() );
            const Point aCenter( aBoundRect.Center() );
            xPolyPoly.transform(basegfx::tools::createTranslateB2DHomMatrix(aCenter.X(), aCenter.Y()));
        }
    }

    rPathObj.SetPathPoly( xPolyPoly );
}

// --------------------------------------------------------------------

void CustomAnimationEffect::updatePathFromSdrPathObj( const SdrPathObj& rPathObj )
{
    ::basegfx::B2DPolyPolygon xPolyPoly( rPathObj.GetPathPoly() );

    SdrObject* pObj = GetSdrObjectFromXShape( getTargetShape() );
    if( pObj )
    {
        Rectangle aBoundRect(0,0,0,0);

        const drawinglayer::primitive2d::Primitive2DSequence xPrimitives(pObj->GetViewContact().getViewIndependentPrimitive2DSequence());
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            const basegfx::B2DRange aRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xPrimitives, aViewInformation2D));

        if(!aRange.isEmpty())
        {
            aBoundRect = Rectangle(
                    (sal_Int32)floor(aRange.getMinX()), (sal_Int32)floor(aRange.getMinY()),
                    (sal_Int32)ceil(aRange.getMaxX()), (sal_Int32)ceil(aRange.getMaxY()));
        }

        const Point aCenter( aBoundRect.Center() );

        xPolyPoly.transform(basegfx::tools::createTranslateB2DHomMatrix(-aCenter.X(), -aCenter.Y()));

        SdrPage* pPage = pObj->GetPage();
        if( pPage )
        {
            const Size aPageSize( pPage->GetSize() );
            xPolyPoly.transform(basegfx::tools::createScaleB2DHomMatrix(
                1.0 / (double)aPageSize.Width(), 1.0 / (double)aPageSize.Height()));
        }
    }

    setPath( ::basegfx::tools::exportToSvgD( xPolyPoly ) );
}

// ====================================================================

EffectSequenceHelper::EffectSequenceHelper()
: mnSequenceType( EffectNodeType::DEFAULT )
{
}

// --------------------------------------------------------------------

EffectSequenceHelper::EffectSequenceHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XTimeContainer >& xSequenceRoot )
: mxSequenceRoot( xSequenceRoot ), mnSequenceType( EffectNodeType::DEFAULT )
{
    Reference< XAnimationNode > xNode( mxSequenceRoot, UNO_QUERY_THROW );
    create( xNode );
}

// --------------------------------------------------------------------

EffectSequenceHelper::~EffectSequenceHelper()
{
    reset();
}

// --------------------------------------------------------------------

void EffectSequenceHelper::reset()
{
    EffectSequence::iterator aIter( maEffects.begin() );
    EffectSequence::iterator aEnd( maEffects.end() );
    if( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect = (*aIter++);
        pEffect->setEffectSequence(0);
    }
    maEffects.clear();
}

Reference< XAnimationNode > EffectSequenceHelper::getRootNode()
{
    Reference< XAnimationNode > xRoot( mxSequenceRoot, UNO_QUERY );
    return xRoot;
}

// --------------------------------------------------------------------

void EffectSequenceHelper::append( const CustomAnimationEffectPtr& pEffect )
{
    pEffect->setEffectSequence( this );
    maEffects.push_back(pEffect);
    rebuild();
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr EffectSequenceHelper::append( const CustomAnimationPresetPtr& pPreset, const Any& rTarget, double fDuration /* = -1.0 */ )
{
    CustomAnimationEffectPtr pEffect;

    if( pPreset.get() )
    {
        OUString strEmpty;
        Reference< XAnimationNode > xNode( pPreset->create( strEmpty ) );
        if( xNode.is() )
        {
            // first, filter all only ui relevant user data
            std::vector< NamedValue > aNewUserData;
            Sequence< NamedValue > aUserData( xNode->getUserData() );
            sal_Int32 nLength = aUserData.getLength();
            const NamedValue* p = aUserData.getConstArray();
            bool bFilter = false;

            while( nLength-- )
            {
                if( p->Name != "text-only" && p->Name != "preset-property" )
                {
                    aNewUserData.push_back( *p );
                    bFilter = true;
                }
                p++;
            }

            if( bFilter )
            {
                aUserData = ::comphelper::containerToSequence< NamedValue, std::vector< NamedValue > >( aNewUserData );
                xNode->setUserData( aUserData );
            }

            // check target, maybe we need to force it to text
            Any aTarget( rTarget );
            sal_Int16 nSubItem = ShapeAnimationSubType::AS_WHOLE;

            if( aTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
            {
                nSubItem = ShapeAnimationSubType::ONLY_TEXT;
            }
            else if( pPreset->isTextOnly() )
            {
                Reference< XShape > xShape;
                aTarget >>= xShape;
                if( xShape.is() )
                {
                    // thats bad, we target a shape here but the effect is only for text
                    // so change subitem
                    nSubItem = ShapeAnimationSubType::ONLY_TEXT;
                }
            }

            // now create effect from preset
            pEffect.reset( new CustomAnimationEffect( xNode ) );
            pEffect->setEffectSequence( this );
            pEffect->setTarget( aTarget );
            pEffect->setTargetSubItem( nSubItem );
            if( fDuration != -1.0 )
                pEffect->setDuration( fDuration );

            maEffects.push_back(pEffect);

            rebuild();
        }
    }

    DBG_ASSERT( pEffect.get(), "sd::EffectSequenceHelper::append(), failed!" );
    return pEffect;
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr EffectSequenceHelper::append( const SdrPathObj& rPathObj, const Any& rTarget, double fDuration /* = -1.0 */ )
{
    CustomAnimationEffectPtr pEffect;

    if( fDuration <= 0.0 )
        fDuration = 2.0;

    try
    {
        Reference< XTimeContainer > xEffectContainer( createParallelTimeContainer() );
        const OUString aServiceName( "com.sun.star.animations.AnimateMotion" );
        Reference< XAnimationNode > xAnimateMotion( ::comphelper::getProcessServiceFactory()->createInstance(aServiceName), UNO_QUERY_THROW );

        xAnimateMotion->setDuration( Any( fDuration ) );
        xAnimateMotion->setFill( AnimationFill::HOLD );
        xEffectContainer->appendChild( xAnimateMotion );

        sal_Int16 nSubItem = ShapeAnimationSubType::AS_WHOLE;

        if( rTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
            nSubItem = ShapeAnimationSubType::ONLY_TEXT;

        Reference< XAnimationNode > xEffectNode( xEffectContainer, UNO_QUERY_THROW );
        pEffect.reset( new CustomAnimationEffect( xEffectNode ) );
        pEffect->setEffectSequence( this );
        pEffect->setTarget( rTarget );
        pEffect->setTargetSubItem( nSubItem );
        pEffect->setNodeType( ::com::sun::star::presentation::EffectNodeType::ON_CLICK );
        pEffect->setPresetClass( ::com::sun::star::presentation::EffectPresetClass::MOTIONPATH );
        pEffect->setAcceleration( 0.5 );
        pEffect->setDecelerate( 0.5 );
        pEffect->setFill( AnimationFill::HOLD );
        pEffect->setBegin( 0.0 );
        pEffect->updatePathFromSdrPathObj( rPathObj );
        if( fDuration != -1.0 )
            pEffect->setDuration( fDuration );

        maEffects.push_back(pEffect);

        rebuild();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::append(), exception cought!" );
    }

    return pEffect;
}

// --------------------------------------------------------------------

void EffectSequenceHelper::replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pPreset, const OUString& rPresetSubType, double fDuration /* = -1.0 */ )
{
    if( pEffect.get() && pPreset.get() ) try
    {
        Reference< XAnimationNode > xNewNode( pPreset->create( rPresetSubType ) );
        if( xNewNode.is() )
        {
            pEffect->replaceNode( xNewNode );
            if( fDuration != -1.0 )
                pEffect->setDuration( fDuration );
        }

        rebuild();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::replace(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::replace( const CustomAnimationEffectPtr& pEffect, const CustomAnimationPresetPtr& pPreset, double fDuration /* = -1.0 */ )
{
    OUString strEmpty;
    replace( pEffect, pPreset, strEmpty, fDuration );
}

// --------------------------------------------------------------------

void EffectSequenceHelper::remove( const CustomAnimationEffectPtr& pEffect )
{
    if( pEffect.get() )
    {
        pEffect->setEffectSequence( 0 );
        maEffects.remove( pEffect );
    }

    rebuild();
}

// --------------------------------------------------------------------

void EffectSequenceHelper::rebuild()
{
    implRebuild();
}

// --------------------------------------------------------------------

void EffectSequenceHelper::implRebuild()
{
    try
    {
        // first we delete all time containers on the first two levels
        Reference< XEnumerationAccess > xEnumerationAccess( mxSequenceRoot, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            Reference< XTimeContainer > xChildContainer( xChildNode, UNO_QUERY_THROW );

            Reference< XEnumerationAccess > xChildEnumerationAccess( xChildNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xChildEnumeration( xChildEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
            while( xChildEnumeration->hasMoreElements() )
            {
                Reference< XAnimationNode > xNode( xChildEnumeration->nextElement(), UNO_QUERY_THROW );
                xChildContainer->removeChild( xNode );
            }

            mxSequenceRoot->removeChild( xChildNode );
        }

        // second, rebuild main sequence
        EffectSequence::iterator aIter( maEffects.begin() );
        EffectSequence::iterator aEnd( maEffects.end() );
        if( aIter != aEnd )
        {
            AfterEffectNodeList aAfterEffects;

            CustomAnimationEffectPtr pEffect = (*aIter++);

            bool bFirst = true;
            do
            {
                // create a par container for the next click node and all following with and after effects
                Reference< XTimeContainer > xOnClickContainer( createParallelTimeContainer() );

                Event aEvent;
                if( mxEventSource.is() )
                {
                    aEvent.Source <<= mxEventSource;
                    aEvent.Trigger = EventTrigger::ON_CLICK;
                }
                else
                {
                    aEvent.Trigger = EventTrigger::ON_NEXT;
                }
                aEvent.Repeat = 0;

                Any aBegin( makeAny( aEvent ) );
                if( bFirst )
                {
                    // if the first node is not a click action, this click container
                    // must not have INDEFINITE begin but start at 0s
                    bFirst = false;
                    if( pEffect->getNodeType() != EffectNodeType::ON_CLICK )
                        aBegin <<= (double)0.0;
                }

                xOnClickContainer->setBegin( aBegin );

                Reference< XAnimationNode > xOnClickContainerNode( xOnClickContainer, UNO_QUERY_THROW );
                mxSequenceRoot->appendChild( xOnClickContainerNode );

                double fBegin = 0.0;

                do
                {
                    // create a par container for the current click or after effect node and all following with effects
                    Reference< XTimeContainer > xWithContainer( createParallelTimeContainer() );
                    Reference< XAnimationNode > xWithContainerNode( xWithContainer, UNO_QUERY_THROW );
                    xWithContainer->setBegin( makeAny( fBegin ) );
                    xOnClickContainer->appendChild( xWithContainerNode );

                    double fDuration = 0.0;
                    do
                    {
                        Reference< XAnimationNode > xEffectNode( pEffect->getNode() );
                        xWithContainer->appendChild( xEffectNode );

                        if( pEffect->hasAfterEffect() )
                        {
                            Reference< XAnimationNode > xAfterEffect( pEffect->createAfterEffectNode() );
                            AfterEffectNode a( xAfterEffect, xEffectNode, pEffect->IsAfterEffectOnNext() );
                            aAfterEffects.push_back( a );
                        }

                        double fTemp = pEffect->getBegin() + pEffect->getAbsoluteDuration();
                        if( fTemp > fDuration )
                            fDuration = fTemp;

                        if( aIter != aEnd )
                            pEffect = (*aIter++);
                        else
                            pEffect.reset();
                    }
                    while( pEffect.get() && (pEffect->getNodeType() == EffectNodeType::WITH_PREVIOUS) );

                    fBegin += fDuration;
                }
                while( pEffect.get() && (pEffect->getNodeType() != EffectNodeType::ON_CLICK) );
            }
            while( pEffect.get() );

            // process after effect nodes
            std::for_each( aAfterEffects.begin(), aAfterEffects.end(), stl_process_after_effect_node_func );

            updateTextGroups();

            // reset duration, might have been altered (see below)
            mxSequenceRoot->setDuration( Any() );
        }
        else
        {
            // empty sequence, set duration to 0.0 explicitly
            // (otherwise, this sequence will never end)
            mxSequenceRoot->setDuration( makeAny((double)0.0) );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::rebuild(), exception cought!" );
    }
}

// --------------------------------------------------------------------

Reference< XTimeContainer > EffectSequenceHelper::createParallelTimeContainer() const
{
    const OUString aServiceName( "com.sun.star.animations.ParallelTimeContainer" );
    return Reference< XTimeContainer >( ::comphelper::getProcessServiceFactory()->createInstance(aServiceName), UNO_QUERY );
}

// --------------------------------------------------------------------

stl_CustomAnimationEffect_search_node_predict::stl_CustomAnimationEffect_search_node_predict( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xSearchNode )
: mxSearchNode( xSearchNode )
{
}

// --------------------------------------------------------------------

bool stl_CustomAnimationEffect_search_node_predict::operator()( CustomAnimationEffectPtr pEffect ) const
{
    return pEffect->getNode() == mxSearchNode;
}

// --------------------------------------------------------------------

static bool implFindNextContainer( Reference< XTimeContainer >& xParent, Reference< XTimeContainer >& xCurrent, Reference< XTimeContainer >& xNext )
 throw(Exception)
{
    Reference< XEnumerationAccess > xEnumerationAccess( xParent, UNO_QUERY_THROW );
    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration() );
    if( xEnumeration.is() )
    {
        Reference< XInterface > x;
        while( xEnumeration->hasMoreElements() && !xNext.is() )
        {
            if( (xEnumeration->nextElement() >>= x) && (x == xCurrent) )
            {
                if( xEnumeration->hasMoreElements() )
                    xEnumeration->nextElement() >>= xNext;
            }
        }
    }
    return xNext.is();
}

// --------------------------------------------------------------------

void stl_process_after_effect_node_func(AfterEffectNode& rNode)
{
    try
    {
        if( rNode.mxNode.is() && rNode.mxMaster.is() )
        {
            // set master node
            Reference< XAnimationNode > xMasterNode( rNode.mxMaster, UNO_QUERY_THROW );
            Sequence< NamedValue > aUserData( rNode.mxNode->getUserData() );
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "master-element";
            aUserData[nSize].Value <<= xMasterNode;
            rNode.mxNode->setUserData( aUserData );

            // insert after effect node into timeline
            Reference< XTimeContainer > xContainer( rNode.mxMaster->getParent(), UNO_QUERY_THROW );

            if( !rNode.mbOnNextEffect ) // sameClick
            {
                // insert the aftereffect after its effect is animated
                xContainer->insertAfter( rNode.mxNode, rNode.mxMaster );
            }
            else // nextClick
            {
                Reference< XMultiServiceFactory > xMsf( ::comphelper::getProcessServiceFactory() );
                // insert the aftereffect in the next group

                Reference< XTimeContainer > xClickContainer( xContainer->getParent(), UNO_QUERY_THROW );
                Reference< XTimeContainer > xSequenceContainer( xClickContainer->getParent(), UNO_QUERY_THROW );

                Reference< XTimeContainer > xNextContainer;

                // first try if we have an after effect container
                if( !implFindNextContainer( xClickContainer, xContainer, xNextContainer ) )
                {
                    Reference< XTimeContainer > xNextClickContainer;
                    // if not, try to find the next click effect container
                    if( implFindNextContainer( xSequenceContainer, xClickContainer, xNextClickContainer ) )
                    {
                        Reference< XEnumerationAccess > xEnumerationAccess( xNextClickContainer, UNO_QUERY_THROW );
                        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                        if( xEnumeration->hasMoreElements() )
                        {
                            // the next container is the first child container
                            xEnumeration->nextElement() >>= xNextContainer;
                        }
                        else
                        {
                            // this does not yet have a child container, create one
                            const OUString aServiceName( "com.sun.star.animations.ParallelTimeContainer" );
                            xNextContainer = Reference< XTimeContainer >::query( xMsf->createInstance(aServiceName) );

                            if( xNextContainer.is() )
                            {
                                Reference< XAnimationNode > xNode( xNextContainer, UNO_QUERY_THROW );
                                xNode->setBegin( makeAny( (double)0.0 ) );
                                xNextClickContainer->appendChild( xNode );
                            }
                        }
                        DBG_ASSERT( xNextContainer.is(), "ppt::stl_process_after_effect_node_func::operator(), could not find/create container!" );
                    }
                }

                // if we don't have a next container, we add one to the sequence container
                if( !xNextContainer.is() )
                {
                    const OUString aServiceName( "com.sun.star.animations.ParallelTimeContainer" );
                    Reference< XTimeContainer > xNewClickContainer( xMsf->createInstance(aServiceName), UNO_QUERY_THROW );

                    Reference< XAnimationNode > xNewClickNode( xNewClickContainer, UNO_QUERY_THROW );

                    Event aEvent;
                    aEvent.Trigger = EventTrigger::ON_NEXT;
                    aEvent.Repeat = 0;
                    xNewClickNode->setBegin( makeAny( aEvent ) );

                    Reference< XAnimationNode > xRefNode( xClickContainer, UNO_QUERY_THROW );
                    xSequenceContainer->insertAfter( xNewClickNode, xRefNode );

                    xNextContainer = Reference< XTimeContainer >::query( xMsf->createInstance(aServiceName) );

                    DBG_ASSERT( xNextContainer.is(), "ppt::stl_process_after_effect_node_func::operator(), could not create container!" );
                    if( xNextContainer.is() )
                    {
                        Reference< XAnimationNode > xNode( xNextContainer, UNO_QUERY_THROW );
                        xNode->setBegin( makeAny( (double)0.0 ) );
                        xNewClickContainer->appendChild( xNode );
                    }
                }

                if( xNextContainer.is() )
                {
                    // find begin time of first element
                    Reference< XEnumerationAccess > xEnumerationAccess( xNextContainer, UNO_QUERY_THROW );
                    Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                    if( xEnumeration->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xChild;
                        // the next container is the first child container
                        xEnumeration->nextElement() >>= xChild;
                        if( xChild.is() )
                        {
                            Any aBegin( xChild->getBegin() );
                            double fBegin = 0.0;
                            if( (aBegin >>= fBegin) && (fBegin >= 0.0))
                                rNode.mxNode->setBegin( aBegin );
                        }
                    }

                    xNextContainer->appendChild( rNode.mxNode );
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "ppt::stl_process_after_effect_node_func::operator(), exception cought!" );
    }
}

// --------------------------------------------------------------------

EffectSequence::iterator EffectSequenceHelper::find( const CustomAnimationEffectPtr& pEffect )
{
    return std::find( maEffects.begin(), maEffects.end(), pEffect );
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr EffectSequenceHelper::findEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) const
{
    CustomAnimationEffectPtr pEffect;

    EffectSequence::const_iterator aIter( maEffects.begin() );
    for( ; aIter != maEffects.end(); ++aIter )
    {
        if( (*aIter)->getNode() == xNode )
        {
            pEffect = (*aIter);
            break;
        }
    }

    return pEffect;
}

// --------------------------------------------------------------------

sal_Int32 EffectSequenceHelper::getOffsetFromEffect( const CustomAnimationEffectPtr& xEffect ) const
{
    sal_Int32 nOffset = 0;

    EffectSequence::const_iterator aIter( maEffects.begin() );
    for( ; aIter != maEffects.end(); ++aIter, nOffset++ )
    {
        if( (*aIter) == xEffect )
            return nOffset;
    }

    return -1;
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr EffectSequenceHelper::getEffectFromOffset( sal_Int32 nOffset ) const
{
    EffectSequence::const_iterator aIter( maEffects.begin() );
    while( nOffset-- && aIter != maEffects.end() )
        ++aIter;

    CustomAnimationEffectPtr pEffect;
    if( aIter != maEffects.end() )
        pEffect = (*aIter);

    return pEffect;
}

// --------------------------------------------------------------------

bool EffectSequenceHelper::disposeShape( const Reference< XShape >& xShape )
{
    bool bChanges = false;

    EffectSequence::iterator aIter( maEffects.begin() );
    while( aIter != maEffects.end() )
    {
        if( (*aIter)->getTargetShape() == xShape )
        {
            (*aIter)->setEffectSequence( 0 );
            bChanges = true;
            aIter = maEffects.erase( aIter );
        }
        else
        {
            ++aIter;
        }
    }

    return bChanges;
}

// --------------------------------------------------------------------

bool EffectSequenceHelper::hasEffect( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape )
{
    EffectSequence::iterator aIter( maEffects.begin() );
    while( aIter != maEffects.end() )
    {
        if( (*aIter)->getTargetShape() == xShape )
            return true;
        ++aIter;
    }

    return false;
}

// --------------------------------------------------------------------

void EffectSequenceHelper::insertTextRange( const com::sun::star::uno::Any& aTarget )
{
    bool bChanges = false;

    ParagraphTarget aParaTarget;
    if( !(aTarget >>= aParaTarget ) )
        return;

    EffectSequence::iterator aIter( maEffects.begin() );
    while( aIter != maEffects.end() )
    {
        if( (*aIter)->getTargetShape() == aParaTarget.Shape )
            bChanges |= (*aIter)->checkForText();
        ++aIter;
    }

    if( bChanges )
        rebuild();
}

// --------------------------------------------------------------------

void EffectSequenceHelper::disposeTextRange( const com::sun::star::uno::Any& aTarget )
{
    ParagraphTarget aParaTarget;
    if( !(aTarget >>= aParaTarget ) )
        return;

    bool bChanges = false;
    bool bErased = false;

    EffectSequence::iterator aIter( maEffects.begin() );
    while( aIter != maEffects.end() )
    {
        Any aIterTarget( (*aIter)->getTarget() );
        if( aIterTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
        {
            ParagraphTarget aIterParaTarget;
            if( (aIterTarget >>= aIterParaTarget) && (aIterParaTarget.Shape == aParaTarget.Shape) )
            {
                if( aIterParaTarget.Paragraph == aParaTarget.Paragraph )
                {
                    // delete this effect if it targets the disposed paragraph directly
                    (*aIter)->setEffectSequence( 0 );
                    aIter = maEffects.erase( aIter );
                    bChanges = true;
                    bErased = true;
                }
                else
                {
                    if( aIterParaTarget.Paragraph > aParaTarget.Paragraph )
                    {
                        // shift all paragraphs after disposed paragraph
                        aIterParaTarget.Paragraph--;
                        (*aIter)->setTarget( makeAny( aIterParaTarget ) );
                    }
                }
            }
        }
        else if( (*aIter)->getTargetShape() == aParaTarget.Shape )
        {
            bChanges |= (*aIter)->checkForText();
        }

        if( bErased )
            bErased = false;
        else
            ++aIter;
    }

    if( bChanges )
        rebuild();
}

// --------------------------------------------------------------------

CustomAnimationTextGroup::CustomAnimationTextGroup( const Reference< XShape >& rTarget, sal_Int32 nGroupId )
:   maTarget( rTarget ),
    mnGroupId( nGroupId )
{
    reset();
}

// --------------------------------------------------------------------

void CustomAnimationTextGroup::reset()
{
    mnTextGrouping = -1;
    mbAnimateForm = false;
    mbTextReverse = false;
    mfGroupingAuto = -1.0;
    mnLastPara = -1; // used to check for TextReverse

    for (int i = 0; i < PARA_LEVELS; ++i)
    {
        mnDepthFlags[i] = 0;
    }

    maEffects.clear();
}

// --------------------------------------------------------------------

void CustomAnimationTextGroup::addEffect( CustomAnimationEffectPtr& pEffect )
{
    maEffects.push_back( pEffect );

    Any aTarget( pEffect->getTarget() );
    if( aTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
    {
        // now look at the paragraph
        ParagraphTarget aParaTarget;
        aTarget >>= aParaTarget;

        if( mnLastPara != -1 )
            mbTextReverse = mnLastPara > aParaTarget.Paragraph;

        mnLastPara = aParaTarget.Paragraph;

        const sal_Int32 nParaDepth = pEffect->getParaDepth();

        // only look at the first PARA_LEVELS levels
        if( nParaDepth < PARA_LEVELS )
        {
            // our first paragraph with this level?
            if( mnDepthFlags[nParaDepth] == 0 )
            {
                // so set it to the first found
                mnDepthFlags[nParaDepth] = (sal_Int8)pEffect->getNodeType();
            }
            else if( mnDepthFlags[nParaDepth] != pEffect->getNodeType() )
            {
                mnDepthFlags[nParaDepth] = -1;
            }

            if( pEffect->getNodeType() == EffectNodeType::AFTER_PREVIOUS )
                mfGroupingAuto = pEffect->getBegin();

            mnTextGrouping = PARA_LEVELS;
            while( (mnTextGrouping > 0)
                   && (mnDepthFlags[mnTextGrouping - 1] <= 0) )
                --mnTextGrouping;
        }
    }
    else
    {
        // if we have an effect with the shape as a target, we animate the background
        mbAnimateForm = pEffect->getTargetSubItem() != ShapeAnimationSubType::ONLY_TEXT;
    }
}

// --------------------------------------------------------------------

class TextGroupMapImpl : public std::map< sal_Int32, CustomAnimationTextGroup* >
{
public:
    CustomAnimationTextGroup* findGroup( sal_Int32 nGroupId );
};

// --------------------------------------------------------------------

CustomAnimationTextGroupPtr EffectSequenceHelper::findGroup( sal_Int32 nGroupId )
{
    CustomAnimationTextGroupPtr aPtr;

    CustomAnimationTextGroupMap::iterator aIter( maGroupMap.find( nGroupId ) );
    if( aIter != maGroupMap.end() )
        aPtr = (*aIter).second;

    return aPtr;
}

// --------------------------------------------------------------------

void EffectSequenceHelper::updateTextGroups()
{
    maGroupMap.clear();

    // first create all the groups
    EffectSequence::iterator aIter( maEffects.begin() );
    const EffectSequence::iterator aEnd( maEffects.end() );
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect( (*aIter++) );

        const sal_Int32 nGroupId = pEffect->getGroupId();

        if( nGroupId == -1 )
            continue; // trivial case, no group

        CustomAnimationTextGroupPtr pGroup = findGroup( nGroupId );
        if( !pGroup.get() )
        {
            pGroup.reset( new CustomAnimationTextGroup( pEffect->getTargetShape(), nGroupId ) );
            maGroupMap[nGroupId] = pGroup;
        }

        pGroup->addEffect( pEffect );
    }
}

// --------------------------------------------------------------------

CustomAnimationTextGroupPtr EffectSequenceHelper::createTextGroup( CustomAnimationEffectPtr pEffect, sal_Int32 nTextGrouping, double fTextGroupingAuto, sal_Bool bAnimateForm, sal_Bool bTextReverse )
{
    // first finde a free group-id
    sal_Int32 nGroupId = 0;

    CustomAnimationTextGroupMap::iterator aIter( maGroupMap.begin() );
    const CustomAnimationTextGroupMap::iterator aEnd( maGroupMap.end() );
    while( aIter != aEnd )
    {
        if( (*aIter).first == nGroupId )
        {
            nGroupId++;
            aIter = maGroupMap.begin();
        }
        else
        {
            ++aIter;
        }
    }

    Reference< XShape > xTarget( pEffect->getTargetShape() );

    CustomAnimationTextGroupPtr pTextGroup( new CustomAnimationTextGroup( xTarget, nGroupId ) );
    maGroupMap[nGroupId] = pTextGroup;

    bool bUsed = false;

    // do we need to target the shape?
    if( (nTextGrouping == 0) || bAnimateForm )
    {
        sal_Int16 nSubItem;
        if( nTextGrouping == 0)
            nSubItem = bAnimateForm ? ShapeAnimationSubType::AS_WHOLE : ShapeAnimationSubType::ONLY_TEXT;
        else
            nSubItem = ShapeAnimationSubType::ONLY_BACKGROUND;

        pEffect->setTarget( makeAny( xTarget ) );
        pEffect->setTargetSubItem( nSubItem );
        pEffect->setEffectSequence( this );
        pEffect->setGroupId( nGroupId );

        pTextGroup->addEffect( pEffect );
        bUsed = true;
    }

    pTextGroup->mnTextGrouping = nTextGrouping;
    pTextGroup->mfGroupingAuto = fTextGroupingAuto;
    pTextGroup->mbTextReverse = bTextReverse;

    // now add an effect for each paragraph
    createTextGroupParagraphEffects( pTextGroup, pEffect, bUsed );

    notify_listeners();

    return pTextGroup;
}

// --------------------------------------------------------------------

void EffectSequenceHelper::createTextGroupParagraphEffects( CustomAnimationTextGroupPtr pTextGroup, CustomAnimationEffectPtr pEffect, bool bUsed )
{
    Reference< XShape > xTarget( pTextGroup->maTarget );

    sal_Int32 nTextGrouping = pTextGroup->mnTextGrouping;
    double fTextGroupingAuto = pTextGroup->mfGroupingAuto;
    sal_Bool bTextReverse = pTextGroup->mbTextReverse;

    // now add an effect for each paragraph
    if( nTextGrouping >= 0 ) try
    {
        EffectSequence::iterator aInsertIter( find( pEffect ) );

        const OUString strNumberingLevel( "NumberingLevel" );
        Reference< XEnumerationAccess > xText( xTarget, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xText->createEnumeration(), UNO_QUERY_THROW );

        std::list< sal_Int16 > aParaList;
        sal_Int16 nPara;

        // fill the list with all valid paragraphs
        for( nPara = 0; xEnumeration->hasMoreElements(); nPara++ )
        {
            Reference< XTextRange > xRange( xEnumeration->nextElement(), UNO_QUERY );
            if( xRange.is() && !xRange->getString().isEmpty() )
            {
                if( bTextReverse ) // sort them
                    aParaList.push_front( nPara );
                else
                    aParaList.push_back( nPara );
            }
        }

        ParagraphTarget aTarget;
        aTarget.Shape = xTarget;

        std::list< sal_Int16 >::iterator aIter( aParaList.begin() );
        std::list< sal_Int16 >::iterator aEnd( aParaList.end() );
        while( aIter != aEnd )
        {
            aTarget.Paragraph = (*aIter++);

            CustomAnimationEffectPtr pNewEffect;
            if( bUsed )
            {
                // clone a new effect from first effect
                pNewEffect = pEffect->clone();
                ++aInsertIter;
                aInsertIter = maEffects.insert( aInsertIter, pNewEffect );
            }
            else
            {
                // reuse first effect if its not yet used
                pNewEffect = pEffect;
                bUsed = true;
                aInsertIter = find( pNewEffect );
            }

            // set target and group-id
            pNewEffect->setTarget( makeAny( aTarget ) );
            pNewEffect->setTargetSubItem( ShapeAnimationSubType::ONLY_TEXT );
            pNewEffect->setGroupId( pTextGroup->mnGroupId );
            pNewEffect->setEffectSequence( this );

            // set correct node type
            if( pNewEffect->getParaDepth() < nTextGrouping )
            {
                if( fTextGroupingAuto == -1.0 )
                {
                    pNewEffect->setNodeType( EffectNodeType::ON_CLICK );
                    pNewEffect->setBegin( 0.0 );
                }
                else
                {
                    pNewEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );
                    pNewEffect->setBegin( fTextGroupingAuto );
                }
            }
            else
            {
                pNewEffect->setNodeType( EffectNodeType::WITH_PREVIOUS );
                pNewEffect->setBegin( 0.0 );
            }

            pTextGroup->addEffect( pNewEffect );
        }
        notify_listeners();
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::EffectSequenceHelper::createTextGroup(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::setTextGrouping( CustomAnimationTextGroupPtr pTextGroup, sal_Int32 nTextGrouping )
{
    if( pTextGroup->mnTextGrouping == nTextGrouping )
    {
        // first case, trivial case, do nothing
    }
    else if( (pTextGroup->mnTextGrouping == -1) && (nTextGrouping >= 0) )
    {
        // second case, we need to add new effects for each paragraph

        CustomAnimationEffectPtr pEffect( pTextGroup->maEffects.front() );

        pTextGroup->mnTextGrouping = nTextGrouping;
        createTextGroupParagraphEffects( pTextGroup, pEffect, true );
        notify_listeners();
    }
    else if( (pTextGroup->mnTextGrouping >= 0) && (nTextGrouping == -1 ) )
    {
        // third case, we need to remove effects for each paragraph

        EffectSequence aEffects( pTextGroup->maEffects );
        pTextGroup->reset();

        EffectSequence::iterator aIter( aEffects.begin() );
        const EffectSequence::iterator aEnd( aEffects.end() );
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect( (*aIter++) );

            if( pEffect->getTarget().getValueType() == ::getCppuType((const ParagraphTarget*)0) )
                remove( pEffect );
            else
                pTextGroup->addEffect( pEffect );
        }
        notify_listeners();
    }
    else
    {
        // fourth case, we need to change the node types for the text nodes
        double fTextGroupingAuto = pTextGroup->mfGroupingAuto;

        EffectSequence aEffects( pTextGroup->maEffects );
        pTextGroup->reset();

        EffectSequence::iterator aIter( aEffects.begin() );
        const EffectSequence::iterator aEnd( aEffects.end() );
        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect( (*aIter++) );

            if( pEffect->getTarget().getValueType() == ::getCppuType((const ParagraphTarget*)0) )
            {
                // set correct node type
                if( pEffect->getParaDepth() < nTextGrouping )
                {
                    if( fTextGroupingAuto == -1.0 )
                    {
                        pEffect->setNodeType( EffectNodeType::ON_CLICK );
                        pEffect->setBegin( 0.0 );
                    }
                    else
                    {
                        pEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );
                        pEffect->setBegin( fTextGroupingAuto );
                    }
                }
                else
                {
                    pEffect->setNodeType( EffectNodeType::WITH_PREVIOUS );
                    pEffect->setBegin( 0.0 );
                }
            }

            pTextGroup->addEffect( pEffect );

        }
        notify_listeners();
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::setAnimateForm( CustomAnimationTextGroupPtr pTextGroup, sal_Bool bAnimateForm )
{
    if( pTextGroup->mbAnimateForm == bAnimateForm )
    {
        // trivial case, do nothing
    }
    else
    {
        EffectSequence aEffects( pTextGroup->maEffects );
        pTextGroup->reset();

        SAL_WARN_IF(aEffects.empty(), "sd", "EffectSequenceHelper::setAnimateForm effects empty" );

        if (aEffects.empty())
            return;

        EffectSequence::iterator aIter( aEffects.begin() );
        const EffectSequence::iterator aEnd( aEffects.end() );

        // first insert if we have to
        if( bAnimateForm )
        {
            EffectSequence::iterator aInsertIter( find( (*aIter) ) );

            CustomAnimationEffectPtr pEffect;
            if( (aEffects.size() == 1) && ((*aIter)->getTarget().getValueType() != ::getCppuType((const ParagraphTarget*)0) ) )
            {
                // special case, only one effect and that targets whole text,
                // convert this to target whole shape
                pEffect = (*aIter++);
                pEffect->setTargetSubItem( ShapeAnimationSubType::AS_WHOLE );
            }
            else
            {
                pEffect = (*aIter)->clone();
                pEffect->setTarget( makeAny( (*aIter)->getTargetShape() ) );
                pEffect->setTargetSubItem( ShapeAnimationSubType::ONLY_BACKGROUND );
                maEffects.insert( aInsertIter, pEffect );
            }

            pTextGroup->addEffect( pEffect );
        }

        if( !bAnimateForm && (aEffects.size() == 1) )
        {
            CustomAnimationEffectPtr pEffect( (*aIter) );
            pEffect->setTarget( makeAny( (*aIter)->getTargetShape() ) );
            pEffect->setTargetSubItem( ShapeAnimationSubType::ONLY_TEXT );
            pTextGroup->addEffect( pEffect );
        }
        else
        {
            // readd the rest to the group again
            while( aIter != aEnd )
            {
                CustomAnimationEffectPtr pEffect( (*aIter++) );

                if( pEffect->getTarget().getValueType() == ::getCppuType((const ParagraphTarget*)0) )
                {
                    pTextGroup->addEffect( pEffect );
                }
                else
                {
                    DBG_ASSERT( !bAnimateForm, "sd::EffectSequenceHelper::setAnimateForm(), something is wrong here!" );
                    remove( pEffect );
                }
            }
        }
        notify_listeners();
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::setTextGroupingAuto( CustomAnimationTextGroupPtr pTextGroup, double fTextGroupingAuto )
{
    sal_Int32 nTextGrouping = pTextGroup->mnTextGrouping;

    EffectSequence aEffects( pTextGroup->maEffects );
    pTextGroup->reset();

    EffectSequence::iterator aIter( aEffects.begin() );
    const EffectSequence::iterator aEnd( aEffects.end() );
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect( (*aIter++) );

        if( pEffect->getTarget().getValueType() == ::getCppuType((const ParagraphTarget*)0) )
        {
            // set correct node type
            if( pEffect->getParaDepth() < nTextGrouping )
            {
                if( fTextGroupingAuto == -1.0 )
                {
                    pEffect->setNodeType( EffectNodeType::ON_CLICK );
                    pEffect->setBegin( 0.0 );
                }
                else
                {
                    pEffect->setNodeType( EffectNodeType::AFTER_PREVIOUS );
                    pEffect->setBegin( fTextGroupingAuto );
                }
            }
            else
            {
                pEffect->setNodeType( EffectNodeType::WITH_PREVIOUS );
                pEffect->setBegin( 0.0 );
            }
        }

        pTextGroup->addEffect( pEffect );

    }
    notify_listeners();
}

// --------------------------------------------------------------------

struct ImplStlTextGroupSortHelper
{
    ImplStlTextGroupSortHelper( bool bReverse ) : mbReverse( bReverse ) {};
    bool operator()( const CustomAnimationEffectPtr& p1, const CustomAnimationEffectPtr& p2 );
    bool mbReverse;
    sal_Int32 getTargetParagraph( const CustomAnimationEffectPtr& p1 );
};

// --------------------------------------------------------------------

sal_Int32 ImplStlTextGroupSortHelper::getTargetParagraph( const CustomAnimationEffectPtr& p1 )
{
    const Any aTarget(p1->getTarget());
    if( aTarget.hasValue() && aTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
    {
        ParagraphTarget aParaTarget;
        aTarget >>= aParaTarget;
        return aParaTarget.Paragraph;
    }
    else
    {
        return mbReverse ? 0x7fffffff : -1;
    }
}

// --------------------------------------------------------------------

bool ImplStlTextGroupSortHelper::operator()( const CustomAnimationEffectPtr& p1, const CustomAnimationEffectPtr& p2 )
{
    if( mbReverse )
    {
        return getTargetParagraph( p2 ) < getTargetParagraph( p1 );
    }
    else
    {
        return getTargetParagraph( p1 ) < getTargetParagraph( p2 );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::setTextReverse( CustomAnimationTextGroupPtr pTextGroup, sal_Bool bTextReverse )
{
    if( pTextGroup->mbTextReverse == bTextReverse )
    {
        // do nothing
    }
    else
    {
        std::vector< CustomAnimationEffectPtr > aSortedVector(pTextGroup->maEffects.size());
        std::copy( pTextGroup->maEffects.begin(), pTextGroup->maEffects.end(), aSortedVector.begin() );
        ImplStlTextGroupSortHelper aSortHelper( bTextReverse );
        std::sort( aSortedVector.begin(), aSortedVector.end(), aSortHelper );

        pTextGroup->reset();

        std::vector< CustomAnimationEffectPtr >::iterator aIter( aSortedVector.begin() );
        const std::vector< CustomAnimationEffectPtr >::iterator aEnd( aSortedVector.end() );

        if( aIter != aEnd )
        {
            pTextGroup->addEffect( (*aIter ) );
            EffectSequence::iterator aInsertIter( find( (*aIter++) ) );
            while( aIter != aEnd )
            {
                CustomAnimationEffectPtr pEffect( (*aIter++) );
                maEffects.erase( find( pEffect ) );
                aInsertIter = maEffects.insert( ++aInsertIter, pEffect );
                pTextGroup->addEffect( pEffect );
            }
        }
        notify_listeners();
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::addListener( ISequenceListener* pListener )
{
    if( std::find( maListeners.begin(), maListeners.end(), pListener ) == maListeners.end() )
        maListeners.push_back( pListener );
}

// --------------------------------------------------------------------

void EffectSequenceHelper::removeListener( ISequenceListener* pListener )
{
    maListeners.remove( pListener );
}

// --------------------------------------------------------------------

struct stl_notify_listeners_func : public std::unary_function<ISequenceListener*, void>
{
    stl_notify_listeners_func() {}
    void operator()(ISequenceListener* pListener) { pListener->notify_change(); }
};

// --------------------------------------------------------------------

void EffectSequenceHelper::notify_listeners()
{
    stl_notify_listeners_func aFunc;
    std::for_each( maListeners.begin(), maListeners.end(), aFunc );
}

// --------------------------------------------------------------------

void EffectSequenceHelper::create( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    DBG_ASSERT( xNode.is(), "sd::EffectSequenceHelper::create(), illegal argument" );

    if( xNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            createEffectsequence( xChildNode );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::create(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::createEffectsequence( const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( xNode.is(), "sd::EffectSequenceHelper::createEffectsequence(), illegal argument" );

    if( xNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );

            createEffects( xChildNode );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::createEffectsequence(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::createEffects( const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( xNode.is(), "sd::EffectSequenceHelper::createEffects(), illegal argument" );

    if( xNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );

            switch( xChildNode->getType() )
            {
            // found an effect
            case AnimationNodeType::PAR:
            case AnimationNodeType::ITERATE:
                {
                    CustomAnimationEffectPtr pEffect( new CustomAnimationEffect( xChildNode ) );

                    if( pEffect->mnNodeType != -1 )
                    {
                        pEffect->setEffectSequence( this );
                        maEffects.push_back(pEffect);
                    }
                }
                break;

            // found an after effect
            case AnimationNodeType::SET:
            case AnimationNodeType::ANIMATECOLOR:
                {
                    processAfterEffect( xChildNode );
                }
                break;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::createEffects(), exception cought!" );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::processAfterEffect( const Reference< XAnimationNode >& xNode )
{
    try
    {
        Reference< XAnimationNode > xMaster;

        Sequence< NamedValue > aUserData( xNode->getUserData() );
        sal_Int32 nLength = aUserData.getLength();
        const NamedValue* p = aUserData.getConstArray();

        while( nLength-- )
        {
            if ( p->Name == "master-element" )
            {
                p->Value >>= xMaster;
                break;
            }
            p++;
        }

        // only process if this is a valid after effect
        if( xMaster.is() )
        {
            CustomAnimationEffectPtr pMasterEffect;

            // find the master effect
            stl_CustomAnimationEffect_search_node_predict aSearchPredict( xMaster );
            EffectSequence::iterator aIter( std::find_if( maEffects.begin(), maEffects.end(), aSearchPredict ) );
            if( aIter != maEffects.end() )
                pMasterEffect = (*aIter );

            if( pMasterEffect.get() )
            {
                pMasterEffect->setHasAfterEffect( true );

                // find out what kind of after effect this is
                if( xNode->getType() == AnimationNodeType::ANIMATECOLOR )
                {
                    // its a dim
                    Reference< XAnimate > xAnimate( xNode, UNO_QUERY_THROW );
                    pMasterEffect->setDimColor( xAnimate->getTo() );
                    pMasterEffect->setAfterEffectOnNext( true );
                }
                else
                {
                    // its a hide
                    Reference< XChild > xNodeChild( xNode, UNO_QUERY_THROW );
                    Reference< XChild > xMasterChild( xMaster, UNO_QUERY_THROW );
                    pMasterEffect->setAfterEffectOnNext( xNodeChild->getParent() != xMasterChild->getParent() );
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::EffectSequenceHelper::processAfterEffect(), exception cought!" );
    }
}

// ====================================================================

class AnimationChangeListener : public cppu::WeakImplHelper1< XChangesListener >
{
public:
    AnimationChangeListener( MainSequence* pMainSequence ) : mpMainSequence( pMainSequence ) {}

    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException);
private:
    MainSequence* mpMainSequence;
};

void SAL_CALL AnimationChangeListener::changesOccurred( const ::com::sun::star::util::ChangesEvent& ) throw (RuntimeException)
{
    if( mpMainSequence )
        mpMainSequence->startRecreateTimer();
}

void SAL_CALL AnimationChangeListener::disposing( const ::com::sun::star::lang::EventObject& ) throw (RuntimeException)
{
}

// ====================================================================

MainSequence::MainSequence()
: mxTimingRootNode( ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.animations.SequenceTimeContainer"), UNO_QUERY )
, mbRebuilding( false )
, mnRebuildLockGuard( 0 )
, mbPendingRebuildRequest( false )
{
    if( mxTimingRootNode.is() )
    {
        Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
        aUserData[0].Name = "node-type";
        aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE;
        mxTimingRootNode->setUserData( aUserData );
    }
    init();
}

// --------------------------------------------------------------------

MainSequence::MainSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
: mxTimingRootNode( xNode, UNO_QUERY )
, mbRebuilding( false )
, mnRebuildLockGuard( 0 )
, mbPendingRebuildRequest( false )
, mbIgnoreChanges( 0 )
{
    init();
}

// --------------------------------------------------------------------

MainSequence::~MainSequence()
{
    reset();
}

// --------------------------------------------------------------------

void MainSequence::init()
{
    mnSequenceType = EffectNodeType::MAIN_SEQUENCE;

    maTimer.SetTimeoutHdl( LINK(this, MainSequence, onTimerHdl) );
    maTimer.SetTimeout(500);

    mxChangesListener.set( new AnimationChangeListener( this ) );

    createMainSequence();
}

// --------------------------------------------------------------------

void MainSequence::reset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xTimingRootNode )
{
    reset();

    mxTimingRootNode.set( xTimingRootNode, UNO_QUERY );

    createMainSequence();
}

// --------------------------------------------------------------------

Reference< ::com::sun::star::animations::XAnimationNode > MainSequence::getRootNode()
{
    DBG_ASSERT( mnRebuildLockGuard == 0, "MainSequence::getRootNode(), rebuild is locked, ist this really what you want?" );

    if( maTimer.IsActive() && mbTimerMode )
    {
        // force a rebuild NOW if one is pending
        maTimer.Stop();
        implRebuild();
    }

    return EffectSequenceHelper::getRootNode();
}

// --------------------------------------------------------------------

void MainSequence::createMainSequence()
{
    if( mxTimingRootNode.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( mxTimingRootNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            sal_Int32 nNodeType = CustomAnimationEffect::get_node_type( xChildNode );
            if( nNodeType == EffectNodeType::MAIN_SEQUENCE )
            {
                mxSequenceRoot.set( xChildNode, UNO_QUERY );
                EffectSequenceHelper::create( xChildNode );
            }
            else if( nNodeType == EffectNodeType::INTERACTIVE_SEQUENCE )
            {
                Reference< XTimeContainer > xInteractiveRoot( xChildNode, UNO_QUERY_THROW );
                InteractiveSequencePtr pIS( new InteractiveSequence( xInteractiveRoot, this ) );
                pIS->addListener( this );
                maInteractiveSequenceList.push_back( pIS );
            }
        }

        // see if we have a mainsequence at all. if not, create one...
        if( !mxSequenceRoot.is() )
        {
            mxSequenceRoot = Reference< XTimeContainer >::query(::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.animations.SequenceTimeContainer"));
            if( mxSequenceRoot.is() )
            {
                uno::Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
                aUserData[0].Name = "node-type";
                aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE;
                mxSequenceRoot->setUserData( aUserData );

                // empty sequence until now, set duration to 0.0
                // explicitly (otherwise, this sequence will never
                // end)
                mxSequenceRoot->setDuration( makeAny((double)0.0) );

                Reference< XAnimationNode > xMainSequenceNode( mxSequenceRoot, UNO_QUERY_THROW );
                mxTimingRootNode->appendChild( xMainSequenceNode );
            }
        }

        updateTextGroups();

        notify_listeners();

        Reference< XChangesNotifier > xNotifier( mxTimingRootNode, UNO_QUERY );
        if( xNotifier.is() )
            xNotifier->addChangesListener( mxChangesListener );
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::MainSequence::create(), exception cought!" );
        return;
    }

    DBG_ASSERT( mxSequenceRoot.is(), "sd::MainSequence::create(), found no main sequence!" );
}

// --------------------------------------------------------------------

void MainSequence::reset()
{
    EffectSequenceHelper::reset();

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end(); ++aIter )
        (*aIter)->reset();
    maInteractiveSequenceList.clear();

    try
    {
        Reference< XChangesNotifier > xNotifier( mxTimingRootNode, UNO_QUERY );
        if( xNotifier.is() )
            xNotifier->removeChangesListener( mxChangesListener );
    }
    catch( Exception& )
    {
        // ...
    }
}

// --------------------------------------------------------------------

InteractiveSequencePtr MainSequence::createInteractiveSequence( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape )
{
    InteractiveSequencePtr pIS;

    // create a new interactive sequence container
    Reference< XTimeContainer > xISRoot( ::comphelper::getProcessServiceFactory()->createInstance("com.sun.star.animations.SequenceTimeContainer"), UNO_QUERY );
    DBG_ASSERT( xISRoot.is(), "sd::MainSequence::createInteractiveSequence(), could not create \"com.sun.star.animations.SequenceTimeContainer\"!");
    if( xISRoot.is() )
    {
        uno::Sequence< ::com::sun::star::beans::NamedValue > aUserData( 1 );
        aUserData[0].Name = "node-type";
        aUserData[0].Value <<= ::com::sun::star::presentation::EffectNodeType::INTERACTIVE_SEQUENCE ;
        xISRoot->setUserData( aUserData );

        Reference< XChild > xChild( mxSequenceRoot, UNO_QUERY_THROW );
        Reference< XAnimationNode > xISNode( xISRoot, UNO_QUERY_THROW );
        Reference< XTimeContainer > xParent( xChild->getParent(), UNO_QUERY_THROW );
        xParent->appendChild( xISNode );
    }
    pIS.reset( new InteractiveSequence( xISRoot, this) );
    pIS->setTriggerShape( xShape );
    pIS->addListener( this );
    maInteractiveSequenceList.push_back( pIS );
    return pIS;
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr MainSequence::findEffect( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) const
{
    CustomAnimationEffectPtr pEffect = EffectSequenceHelper::findEffect( xNode );

    if( pEffect.get() == 0 )
    {
        InteractiveSequenceList::const_iterator aIter;
        for( aIter = maInteractiveSequenceList.begin(); (aIter != maInteractiveSequenceList.end()) && (pEffect.get() == 0); ++aIter )
        {
            pEffect = (*aIter)->findEffect( xNode );
        }
    }
    return pEffect;
}

// --------------------------------------------------------------------

sal_Int32 MainSequence::getOffsetFromEffect( const CustomAnimationEffectPtr& pEffect ) const
{
    sal_Int32 nOffset = EffectSequenceHelper::getOffsetFromEffect( pEffect );

    if( nOffset != -1 )
        return nOffset;

    nOffset = EffectSequenceHelper::getCount();

    InteractiveSequenceList::const_iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end(); ++aIter )
    {
        sal_Int32 nTemp = (*aIter)->getOffsetFromEffect( pEffect );
        if( nTemp != -1 )
            return nOffset + nTemp;

        nOffset += (*aIter)->getCount();
    }

    return -1;
}

// --------------------------------------------------------------------

CustomAnimationEffectPtr MainSequence::getEffectFromOffset( sal_Int32 nOffset ) const
{
    if( nOffset >= 0 )
    {
        if( nOffset < getCount() )
            return EffectSequenceHelper::getEffectFromOffset( nOffset );

        nOffset -= getCount();

        InteractiveSequenceList::const_iterator aIter( maInteractiveSequenceList.begin() );

        while( (aIter != maInteractiveSequenceList.end()) && (nOffset > (*aIter)->getCount()) )
            nOffset -= (*aIter++)->getCount();

        if( (aIter != maInteractiveSequenceList.end()) && (nOffset >= 0) )
            return (*aIter)->getEffectFromOffset( nOffset );
    }

    CustomAnimationEffectPtr pEffect;
    return pEffect;
}

// --------------------------------------------------------------------

bool MainSequence::disposeShape( const Reference< XShape >& xShape )
{
    bool bChanges = EffectSequenceHelper::disposeShape( xShape );

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end();  )
    {
        if( (*aIter)->getTriggerShape() == xShape )
        {
            aIter = maInteractiveSequenceList.erase( aIter );
            bChanges = true;
        }
        else
        {
            bChanges |= (*aIter++)->disposeShape( xShape );
        }
    }

    if( bChanges )
        startRebuildTimer();

    return bChanges;
}

// --------------------------------------------------------------------

bool MainSequence::hasEffect( const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape )
{
    if( EffectSequenceHelper::hasEffect( xShape ) )
        return true;

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end();  )
    {
        if( (*aIter)->getTriggerShape() == xShape )
            return true;

        if( (*aIter++)->hasEffect( xShape ) )
            return true;
    }

    return false;
}

// --------------------------------------------------------------------

void MainSequence::insertTextRange( const com::sun::star::uno::Any& aTarget )
{
    EffectSequenceHelper::insertTextRange( aTarget );

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end(); ++aIter )
    {
        (*aIter)->insertTextRange( aTarget );
    }
}
// --------------------------------------------------------------------

void MainSequence::disposeTextRange( const com::sun::star::uno::Any& aTarget )
{
    EffectSequenceHelper::disposeTextRange( aTarget );

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end(); ++aIter )
    {
        (*aIter)->disposeTextRange( aTarget );
    }
}

// --------------------------------------------------------------------

/** callback from the sd::View when an object just left text edit mode */
void MainSequence::onTextChanged( const Reference< XShape >& xShape )
{
    EffectSequenceHelper::onTextChanged( xShape );

    InteractiveSequenceList::iterator aIter;
    for( aIter = maInteractiveSequenceList.begin(); aIter != maInteractiveSequenceList.end(); ++aIter )
    {
        (*aIter)->onTextChanged( xShape );
    }
}

// --------------------------------------------------------------------

void EffectSequenceHelper::onTextChanged( const Reference< XShape >& xShape )
{
    bool bChanges = false;

    EffectSequence::iterator aIter;
    for( aIter = maEffects.begin(); aIter != maEffects.end(); ++aIter )
    {
        if( (*aIter)->getTargetShape() == xShape )
            bChanges |= (*aIter)->checkForText();
    }

    if( bChanges )
        EffectSequenceHelper::implRebuild();
}

// --------------------------------------------------------------------

void MainSequence::rebuild()
{
    startRebuildTimer();
}

// --------------------------------------------------------------------

void MainSequence::lockRebuilds()
{
    mnRebuildLockGuard++;
}

// --------------------------------------------------------------------

void MainSequence::unlockRebuilds()
{
    DBG_ASSERT( mnRebuildLockGuard, "sd::MainSequence::unlockRebuilds(), no corresponding lockRebuilds() call!" );
    if( mnRebuildLockGuard )
        mnRebuildLockGuard--;

    if( (mnRebuildLockGuard == 0) && mbPendingRebuildRequest )
    {
        mbPendingRebuildRequest = false;
        startRebuildTimer();
    }
}

// --------------------------------------------------------------------

void MainSequence::implRebuild()
{
    if( mnRebuildLockGuard )
    {
        mbPendingRebuildRequest = true;
        return;
    }

    mbRebuilding = true;

    EffectSequenceHelper::implRebuild();

    InteractiveSequenceList::iterator aIter( maInteractiveSequenceList.begin() );
    const InteractiveSequenceList::iterator aEnd( maInteractiveSequenceList.end() );
    while( aIter != aEnd )
    {
        InteractiveSequencePtr pIS( (*aIter) );
        if( pIS->maEffects.empty() )
        {
            // remove empty interactive sequences
            aIter = maInteractiveSequenceList.erase( aIter );

            Reference< XChild > xChild( mxSequenceRoot, UNO_QUERY_THROW );
            Reference< XTimeContainer > xParent( xChild->getParent(), UNO_QUERY_THROW );
            Reference< XAnimationNode > xISNode( pIS->mxSequenceRoot, UNO_QUERY_THROW );
            xParent->removeChild( xISNode );
        }
        else
        {
            pIS->implRebuild();
            ++aIter;
        }
    }

    notify_listeners();
    mbRebuilding = false;
}

// --------------------------------------------------------------------

void MainSequence::notify_change()
{
    notify_listeners();
}

// --------------------------------------------------------------------

bool MainSequence::setTrigger( const CustomAnimationEffectPtr& pEffect, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xTriggerShape )
{
    EffectSequenceHelper* pOldSequence = pEffect->getEffectSequence();

    EffectSequenceHelper* pNewSequence = 0;
    if( xTriggerShape.is() )
    {
        InteractiveSequenceList::iterator aIter( maInteractiveSequenceList.begin() );
        const InteractiveSequenceList::iterator aEnd( maInteractiveSequenceList.end() );
        while( aIter != aEnd )
        {
            InteractiveSequencePtr pIS( (*aIter++) );
            if( pIS->getTriggerShape() == xTriggerShape )
            {
                pNewSequence = pIS.get();
                break;
            }
        }

        if( !pNewSequence )
            pNewSequence = createInteractiveSequence( xTriggerShape ).get();
    }
    else
    {
        pNewSequence = this;
    }

    if( pOldSequence != pNewSequence )
    {
        if( pOldSequence )
            pOldSequence->maEffects.remove( pEffect );
        if( pNewSequence )
            pNewSequence->maEffects.push_back( pEffect );
        pEffect->setEffectSequence( pNewSequence );
        return true;
    }
    else
    {
        return false;
    }

}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(MainSequence, onTimerHdl)
{
    if( mbTimerMode )
    {
        implRebuild();
    }
    else
    {
        reset();
        createMainSequence();
    }

    return 0;
}

// --------------------------------------------------------------------

/** starts a timer that recreates the internal structure from the API core after 1 second */
void MainSequence::startRecreateTimer()
{
    if( !mbRebuilding && (mbIgnoreChanges == 0) )
    {
        mbTimerMode = false;
        maTimer.Start();
    }
}

// --------------------------------------------------------------------

/** starts a timer that rebuilds the API core from the internal structure after 1 second */
void MainSequence::startRebuildTimer()
{
    mbTimerMode = true;
    maTimer.Start();
}

// ====================================================================

InteractiveSequence::InteractiveSequence( const Reference< XTimeContainer >& xSequenceRoot, MainSequence* pMainSequence )
: EffectSequenceHelper( xSequenceRoot ), mpMainSequence( pMainSequence )
{
    mnSequenceType = EffectNodeType::INTERACTIVE_SEQUENCE;

    try
    {
        if( mxSequenceRoot.is() )
        {
            Reference< XEnumerationAccess > xEnumerationAccess( mxSequenceRoot, UNO_QUERY_THROW );
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
            while( !mxEventSource.is() && xEnumeration->hasMoreElements() )
            {
                Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );

                Event aEvent;
                if( (xChildNode->getBegin() >>= aEvent) && (aEvent.Trigger == EventTrigger::ON_CLICK) )
                    aEvent.Source >>= mxEventSource;
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::InteractiveSequence::InteractiveSequence(), exception cought!" );
        return;
    }
}

// --------------------------------------------------------------------

void InteractiveSequence::rebuild()
{
    mpMainSequence->rebuild();
}

void InteractiveSequence::implRebuild()
{
    EffectSequenceHelper::implRebuild();
}

// --------------------------------------------------------------------

MainSequenceRebuildGuard::MainSequenceRebuildGuard( const MainSequencePtr& pMainSequence )
: mpMainSequence( pMainSequence )
{
    if( mpMainSequence.get() )
        mpMainSequence->lockRebuilds();
}

MainSequenceRebuildGuard::~MainSequenceRebuildGuard()
{
    if( mpMainSequence.get() )
        mpMainSequence->unlockRebuilds();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
