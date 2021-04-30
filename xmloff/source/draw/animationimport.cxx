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

#include <memory>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/SequenceTimeContainer.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimatePhysics.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <sax/tools/converter.hxx>

#include <vector>

#include <xmloff/xmltypes.hxx>
#include "sdpropls.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <xmlsdtypes.hxx>

#include <animations.hxx>
#include <animationimport.hxx>

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRangeCompare;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::lang::XInitialization;

static OUString
lcl_GetMediaReference(SvXMLImport const& rImport, OUString const& rURL)
{
    if (rImport.IsPackageURL(rURL))
        return "vnd.sun.star.Package:" + rURL;

    return rImport.GetAbsoluteReference(rURL);
}

namespace xmloff
{

class AnimationsImportHelperImpl
{
private:
    SvXMLImport& mrImport;

public:
    explicit AnimationsImportHelperImpl( SvXMLImport& rImport );

    Any convertValue( XMLTokenEnum eAttributeName, const OUString& rValue );
    Sequence< Any > convertValueSequence( XMLTokenEnum eAttributeName, const OUString& rValue );

    Any convertTarget( const OUString& rValue );
    static Any convertPath( const OUString& rValue );
    Any convertTiming( const OUString& rValue );
    static Sequence< double > convertKeyTimes( const OUString& rValue );
    static Sequence< TimeFilterPair > convertTimeFilter( const OUString& rValue );
};

AnimationsImportHelperImpl::AnimationsImportHelperImpl( SvXMLImport& rImport )
:   mrImport( rImport )
{
}

static bool isDouble( std::string_view rValue )
{
    sal_Int32 nLength = rValue.size();
    const char * pStr = rValue.data();
    while( nLength )
    {
        if( (*pStr >= '0' && *pStr <= '9') || *pStr == '-' || *pStr == '.' || *pStr == '+' || *pStr == 'e' || *pStr == 'E' )
        {
            pStr++;
            nLength--;
        }
        else
        {
            return false;
        }
    }

    return true;
}

static bool isTime( const OUString& rValue )
{
    sal_Int32 nLength = rValue.getLength();
    const sal_Unicode * pStr;
    for( pStr = rValue.getStr(); nLength; pStr++, nLength-- )
    {
        if( !( (*pStr >= '0' && *pStr <= '9') || *pStr == '-' || *pStr == '.' || *pStr == '+' || *pStr == 'e' || *pStr == 'E' ) )
            break;
    }

    // return true if this is a double (if someone forgot the 's' we silently ignore it)
    // or if it's a double that ends with a 's' or 'S'
    return (nLength == 0) || ((*pStr == 's' || *pStr == 'S') && (nLength == 1));
}

Any AnimationsImportHelperImpl::convertTarget( const OUString& rValue )
{
    try
    {
        Reference< XInterface > xRef( mrImport.getInterfaceToIdentifierMapper().getReference( rValue ) );

        Reference< XShape > _xShape( xRef, UNO_QUERY );
        if( _xShape.is() )
            return makeAny( _xShape );

        Reference< XTextCursor > xTextCursor( xRef, UNO_QUERY );
        if( xTextCursor.is() )
        {
            Reference< XTextRange > xStart( xTextCursor->getStart() ), xRange;
            Reference< XShape > xShape( xTextCursor->getText(), UNO_QUERY_THROW );
            Reference< XTextRangeCompare > xTextRangeCompare( xShape, UNO_QUERY_THROW );

            Reference< XEnumerationAccess > xParaEnumAccess( xShape, UNO_QUERY_THROW );
            Reference< XEnumeration > xEnumeration( xParaEnumAccess->createEnumeration(), UNO_SET_THROW );
            sal_Int16 nParagraph = 0;

            while( xEnumeration->hasMoreElements() )
            {
                xEnumeration->nextElement() >>= xRange;

                // break if start of selection is prior to end of current paragraph
                if( xRange.is() && (xTextRangeCompare->compareRegionEnds( xStart, xRange ) >= 0 ) )
                {
                    return makeAny( ParagraphTarget( xShape, nParagraph ) );
                }

                nParagraph++;
            }
        }
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }

    Any aAny;
    return aAny;
}

Any AnimationsImportHelperImpl::convertValue( XMLTokenEnum eAttributeName, const OUString& rValue )
{
    sal_Int32 nCommaPos = -1, nPos;
    sal_Int32 nOpenBrakets = 0;
    for( nPos = 0; (nPos < rValue.getLength()) && (nCommaPos == -1); nPos++ )
    {
        switch( rValue[nPos] )
        {
        case ',':
            if( nOpenBrakets == 0 )
                nCommaPos = nPos;
            break;
        case '(':
        case '[':
        case '{':
            nOpenBrakets++;
            break;
        case ')':
        case ']':
        case '}':
            nOpenBrakets--;
            break;
        }
    }

    if( nCommaPos >= 0 )
    {
        ValuePair aPair;
        aPair.First = convertValue( eAttributeName, rValue.copy( 0, nCommaPos ) );
        aPair.Second = convertValue( eAttributeName, rValue.copy( nCommaPos+1 ) );
        return makeAny( aPair );
    }
    else
    {
        Any aAny;
        sal_Int32 nType = XML_TYPE_STRING;

        if( rValue.getLength() ) switch( eAttributeName )
        {
        case XML_X:
        case XML_Y:
        case XML_WIDTH:
        case XML_HEIGHT:
        case XML_TRANSLATE:
        {
            return makeAny( rValue );
        }

        case XML_SCALE:
        case XML_SKEWY:
        case XML_SKEWX:
        case XML_OPACITY:
        case XML_ROTATE:            nType = XML_TYPE_DOUBLE;                    break;
        case XML_TEXT_ROTATION_ANGLE:nType = XML_TYPE_TEXT_ROTATION_ANGLE;      break;
        case XML_FILL_COLOR:
        case XML_STROKE_COLOR:
        case XML_DIM:
        case XML_COLOR:             nType = XML_TYPE_COLOR;                     break;
        case XML_FILL:              nType = XML_SD_TYPE_FILLSTYLE;              break;
        case XML_STROKE:            nType = XML_SD_TYPE_STROKE;                 break;
        case XML_FONT_WEIGHT:       nType = XML_TYPE_TEXT_WEIGHT;               break;
        case XML_FONT_STYLE:        nType = XML_TYPE_TEXT_POSTURE;              break;
        case XML_TEXT_UNDERLINE:    nType = XML_TYPE_TEXT_UNDERLINE_STYLE;      break;
        case XML_FONT_SIZE:         nType = XML_TYPE_DOUBLE_PERCENT;            break;
        case XML_VISIBILITY:        nType = XML_SD_TYPE_PRESPAGE_VISIBILITY;    break;

        default:
            if( !rValue.isEmpty() )
                aAny <<= rValue;
            return aAny;
        }

        const XMLPropertyHandler* pHandler = mrImport.GetShapeImport()->GetSdPropHdlFactory()->GetPropertyHandler( nType );
        if( pHandler )
            pHandler->importXML( rValue, aAny, mrImport.GetMM100UnitConverter() );

        return aAny;
    }
}

Sequence< Any > AnimationsImportHelperImpl::convertValueSequence( XMLTokenEnum eAttributeName, const OUString& rValue )
{
    Sequence< Any > aValues;

    const sal_Int32 nElements { comphelper::string::getTokenCount(rValue, ';') };
    if ( nElements>0 )
    {
        // prepare the sequence
        aValues.realloc( nElements );

        // fill the sequence
        Any* pValues = aValues.getArray();
        for (sal_Int32 nIndex = 0; nIndex >= 0; )
            *pValues++ = convertValue( eAttributeName, rValue.getToken( 0, ';', nIndex ) );
    }

    return aValues;
}

Any AnimationsImportHelperImpl::convertTiming( const OUString& rValue )
{
    Any aAny;

    const sal_Int32 nElements { comphelper::string::getTokenCount(rValue, ';') };
    if ( nElements>0 )
    {
        if( nElements == 1 )
        {
            if( IsXMLToken( rValue, XML_MEDIA ) )
            {
                aAny <<= Timing_MEDIA;
            }
            else if( IsXMLToken( rValue, XML_INDEFINITE ) )
            {
                aAny <<= Timing_INDEFINITE;
            }
            else if( isTime( rValue ) )
            {
                aAny <<= rValue.toDouble();
            }
            else
            {
                Event aEvent;
                aEvent.Repeat = 0;
                aEvent.Trigger = 0;

                OUString aEventTrigger;

                sal_Int32 nPos = rValue.indexOf( '+' );
                if( nPos == -1 )
                {
                    aEventTrigger = rValue;
                }
                else
                {
                    aEventTrigger = rValue.copy( 0, nPos );

                    // convert offset
                    aEvent.Offset = convertTiming( rValue.copy( nPos + 1 ) );
                }

                nPos = aEventTrigger.indexOf( '.' );
                if( nPos != -1 )
                {
                    aEvent.Source <<= mrImport.getInterfaceToIdentifierMapper().getReference( aEventTrigger.copy( 0, nPos ) );
                    aEventTrigger = aEventTrigger.copy( nPos + 1 );
                }

                sal_Int16 nEnum;
                if( SvXMLUnitConverter::convertEnum( nEnum, aEventTrigger, aAnimations_EnumMap_EventTrigger ) )
                {
                    aEvent.Trigger = nEnum;
                }
                else
                {
                    OSL_FAIL("AnimationsImportHelperImpl::convertTiming(), unknown event trigger!");
                }

                aAny <<= aEvent;
            }
        }
        else
        {
            // fill the sequence
            Sequence< Any > aValues( nElements );
            Any* pValues = aValues.getArray();
            for (sal_Int32 nIndex = 0; nIndex >= 0; )
                *pValues++ = convertTiming( rValue.getToken( 0, ';', nIndex ) );

            aAny <<= aValues;
        }
    }
    return aAny;
}

Sequence< double > AnimationsImportHelperImpl::convertKeyTimes( const OUString& rValue )
{
    const sal_Int32 nElements { comphelper::string::getTokenCount(rValue, ';') };

    Sequence< double > aKeyTimes( nElements );

    if( nElements )
    {
        double* pValues = aKeyTimes.getArray();
        for (sal_Int32 nIndex = 0; nIndex >= 0; )
            *pValues++ = rValue.getToken( 0, ';', nIndex ).toDouble();
    }

    return aKeyTimes;
}

Sequence< TimeFilterPair > AnimationsImportHelperImpl::convertTimeFilter( const OUString& rValue )
{
    const sal_Int32 nElements { comphelper::string::getTokenCount(rValue, ';') };

    Sequence< TimeFilterPair > aTimeFilter( nElements );

    if( nElements )
    {
        TimeFilterPair* pValues = aTimeFilter.getArray();
        for (sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            const OUString aToken( rValue.getToken( 0, ';', nIndex ) );

            sal_Int32 nPos = aToken.indexOf( ',' );
            if( nPos >= 0 )
            {
                pValues->Time = aToken.copy( 0, nPos ).toDouble();
                pValues->Progress = aToken.copy( nPos+1 ).toDouble();
            }
            pValues++;
        }
    }

    return aTimeFilter;
}

Any AnimationsImportHelperImpl::convertPath( const OUString& rValue )
{
    return makeAny( rValue );
}


AnimationNodeContext::AnimationNodeContext(
        const Reference< XAnimationNode >& xParentNode,
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        const std::shared_ptr<AnimationsImportHelperImpl>& pHelper )
:   SvXMLImportContext(rImport),
    mpHelper( pHelper )
{
    bool bRootContext = !pHelper;
    try
    {
        if( bRootContext )
        {
            mpHelper = std::make_shared<AnimationsImportHelperImpl>( rImport );
            mxNode = xParentNode;
        }
        else
        {
            sal_Int16 nPresetClass = EffectPresetClass::CUSTOM;

            const char* pServiceName = nullptr;

            // we see namespace ANIMATION and ANIMATION_OOO and PRESENTATION_OASIS and PRESENTATION_SO52 and PRESENTATION_OOO
            switch( nElement & TOKEN_MASK )
            {
            case XML_SEQ:
                pServiceName = "com.sun.star.animations.SequenceTimeContainer"; break;
            case XML_ITERATE:
                pServiceName = "com.sun.star.animations.IterateContainer"; break;
            case XML_ANIMATE:
                pServiceName = "com.sun.star.animations.Animate"; break;
            case XML_SET:
                pServiceName = "com.sun.star.animations.AnimateSet"; break;
            case XML_ANIMATEMOTION:
                pServiceName = "com.sun.star.animations.AnimateMotion"; break;
            case XML_ANIMATEPHYSICS:
                pServiceName = "com.sun.star.animations.AnimatePhysics"; break;
            case XML_ANIMATECOLOR:
                pServiceName = "com.sun.star.animations.AnimateColor"; break;
            case XML_ANIMATETRANSFORM:
                pServiceName = "com.sun.star.animations.AnimateTransform"; break;
            case XML_TRANSITIONFILTER:
                pServiceName = "com.sun.star.animations.TransitionFilter"; break;
            case XML_AUDIO:
                pServiceName = "com.sun.star.animations.Audio"; break;
            case XML_COMMAND:
                pServiceName = "com.sun.star.animations.Command"; break;
            case XML_PAR:
                {
                    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
                    {
                        if( (aIter.getToken() & TOKEN_MASK) == XML_PRESET_ID)
                        {
                            const OUString& rValue = aIter.toString();
                            if ( rValue == "ooo-entrance-random" )
                            {
                                nPresetClass = EffectPresetClass::ENTRANCE;
                            }
                            else if ( rValue == "ooo-exit-random" )
                            {
                                nPresetClass = EffectPresetClass::EXIT;
                            }

                            if( nPresetClass != EffectPresetClass::CUSTOM )
                            {
                                pServiceName = "com.sun.star.comp.sd.RandomAnimationNode";
                                break;
                            }
                        }
                    }
                    if( !pServiceName )
                        pServiceName = "com.sun.star.animations.ParallelTimeContainer";
                }
                break;
            default:
                SAL_WARN("xmloff", "unexpected token '" + SvXMLImport::getNameFromToken(nElement)
                            << "' 0x" << std::hex << nElement);
                break;
            }

            if( pServiceName )
            {
                Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

                mxNode.set(
                    xContext->getServiceManager()->createInstanceWithContext(OUString::createFromAscii(pServiceName), xContext),
                    UNO_QUERY_THROW );

                if( nPresetClass != EffectPresetClass::CUSTOM )
                {
                    Reference< XInitialization > xInit( mxNode, UNO_QUERY_THROW );
                    const Any aAny( makeAny( nPresetClass ) );
                    Sequence< Any > aArgs( &aAny, 1 ) ;
                    xInit->initialize( aArgs );
                }

                init_node( xAttrList );

                Reference< XTimeContainer > xParentContainer( xParentNode, UNO_QUERY_THROW );
                xParentContainer->appendChild( mxNode );
            }
        }
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

void AnimationNodeContext::init_node(  const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( !mxNode.is() )
        return;

    try
    {
        const sal_Int16 nNodeType = mxNode->getType();

        // query for optional interfaces that are often used later
        Reference< XAnimate > xAnimate( mxNode, UNO_QUERY );
        Reference< XCommand > xCommand( mxNode, UNO_QUERY );
        Reference< XTransitionFilter > xTransitionFilter( mxNode, UNO_QUERY );
        Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );

        std::vector< NamedValue > aUserData;
        XMLTokenEnum meAttributeName = XML_TOKEN_INVALID;
        OUString aFrom, aBy, aTo, aValues;
        bool bHaveXmlId( false );
        OUString sXmlId;

        sal_Int16 nEnum;
        for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
        {
            auto nToken = aIter.getToken();
            switch( nToken )
            {
            case XML_ELEMENT(SMIL, XML_BEGIN):
            case XML_ELEMENT(SMIL_COMPAT, XML_BEGIN):
            case XML_ELEMENT(SMIL_SO52, XML_BEGIN):
            {
                mxNode->setBegin( mpHelper->convertTiming( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(SMIL, XML_DUR):
            case XML_ELEMENT(SMIL_COMPAT, XML_DUR):
            case XML_ELEMENT(SMIL_SO52, XML_DUR):
            {
                mxNode->setDuration( mpHelper->convertTiming( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(SMIL, XML_END):
            case XML_ELEMENT(SMIL_COMPAT, XML_END):
            case XML_ELEMENT(SMIL_SO52, XML_END):
            {
                mxNode->setEnd( mpHelper->convertTiming( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(SMIL, XML_FILL):
            case XML_ELEMENT(SMIL_COMPAT, XML_FILL):
            case XML_ELEMENT(SMIL_SO52, XML_FILL):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_Fill ) )
                    mxNode->setFill( nEnum );
            }
            break;
            case XML_ELEMENT(SMIL, XML_FILLDEFAULT):
            case XML_ELEMENT(SMIL_COMPAT, XML_FILLDEFAULT):
            case XML_ELEMENT(SMIL_SO52, XML_FILLDEFAULT):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_FillDefault ) )
                    mxNode->setFillDefault( nEnum );
            }
            break;
            case XML_ELEMENT(SMIL, XML_RESTART):
            case XML_ELEMENT(SMIL_COMPAT, XML_RESTART):
            case XML_ELEMENT(SMIL_SO52, XML_RESTART):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_Restart ) )
                    mxNode->setRestart( nEnum );
            }
            break;
            case XML_ELEMENT(SMIL, XML_RESTARTDEFAULT):
            case XML_ELEMENT(SMIL_COMPAT, XML_RESTARTDEFAULT):
            case XML_ELEMENT(SMIL_SO52, XML_RESTARTDEFAULT):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_RestartDefault ) )
                    mxNode->setRestartDefault( nEnum );
            }
            break;
            case XML_ELEMENT(SMIL, XML_ACCELERATE):
            case XML_ELEMENT(SMIL_COMPAT, XML_ACCELERATE):
            case XML_ELEMENT(SMIL_SO52, XML_ACCELERATE):
            {
                if( isDouble( aIter.toView() ) )
                    mxNode->setAcceleration( aIter.toDouble() );
            }
            break;
            case XML_ELEMENT(SMIL, XML_DECELERATE):
            case XML_ELEMENT(SMIL_COMPAT, XML_DECELERATE):
            case XML_ELEMENT(SMIL_SO52, XML_DECELERATE):
            {
                if( isDouble( aIter.toView() ) )
                    mxNode->setDecelerate( aIter.toDouble() );
            }
            break;
            case XML_ELEMENT(SMIL, XML_AUTOREVERSE):
            case XML_ELEMENT(SMIL_COMPAT, XML_AUTOREVERSE):
            case XML_ELEMENT(SMIL_SO52, XML_AUTOREVERSE):
            {
                bool bTemp;
                if (::sax::Converter::convertBool( bTemp, aIter.toView() ))
                    mxNode->setAutoReverse( bTemp  );
            }
            break;
            case XML_ELEMENT(SMIL, XML_REPEATCOUNT):
            case XML_ELEMENT(SMIL_COMPAT, XML_REPEATCOUNT):
            case XML_ELEMENT(SMIL_SO52, XML_REPEATCOUNT):
            {
                mxNode->setRepeatCount( mpHelper->convertTiming( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(SMIL, XML_REPEATDUR):
            case XML_ELEMENT(SMIL_COMPAT, XML_REPEATDUR):
            case XML_ELEMENT(SMIL_SO52, XML_REPEATDUR):
            {
                mxNode->setRepeatDuration( mpHelper->convertTiming( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(SMIL, XML_ENDSYNC):
            case XML_ELEMENT(SMIL_COMPAT, XML_ENDSYNC):
            case XML_ELEMENT(SMIL_SO52, XML_ENDSYNC):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_Endsync ) )
                    mxNode->setEndSync( makeAny( nEnum ) );
            }
            break;
            case XML_ELEMENT(PRESENTATION, XML_NODE_TYPE):
            case XML_ELEMENT(PRESENTATION_SO52, XML_NODE_TYPE):
            case XML_ELEMENT(PRESENTATION_OOO, XML_NODE_TYPE):
            case XML_ELEMENT(PRESENTATION_OASIS, XML_NODE_TYPE):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_EffectNodeType ) )
                    aUserData.emplace_back( GetXMLToken( XML_NODE_TYPE ), makeAny( nEnum ) );
            }
            break;
            case XML_ELEMENT(PRESENTATION, XML_PRESET_ID):
            case XML_ELEMENT(PRESENTATION_SO52, XML_PRESET_ID):
            case XML_ELEMENT(PRESENTATION_OOO, XML_PRESET_ID):
            case XML_ELEMENT(PRESENTATION_OASIS, XML_PRESET_ID):
            {
                aUserData.emplace_back( GetXMLToken( XML_PRESET_ID ), makeAny( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(PRESENTATION, XML_PRESET_SUB_TYPE):
            case XML_ELEMENT(PRESENTATION_SO52, XML_PRESET_SUB_TYPE):
            case XML_ELEMENT(PRESENTATION_OOO, XML_PRESET_SUB_TYPE):
            case XML_ELEMENT(PRESENTATION_OASIS, XML_PRESET_SUB_TYPE):
            {
                aUserData.emplace_back( GetXMLToken( XML_PRESET_SUB_TYPE ), makeAny( aIter.toString() ) );
            }
            break;
            case XML_ELEMENT(PRESENTATION, XML_PRESET_CLASS):
            case XML_ELEMENT(PRESENTATION_SO52, XML_PRESET_CLASS):
            case XML_ELEMENT(PRESENTATION_OOO, XML_PRESET_CLASS):
            case XML_ELEMENT(PRESENTATION_OASIS, XML_PRESET_CLASS):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_EffectPresetClass ) )
                    aUserData.emplace_back( GetXMLToken( XML_PRESET_CLASS ), makeAny( nEnum ) );
            }
            break;
            case XML_ELEMENT(PRESENTATION, XML_AFTER_EFFECT):
            case XML_ELEMENT(PRESENTATION_SO52, XML_AFTER_EFFECT):
            case XML_ELEMENT(PRESENTATION_OOO, XML_AFTER_EFFECT):
            {
                bool bTemp;
                if (::sax::Converter::convertBool( bTemp, aIter.toView() ))
                    aUserData.emplace_back( GetXMLToken( XML_AFTER_EFFECT ), makeAny( bTemp ) );
            }
            break;
            case XML_ELEMENT(XLINK, XML_HREF):
            {
                if( nNodeType == AnimationNodeType::AUDIO )
                {
                    Reference< XAudio > xAudio( mxNode, UNO_QUERY_THROW );
                    xAudio->setSource( makeAny(lcl_GetMediaReference(GetImport(), aIter.toString())) );
                    break;
                }
                [[fallthrough]];
            }
            case XML_ELEMENT(SMIL, XML_TARGETELEMENT):
            case XML_ELEMENT(SMIL_COMPAT, XML_TARGETELEMENT):
            case XML_ELEMENT(SMIL_SO52, XML_TARGETELEMENT):
            {
                Any aTarget( mpHelper->convertTarget( aIter.toString() ) );

                if( xAnimate.is() )
                {
                    xAnimate->setTarget( aTarget );
                }
                else if( xIter.is() )
                {
                    xIter->setTarget( aTarget );
                }
                else if( xCommand.is() )
                {
                    xCommand->setTarget( aTarget );
                }
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_AUDIO_LEVEL):
            case XML_ELEMENT(ANIMATION_OOO, XML_AUDIO_LEVEL):
            {
                if( nNodeType == AnimationNodeType::AUDIO )
                {
                    if( isDouble( aIter.toView() ) )
                    {
                        Reference< XAudio > xAudio( mxNode, UNO_QUERY_THROW );
                        xAudio->setVolume( aIter.toDouble() );
                    }
                }
            }
            break;

            case XML_ELEMENT(PRESENTATION, XML_MASTER_ELEMENT):
            case XML_ELEMENT(PRESENTATION_SO52, XML_MASTER_ELEMENT):
            case XML_ELEMENT(PRESENTATION_OOO, XML_MASTER_ELEMENT):
            {
                Reference< XAnimationNode > xMaster( GetImport().getInterfaceToIdentifierMapper().getReference( aIter.toString() ), UNO_QUERY );
                aUserData.emplace_back( GetXMLToken( XML_MASTER_ELEMENT ), makeAny( xMaster ) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_SUB_ITEM):
            case XML_ELEMENT(ANIMATION_OOO, XML_SUB_ITEM):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_SubItem ) )
                {
                    if( xAnimate.is() )
                    {
                        xAnimate->setSubItem( nEnum );
                    }
                    else if( xIter.is() )
                    {
                        xIter->setSubItem( nEnum );
                    }
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_ATTRIBUTENAME):
            case XML_ELEMENT(SMIL_COMPAT, XML_ATTRIBUTENAME):
            case XML_ELEMENT(SMIL_SO52, XML_ATTRIBUTENAME):
            {
                if( xAnimate.is() )
                {
                    OUString aName( aIter.toString() );

                    const struct ImplAttributeNameConversion* p = getAnimationAttributeNamesConversionList();
                    while( p->mpAPIName )
                    {
                        if( IsXMLToken( aIter, p->meXMLToken ) )
                        {
                            aName = OUString::createFromAscii( p->mpAPIName );
                            meAttributeName = p->meXMLToken;
                            break;
                        }

                        p++;
                    }

                    xAnimate->setAttributeName( aName );
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_VALUES):
            case XML_ELEMENT(SMIL_COMPAT, XML_VALUES):
            case XML_ELEMENT(SMIL_SO52, XML_VALUES):
            {
                aValues = aIter.toString();
            }
            break;

            case XML_ELEMENT(SMIL, XML_FROM):
            case XML_ELEMENT(SMIL_COMPAT, XML_FROM):
            case XML_ELEMENT(SMIL_SO52, XML_FROM):
            {
                aFrom = aIter.toString();
            }
            break;

            case XML_ELEMENT(SMIL, XML_BY):
            case XML_ELEMENT(SMIL_COMPAT, XML_BY):
            case XML_ELEMENT(SMIL_SO52, XML_BY):
            {
                aBy = aIter.toString();
            }
            break;

            case XML_ELEMENT(SMIL, XML_TO):
            case XML_ELEMENT(SMIL_COMPAT, XML_TO):
            case XML_ELEMENT(SMIL_SO52, XML_TO):
            {
                aTo = aIter.toString();
            }
            break;

            case XML_ELEMENT(SMIL, XML_KEYTIMES):
            case XML_ELEMENT(SMIL_COMPAT, XML_KEYTIMES):
            case XML_ELEMENT(SMIL_SO52, XML_KEYTIMES):
            {
                if( xAnimate.is() )
                    xAnimate->setKeyTimes( AnimationsImportHelperImpl::convertKeyTimes( aIter.toString() ) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_FORMULA):
            case XML_ELEMENT(ANIMATION_OOO, XML_FORMULA):
            {
                if( xAnimate.is() )
                    xAnimate->setFormula( aIter.toString() );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_ID):
            case XML_ELEMENT(ANIMATION_OOO, XML_ID):
            {
                if (!bHaveXmlId) { sXmlId = aIter.toString(); }
            }
            break;
            case XML_ELEMENT(XML, XML_ID):
            {
                sXmlId = aIter.toString();
                bHaveXmlId = true;
            }
            break;

            case XML_ELEMENT(SMIL, XML_CALCMODE):
            case XML_ELEMENT(SMIL_COMPAT, XML_CALCMODE):
            case XML_ELEMENT(SMIL_SO52, XML_CALCMODE):
            {
                if( xAnimate.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_CalcMode ) )
                        xAnimate->setCalcMode( nEnum );
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_ACCUMULATE):
            case XML_ELEMENT(SMIL_COMPAT, XML_ACCUMULATE):
            case XML_ELEMENT(SMIL_SO52, XML_ACCUMULATE):
            {
                if( xAnimate.is() )
                    xAnimate->setAccumulate( IsXMLToken( aIter, XML_SUM ) );
            }
            break;

            case XML_ELEMENT(PRESENTATION, XML_ADDITIVE):
            case XML_ELEMENT(PRESENTATION_SO52, XML_ADDITIVE):
            case XML_ELEMENT(PRESENTATION_OOO, XML_ADDITIVE):
            case XML_ELEMENT(SMIL, XML_ADDITIVE):
            case XML_ELEMENT(SMIL_COMPAT, XML_ADDITIVE):
            case XML_ELEMENT(SMIL_SO52, XML_ADDITIVE):
            {
                if( xAnimate.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_AdditiveMode ) )
                        xAnimate->setAdditive( nEnum );
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_KEYSPLINES):
            case XML_ELEMENT(SMIL_COMPAT, XML_KEYSPLINES):
            case XML_ELEMENT(SMIL_SO52, XML_KEYSPLINES):
            {
                if( xAnimate.is() )
                    xAnimate->setTimeFilter( AnimationsImportHelperImpl::convertTimeFilter( aIter.toString() ) );
            }
            break;

            case XML_ELEMENT(SVG, XML_PATH):
            case XML_ELEMENT(SVG_COMPAT, XML_PATH):
            {
                Reference< XAnimateMotion > xAnimateMotion( mxNode, UNO_QUERY );
                if( xAnimateMotion.is() )
                    xAnimateMotion->setPath( AnimationsImportHelperImpl::convertPath( aIter.toString() ) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_PHYSICS_ANIMATION_START_VELOCITY_X):
            case XML_ELEMENT(ANIMATION_OOO, XML_PHYSICS_ANIMATION_START_VELOCITY_X):
            case XML_ELEMENT(LO_EXT, XML_PHYSICS_ANIMATION_START_VELOCITY_X):
            {
                Reference< XAnimatePhysics > xAnimatePhysics( mxNode, UNO_QUERY );
                if( xAnimatePhysics.is() )
                    xAnimatePhysics->setStartVelocityX( makeAny(aIter.toDouble()) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_PHYSICS_ANIMATION_START_VELOCITY_Y):
            case XML_ELEMENT(ANIMATION_OOO, XML_PHYSICS_ANIMATION_START_VELOCITY_Y):
            case XML_ELEMENT(LO_EXT, XML_PHYSICS_ANIMATION_START_VELOCITY_Y):
            {
                Reference< XAnimatePhysics > xAnimatePhysics( mxNode, UNO_QUERY );
                if( xAnimatePhysics.is() )
                    xAnimatePhysics->setStartVelocityY( makeAny(aIter.toDouble()) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_PHYSICS_ANIMATION_DENSITY):
            case XML_ELEMENT(ANIMATION_OOO, XML_PHYSICS_ANIMATION_DENSITY):
            case XML_ELEMENT(LO_EXT, XML_PHYSICS_ANIMATION_DENSITY):
            {
                Reference< XAnimatePhysics > xAnimatePhysics( mxNode, UNO_QUERY );
                if( xAnimatePhysics.is() )
                    xAnimatePhysics->setDensity( makeAny(aIter.toDouble()) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_PHYSICS_ANIMATION_BOUNCINESS):
            case XML_ELEMENT(ANIMATION_OOO, XML_PHYSICS_ANIMATION_BOUNCINESS):
            case XML_ELEMENT(LO_EXT, XML_PHYSICS_ANIMATION_BOUNCINESS):
            {
                Reference< XAnimatePhysics > xAnimatePhysics( mxNode, UNO_QUERY );
                if( xAnimatePhysics.is() )
                    xAnimatePhysics->setBounciness( makeAny(aIter.toDouble()) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_COLOR_INTERPOLATION):
            case XML_ELEMENT(ANIMATION_OOO, XML_COLOR_INTERPOLATION):
            {
                Reference< XAnimateColor > xAnimateColor( mxNode, UNO_QUERY );
                if( xAnimateColor.is() )
                    xAnimateColor->setColorInterpolation( IsXMLToken( aIter, XML_HSL ) ? AnimationColorSpace::HSL : AnimationColorSpace::RGB );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_COLOR_INTERPOLATION_DIRECTION):
            case XML_ELEMENT(ANIMATION_OOO, XML_COLOR_INTERPOLATION_DIRECTION):
            {
                Reference< XAnimateColor > xAnimateColor( mxNode, UNO_QUERY );
                if( xAnimateColor.is() )
                    xAnimateColor->setDirection( IsXMLToken( aIter, XML_CLOCKWISE ) );
            }
            break;

            case XML_ELEMENT(SVG, XML_TYPE):
            case XML_ELEMENT(SVG_COMPAT, XML_TYPE):
            {
                Reference< XAnimateTransform > xTransform( mxNode, UNO_QUERY );
                if( xTransform.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_TransformType ) )
                    {
                        xTransform->setTransformType( nEnum );
                        switch( nEnum )
                        {
                        case AnimationTransformType::SCALE: meAttributeName = XML_SCALE; break;
                        case AnimationTransformType::ROTATE: meAttributeName = XML_ROTATE; break;
                        case AnimationTransformType::SKEWX: meAttributeName = XML_SKEWX; break;
                        case AnimationTransformType::SKEWY: meAttributeName = XML_SKEWY; break;
                        //case AnimationTransformType::TRANSLATE:
                        default:
                            meAttributeName = XML_TRANSLATE; break;
                        }
                    }
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_TYPE):
            case XML_ELEMENT(SMIL_COMPAT, XML_TYPE):
            case XML_ELEMENT(SMIL_SO52, XML_TYPE):
            {
                if( xTransitionFilter.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_TransitionType ) )
                        xTransitionFilter->setTransition( nEnum );
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_SUBTYPE):
            case XML_ELEMENT(SMIL_COMPAT, XML_SUBTYPE):
            case XML_ELEMENT(SMIL_SO52, XML_SUBTYPE):
            {
                if( xTransitionFilter.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_TransitionSubType ) )
                        xTransitionFilter->setSubtype( nEnum );
                }
            }
            break;

            case XML_ELEMENT(SMIL, XML_MODE):
            case XML_ELEMENT(SMIL_COMPAT, XML_MODE):
            case XML_ELEMENT(SMIL_SO52, XML_MODE):
            {
                if( xTransitionFilter.is() )
                    xTransitionFilter->setMode( IsXMLToken( aIter, XML_IN ) );
            }
            break;

            case XML_ELEMENT(SMIL, XML_DIRECTION):
            case XML_ELEMENT(SMIL_COMPAT, XML_DIRECTION):
            case XML_ELEMENT(SMIL_SO52, XML_DIRECTION):
            {
                if( xTransitionFilter.is() )
                    xTransitionFilter->setDirection( IsXMLToken( aIter, XML_FORWARD ) );
            }
            break;

            case XML_ELEMENT(SMIL, XML_FADECOLOR):
            case XML_ELEMENT(SMIL_COMPAT, XML_FADECOLOR):
            case XML_ELEMENT(SMIL_SO52, XML_FADECOLOR):
            {
                if( xTransitionFilter.is() )
                {
                    sal_Int32 nColor(0);
                    ::sax::Converter::convertColor(nColor, aIter.toView());
                    xTransitionFilter->setFadeColor(nColor);
                }
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_ITERATE_TYPE):
            case XML_ELEMENT(ANIMATION_OOO, XML_ITERATE_TYPE):
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_IterateType ) )
                {
                    if( xIter.is() )
                        xIter->setIterateType( nEnum );
                }
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_ITERATE_INTERVAL):
            case XML_ELEMENT(ANIMATION_OOO, XML_ITERATE_INTERVAL):
            {
                if( xIter.is() )
                {
                    OUString rValue = aIter.toString();
                    double fInterval = 0.0;
                    if( rValue.match("P") )
                    {
                        css::util::Duration aDuration;
                        if (::sax::Converter::convertDuration(aDuration, rValue))
                        {
                            fInterval = ((((aDuration.Hours * 60)
                                + aDuration.Minutes) * 60) + aDuration.Seconds)
                              + (aDuration.NanoSeconds / 1000000000.0);
                        }
                    }
                    else
                    {
                        fInterval = aIter.toDouble();
                    }

                    xIter->setIterateInterval( fInterval );
                }
            }
            break;

            case XML_ELEMENT(PRESENTATION, XML_GROUP_ID):
            case XML_ELEMENT(PRESENTATION_SO52, XML_GROUP_ID):
            case XML_ELEMENT(PRESENTATION_OOO, XML_GROUP_ID):
            {
                aUserData.emplace_back( "group-id", makeAny( aIter.toInt32() ) );
            }
            break;

            case XML_ELEMENT(ANIMATION, XML_COMMAND):
            case XML_ELEMENT(ANIMATION_OOO, XML_COMMAND):
            {
                if( xCommand.is() && nNodeType == AnimationNodeType::COMMAND )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, aIter.toView(), aAnimations_EnumMap_Command ) )
                    {
                        xCommand->setCommand( nEnum );
                    }
                }
            }
            break;

            default:
            {
                // push all unknown attributes within the presentation namespace as user data
                if (IsTokenInNamespace(nToken, XML_NAMESPACE_PRESENTATION)
                    || IsTokenInNamespace(nToken, XML_NAMESPACE_PRESENTATION_SO52)
                    || IsTokenInNamespace(nToken, XML_NAMESPACE_PRESENTATION_OASIS)
                    || IsTokenInNamespace(nToken, XML_NAMESPACE_PRESENTATION_OOO))
                {
                    aUserData.emplace_back( SvXMLImport::getNameFromToken(aIter.getToken()), makeAny( aIter.toString() ) );
                }
                else
                    XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            }
            }
        }

        if (!sXmlId.isEmpty())
        {
            Reference< XInterface > const xRef( mxNode, UNO_QUERY );
            GetImport().getInterfaceToIdentifierMapper().registerReference(
                sXmlId, xRef );
        }

        sal_Int32 nUserDataCount = aUserData.size();
        if( nUserDataCount )
        {
            Sequence< NamedValue > aUnoUserData( nUserDataCount );
            NamedValue* pData = aUnoUserData.getArray();
            for (auto const& item : aUserData)
                *pData++ = item;

            mxNode->setUserData( aUnoUserData );
        }

        // convert values
        if( xAnimate.is() )
        {
            if( !aFrom.isEmpty() )
                xAnimate->setFrom( mpHelper->convertValue( meAttributeName, aFrom ) );

            if( !aBy.isEmpty() )
                xAnimate->setBy( mpHelper->convertValue( meAttributeName, aBy ) );

            if( !aTo.isEmpty() )
                xAnimate->setTo( mpHelper->convertValue( meAttributeName, aTo ) );

            if( !aValues.isEmpty() )
                xAnimate->setValues( mpHelper->convertValueSequence( meAttributeName, aValues ) );

            if (xAnimate->getValues().getLength() != xAnimate->getKeyTimes().getLength())
                throw css::io::WrongFormatException();
        }
    }
    catch (const css::io::WrongFormatException&)
    {
        throw;
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler >  AnimationNodeContext::createFastChildContext(sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    if( mxNode.is())
        return new AnimationNodeContext( mxNode, GetImport(), nElement, xAttrList, mpHelper );
    return nullptr;
}

namespace {

class AnimationsImport: public SvXMLImport, public XAnimationNodeSupplier
{
public:
    explicit AnimationsImport( const Reference< XComponentContext > & rxContext );

    SvXMLImportContext* CreateFastContext(sal_Int32 nElement,
                    const Reference<XFastAttributeList>& xAttrList) override;

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XAnimationNodeSupplier
    Reference< XAnimationNode > SAL_CALL getAnimationNode() override;

private:
    Reference< XAnimationNode > mxRootNode;
};

}

AnimationsImport::AnimationsImport( const Reference< XComponentContext > & rxContext )
: SvXMLImport( rxContext, "xmloff::AnimationsImport", SvXMLImportFlags::META )
    //FIXME: the above "IMPORT_META" used to be a nonsensical "true", question
    // remains whether this should be IMPORT_META (same numerical value as
    // true) or default IMPORT_ALL
{
    mxRootNode.set( SequenceTimeContainer::create(rxContext), UNO_QUERY_THROW );
}

// XInterface
Any SAL_CALL AnimationsImport::queryInterface( const Type& aType )
{
    if ( aType == cppu::UnoType<XAnimationNodeSupplier>::get())
    {
        return makeAny( Reference<XAnimationNodeSupplier>( this ) );
    }
    else
    {
        return SvXMLImport::queryInterface( aType );
    }
}

void SAL_CALL AnimationsImport::acquire() noexcept
{
    SvXMLImport::acquire();
}

void SAL_CALL AnimationsImport::release() noexcept
{
    SvXMLImport::release();
}

SvXMLImportContext *AnimationsImport::CreateFastContext(
        sal_Int32 nElement,
        const Reference<XFastAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;

    if( nElement == XML_ELEMENT(ANIMATION, XML_SEQ) || nElement == XML_ELEMENT(ANIMATION_OOO, XML_SEQ) )
    {
         pContext = new AnimationNodeContext( mxRootNode, *this, nElement, xAttrList );
    }

    return pContext;
}

// XAnimationNodeSupplier
Reference< XAnimationNode > SAL_CALL AnimationsImport::getAnimationNode()
{
    return mxRootNode;
}

void AnimationNodeContext::postProcessRootNode( const Reference< XAnimationNode >& xRootNode, Reference< XPropertySet > const & xPageProps )
{
    if( !(xRootNode.is() && xPageProps.is()) )
        return;

    try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_SET_THROW );
        if( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            if( xNode->getType() == AnimationNodeType::PAR )
            {
                Event aEvent;
                if( (xNode->getBegin() >>= aEvent) && (aEvent.Trigger == EventTrigger::BEGIN_EVENT) )
                {
                    // found transition node
                    Reference< XEnumerationAccess > xChildEnumerationAccess( xNode, UNO_QUERY_THROW );
                    Reference< XEnumeration > xChildEnumeration( xChildEnumerationAccess->createEnumeration(), UNO_SET_THROW );
                    while( xChildEnumeration->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xChildNode( xChildEnumeration->nextElement(), UNO_QUERY_THROW );
                        switch( xChildNode->getType() )
                        {
                        case AnimationNodeType::TRANSITIONFILTER:
                        {
                            Reference< XTransitionFilter > xTransFilter( xChildNode, UNO_QUERY_THROW );

                            xPageProps->setPropertyValue("TransitionType", Any( xTransFilter->getTransition() ) );
                            xPageProps->setPropertyValue("TransitionSubtype", Any( xTransFilter->getSubtype() ) );
                            xPageProps->setPropertyValue("TransitionDirection", Any( xTransFilter->getDirection() ) );
                            xPageProps->setPropertyValue("TransitionFadeColor", Any( xTransFilter->getFadeColor() ) );

                            double fDuration;
                            if( xTransFilter->getDuration() >>= fDuration )
                                xPageProps->setPropertyValue("TransitionDuration", Any( fDuration ) );

                        }
                        break;

                        case AnimationNodeType::COMMAND:
                        {
                            Reference< XCommand > xCommand( xChildNode, UNO_QUERY_THROW );
                            if( xCommand->getCommand() == EffectCommands::STOPAUDIO )
                            {
                                xPageProps->setPropertyValue("Sound", Any(true) );
                            }
                        }
                        break;

                        case AnimationNodeType::AUDIO:
                        {
                            Reference< XAudio > xAudio( xChildNode, UNO_QUERY_THROW );
                            OUString sSoundURL;
                            if( (xAudio->getSource() >>= sSoundURL) && !sSoundURL.isEmpty() )
                            {
                                xPageProps->setPropertyValue("Sound", Any(sSoundURL) );

                                Timing eTiming;
                                if( (xAudio->getRepeatCount() >>= eTiming) && (eTiming == Timing_INDEFINITE) )
                                    xPageProps->setPropertyValue("LoopSound", Any( true ) );
                            }
                        }
                        break;

                        }
                    }

                    Reference< XTimeContainer > xRootContainer( xRootNode, UNO_QUERY_THROW );
                    xRootContainer->removeChild( xNode );
                }
            }
        }
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

} // namespace xmloff

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Xmloff_AnimationsImport(uno::XComponentContext* pCtx,
                                          uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new xmloff::AnimationsImport(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
