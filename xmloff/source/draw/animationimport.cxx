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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/SequenceTimeContainer.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
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
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <sax/tools/converter.hxx>

#include <list>
#include <xmloff/xmltypes.hxx>
#include "sdpropls.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <osl/mutex.hxx>
#include <xmloff/nmspmap.hxx>
#include "anim.hxx"

#include "animations.hxx"
#include "animationimport.hxx"

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::text::XTextRange;
using ::com::sun::star::text::XTextCursor;
using ::com::sun::star::text::XTextRangeCompare;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XInitialization;

namespace xmloff
{

class AnimationsImportHelperImpl
{
private:
    SvXMLImport& mrImport;

    SvXMLTokenMap* mpAnimationNodeTokenMap;
    SvXMLTokenMap* mpAnimationNodeAttributeTokenMap;

public:
    AnimationsImportHelperImpl( SvXMLImport& rImport );
    ~AnimationsImportHelperImpl();

    const SvXMLTokenMap& getAnimationNodeTokenMap();
    const SvXMLTokenMap& getAnimationNodeAttributeTokenMap();

    Any convertValue( XMLTokenEnum eAttributeName, const OUString& rValue );
    Sequence< Any > convertValueSequence( XMLTokenEnum eAttributeName, const OUString& rValue );

    Any convertTarget( const OUString& rValue );
    Any convertPath( const OUString& rValue );
    Any convertTiming( const OUString& rValue );
    Sequence< double > convertKeyTimes( const OUString& rValue );
    Sequence< TimeFilterPair > convertTimeFilter( const OUString& rValue );

    bool convertAnimationValue( XMLTokenEnum eAttributeName, Any& rValue );
    const OUString mastrHSL;
};

AnimationsImportHelperImpl::AnimationsImportHelperImpl( SvXMLImport& rImport )
:   mrImport( rImport ),
    mpAnimationNodeTokenMap( NULL ),
    mpAnimationNodeAttributeTokenMap( NULL ),
    mastrHSL( "hsl" )
{
}

AnimationsImportHelperImpl::~AnimationsImportHelperImpl()
{
    delete mpAnimationNodeTokenMap;
    delete mpAnimationNodeAttributeTokenMap;
}

const SvXMLTokenMap& AnimationsImportHelperImpl::getAnimationNodeTokenMap()
{
    if( mpAnimationNodeTokenMap == NULL )
    {
        static const SvXMLTokenMapEntry aAnimationNodeTokenMap[] =
        {
            { XML_NAMESPACE_ANIMATION,  XML_PAR,                (sal_uInt16)AnimationNodeType::PAR },
            { XML_NAMESPACE_ANIMATION,  XML_SEQ,                (sal_uInt16)AnimationNodeType::SEQ },
            { XML_NAMESPACE_ANIMATION,  XML_ITERATE,            (sal_uInt16)AnimationNodeType::ITERATE },
            { XML_NAMESPACE_ANIMATION,  XML_ANIMATE,            (sal_uInt16)AnimationNodeType::ANIMATE },
            { XML_NAMESPACE_ANIMATION,  XML_SET,                (sal_uInt16)AnimationNodeType::SET },
            { XML_NAMESPACE_ANIMATION,  XML_ANIMATEMOTION,      (sal_uInt16)AnimationNodeType::ANIMATEMOTION },
            { XML_NAMESPACE_ANIMATION,  XML_ANIMATECOLOR,       (sal_uInt16)AnimationNodeType::ANIMATECOLOR },
            { XML_NAMESPACE_ANIMATION,  XML_ANIMATETRANSFORM,   (sal_uInt16)AnimationNodeType::ANIMATETRANSFORM },
            { XML_NAMESPACE_ANIMATION,  XML_TRANSITIONFILTER,   (sal_uInt16)AnimationNodeType::TRANSITIONFILTER },
            { XML_NAMESPACE_ANIMATION,  XML_AUDIO,              (sal_uInt16)AnimationNodeType::AUDIO },
            { XML_NAMESPACE_ANIMATION,  XML_COMMAND,            (sal_uInt16)AnimationNodeType::COMMAND },
            XML_TOKEN_MAP_END
        };

        mpAnimationNodeTokenMap = new SvXMLTokenMap( aAnimationNodeTokenMap );
    }

    return *mpAnimationNodeTokenMap;
}

enum AnimationNodeAttributes
{
    ANA_Begin,
    ANA_Dur,
    ANA_End,
    ANA_Fill,
    ANA_FillDefault,
    ANA_Restart,
    ANA_RestartDefault,
    ANA_Accelerate,
    ANA_Decelerate,
    ANA_AutoReverse,
    ANA_RepeatCount,
    ANA_RepeatDur,
    ANA_EndSync,
    ANA_Node_Type,
    ANA_Preset_ID,
    ANA_Preset_Sub_Type,
    ANA_Preset_Class,
    ANA_After_Effect,
    ANA_Target,
    ANA_XLink,
    ANA_MasterElement,
    ANA_SubItem,
    ANA_AttributeName,
    ANA_Values,
    ANA_From,
    ANA_By,
    ANA_To,
    ANA_KeyTimes,
    ANA_CalcMode,
    ANA_Accumulate,
    ANA_AdditiveMode,
    ANA_KeySplines,
    ANA_Path,
    ANA_ColorSpace,
    ANA_ColorDirection,
    ANA_TransformType,
    ANA_TransitionType,
    ANA_TransitionSubType,
    ANA_Mode,
    ANA_Direction,
    ANA_FadeColor,
    ANA_IterateType,
    ANA_IterateInterval,
    ANA_Formula,
    ANA_ANIMID,
    ANA_XMLID,
    ANA_Group_Id,
    ANA_Command,
    ANA_Volume
};

const SvXMLTokenMap& AnimationsImportHelperImpl::getAnimationNodeAttributeTokenMap()
{
    if( mpAnimationNodeAttributeTokenMap == NULL )
    {
        static const SvXMLTokenMapEntry aAnimationNodeAttributeTokenMap[] =
        {
            { XML_NAMESPACE_SMIL, XML_BEGIN,                    (sal_uInt16)ANA_Begin },
            { XML_NAMESPACE_SMIL, XML_DUR,                      (sal_uInt16)ANA_Dur },
            { XML_NAMESPACE_SMIL, XML_END,                      (sal_uInt16)ANA_End },
            { XML_NAMESPACE_SMIL, XML_FILL,                     (sal_uInt16)ANA_Fill },
            { XML_NAMESPACE_SMIL, XML_FILLDEFAULT,              (sal_uInt16)ANA_FillDefault },
            { XML_NAMESPACE_SMIL, XML_RESTART,                  (sal_uInt16)ANA_Restart },
            { XML_NAMESPACE_SMIL, XML_RESTARTDEFAULT,           (sal_uInt16)ANA_RestartDefault },
            { XML_NAMESPACE_SMIL, XML_ACCELERATE,               (sal_uInt16)ANA_Accelerate },
            { XML_NAMESPACE_SMIL, XML_DECELERATE,               (sal_uInt16)ANA_Decelerate },
            { XML_NAMESPACE_SMIL, XML_AUTOREVERSE,              (sal_uInt16)ANA_AutoReverse },
            { XML_NAMESPACE_SMIL, XML_REPEATCOUNT,              (sal_uInt16)ANA_RepeatCount },
            { XML_NAMESPACE_SMIL, XML_REPEATDUR,                (sal_uInt16)ANA_RepeatDur },
            { XML_NAMESPACE_SMIL, XML_ENDSYNC,                  (sal_uInt16)ANA_EndSync },
            { XML_NAMESPACE_PRESENTATION, XML_NODE_TYPE,        (sal_uInt16)ANA_Node_Type },
            { XML_NAMESPACE_PRESENTATION, XML_PRESET_ID,        (sal_uInt16)ANA_Preset_ID },
            { XML_NAMESPACE_PRESENTATION, XML_PRESET_SUB_TYPE,  (sal_uInt16)ANA_Preset_Sub_Type },
            { XML_NAMESPACE_PRESENTATION, XML_PRESET_CLASS,     (sal_uInt16)ANA_Preset_Class },
            { XML_NAMESPACE_PRESENTATION, XML_AFTER_EFFECT,     (sal_uInt16)ANA_After_Effect },
            { XML_NAMESPACE_SMIL, XML_TARGETELEMENT,            (sal_uInt16)ANA_Target },
            { XML_NAMESPACE_XLINK, XML_HREF,                    (sal_uInt16)ANA_XLink },
            { XML_NAMESPACE_PRESENTATION, XML_MASTER_ELEMENT,   (sal_uInt16)ANA_MasterElement },
            { XML_NAMESPACE_ANIMATION, XML_SUB_ITEM,            (sal_uInt16)ANA_SubItem },
            { XML_NAMESPACE_SMIL, XML_ATTRIBUTENAME,            (sal_uInt16)ANA_AttributeName },
            { XML_NAMESPACE_SMIL, XML_VALUES,                   (sal_uInt16)ANA_Values },
            { XML_NAMESPACE_SMIL, XML_FROM,                     (sal_uInt16)ANA_From },
            { XML_NAMESPACE_SMIL, XML_BY,                       (sal_uInt16)ANA_By },
            { XML_NAMESPACE_SMIL, XML_TO,                       (sal_uInt16)ANA_To },
            { XML_NAMESPACE_SMIL, XML_KEYTIMES,                 (sal_uInt16)ANA_KeyTimes },
            { XML_NAMESPACE_SMIL, XML_CALCMODE,                 (sal_uInt16)ANA_CalcMode },
            { XML_NAMESPACE_SMIL, XML_ACCUMULATE,               (sal_uInt16)ANA_Accumulate },
            { XML_NAMESPACE_PRESENTATION, XML_ADDITIVE,         (sal_uInt16)ANA_AdditiveMode },
            { XML_NAMESPACE_SMIL, XML_ADDITIVE,                 (sal_uInt16)ANA_AdditiveMode },
            { XML_NAMESPACE_SMIL, XML_KEYSPLINES,               (sal_uInt16)ANA_KeySplines },
            { XML_NAMESPACE_SVG, XML_PATH,                      (sal_uInt16)ANA_Path },
            { XML_NAMESPACE_ANIMATION, XML_COLOR_INTERPOLATION, (sal_uInt16)ANA_ColorSpace },
            { XML_NAMESPACE_ANIMATION, XML_COLOR_INTERPOLATION_DIRECTION,       (sal_uInt16)ANA_ColorDirection },
            { XML_NAMESPACE_SVG, XML_TYPE,                      (sal_uInt16)ANA_TransformType },
            { XML_NAMESPACE_SMIL, XML_TYPE,                     (sal_uInt16)ANA_TransitionType },
            { XML_NAMESPACE_SMIL, XML_SUBTYPE,                  (sal_uInt16)ANA_TransitionSubType },
            { XML_NAMESPACE_SMIL, XML_MODE,                     (sal_uInt16)ANA_Mode },
            { XML_NAMESPACE_SMIL, XML_DIRECTION,                (sal_uInt16)ANA_Direction },
            { XML_NAMESPACE_SMIL, XML_FADECOLOR,                (sal_uInt16)ANA_FadeColor },
            { XML_NAMESPACE_ANIMATION, XML_ITERATE_TYPE,        (sal_uInt16)ANA_IterateType },
            { XML_NAMESPACE_ANIMATION, XML_ITERATE_INTERVAL,    (sal_uInt16)ANA_IterateInterval },
            { XML_NAMESPACE_ANIMATION, XML_FORMULA,             (sal_uInt16)ANA_Formula },
            { XML_NAMESPACE_ANIMATION, XML_ID,                  (sal_uInt16)ANA_ANIMID },
            { XML_NAMESPACE_XML, XML_ID,                        (sal_uInt16)ANA_XMLID },
            { XML_NAMESPACE_PRESENTATION, XML_GROUP_ID,         (sal_uInt16)ANA_Group_Id },
            { XML_NAMESPACE_ANIMATION, XML_AUDIO_LEVEL,         (sal_uInt16)ANA_Volume },
            { XML_NAMESPACE_ANIMATION, XML_COMMAND,             (sal_uInt16)ANA_Command },

            XML_TOKEN_MAP_END
        };

        mpAnimationNodeAttributeTokenMap = new SvXMLTokenMap( aAnimationNodeAttributeTokenMap );
    }

    return *mpAnimationNodeAttributeTokenMap;
}

static bool isDouble( const OUString& rValue )
{
    sal_Int32 nLength = rValue.getLength();
    const sal_Unicode * pStr = rValue.getStr();
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
    // or if its a double that ends with a 's' or 'S'
    return (nLength == 0) || ((*pStr == 's' || *pStr == 'S') && (nLength == 1));
}

static sal_Int32 count_codes( const OUString& rString, sal_Unicode nCode )
{
    sal_Int32 nCount = 0;
    sal_Int32 fromIndex = 0;

    while(true)
    {
        fromIndex = rString.indexOf( nCode, fromIndex );
        if( fromIndex == -1 )
            break;

        fromIndex++;
        nCount++;
    }

    return nCount;
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
            Reference< XEnumeration > xEnumeration( xParaEnumAccess->createEnumeration(), UNO_QUERY_THROW );
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
        OSL_FAIL( "xmloff::AnimationsImportImpl::convertTarget(), RuntimeException caught!" );
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
        aPair.Second = convertValue( eAttributeName, rValue.copy( nCommaPos+1, rValue.getLength() - nCommaPos - 1 ) );
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

    // do we have any value at all?
    if( !rValue.isEmpty() )
    {
        sal_Int32 nElements = count_codes( rValue, (sal_Unicode)';') + 1; // a non empty string has at least one value

        // prepare the sequence
        aValues.realloc( nElements );

        // fill the sequence
        Any* pValues = aValues.getArray();
        sal_Int32 nIndex;
        for( nIndex = 0; nElements && (nIndex >= 0); nElements-- )
        {
            *pValues++ = convertValue( eAttributeName, rValue.getToken( 0, ';', nIndex ) );
        }
    }

    return aValues;
}

Any AnimationsImportHelperImpl::convertTiming( const OUString& rValue )
{
    Any aAny;

    // do we have any value at all?
    if( !rValue.isEmpty() )
    {
        // count the values
        sal_Int32 nElements = count_codes( rValue, (sal_Unicode)';' ) + 1; // a non empty string has at least one value

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

                sal_Int32 nPos = rValue.indexOf( (sal_Unicode)'+' );
                if( nPos == -1 )
                {
                    aEventTrigger = rValue;
                }
                else
                {
                    aEventTrigger = rValue.copy( 0, nPos );

                    // convert offset
                    aEvent.Offset <<= convertTiming( rValue.copy( nPos + 1 ) );
                }

                nPos = aEventTrigger.indexOf( (sal_Unicode)'.' );
                if( nPos != -1 )
                {
                    aEvent.Source <<= mrImport.getInterfaceToIdentifierMapper().getReference( aEventTrigger.copy( 0, nPos ) );
                    aEventTrigger = aEventTrigger.copy( nPos + 1 );
                }

                sal_uInt16 nEnum;
                if( SvXMLUnitConverter::convertEnum( nEnum, aEventTrigger, getAnimationsEnumMap(Animations_EnumMap_EventTrigger) ) )
                {
                    aEvent.Trigger = (sal_Int16)nEnum;
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
            sal_Int32 nIndex = 0;
            while( (nElements--) && (nIndex >= 0) )
                *pValues++ = convertTiming( rValue.getToken( 0, ';', nIndex ) );

            aAny <<= aValues;
        }
    }
    return aAny;
}

Sequence< double > AnimationsImportHelperImpl::convertKeyTimes( const OUString& rValue )
{
    sal_Int32 nElements = 0;

    if( !rValue.isEmpty() )
        nElements = count_codes( rValue, (sal_Unicode)';' ) + 1; // a non empty string has at least one value

    Sequence< double > aKeyTimes( nElements );

    if( nElements )
    {
        double* pValues = aKeyTimes.getArray();
        sal_Int32 nIndex = 0;
        while( (nElements--) && (nIndex >= 0) )
            *pValues++ = rValue.getToken( 0, ';', nIndex ).toDouble();
    }

    return aKeyTimes;
}

Sequence< TimeFilterPair > AnimationsImportHelperImpl::convertTimeFilter( const OUString& rValue )
{
    sal_Int32 nElements = 0;

    if( !rValue.isEmpty() )
        nElements = count_codes( rValue, (sal_Unicode)';' ) + 1; // a non empty string has at least one value

    Sequence< TimeFilterPair > aTimeFilter( nElements );

    if( nElements )
    {
        TimeFilterPair* pValues = aTimeFilter.getArray();
        sal_Int32 nIndex = 0;
        while( (nElements--) && (nIndex >= 0) )
        {
            const OUString aToken( rValue.getToken( 0, ';', nIndex ) );

            sal_Int32 nPos = aToken.indexOf( ',' );
            if( nPos >= 0 )
            {
                pValues->Time = aToken.copy( 0, nPos ).toDouble();
                pValues->Progress = aToken.copy( nPos+1, aToken.getLength() - nPos - 1 ).toDouble();
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

TYPEINIT1( AnimationNodeContext, SvXMLImportContext );

AnimationNodeContext::AnimationNodeContext(
        const Reference< XAnimationNode >& xParentNode,
        SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        AnimationsImportHelperImpl* pHelper /* = NULL */ )
:   SvXMLImportContext(rImport, nPrfx, rLocalName),
    mpHelper( pHelper ),
    mbRootContext( pHelper == NULL )
{
    try
    {
        if( mbRootContext )
        {
            mpHelper = new AnimationsImportHelperImpl( rImport );
            mxNode = xParentNode;
        }
        else
        {
            sal_Int16 nPresetClass = EffectPresetClass::CUSTOM;

            const sal_Char* pServiceName = 0;

            sal_Int16 nNodeType = (sal_Int16)mpHelper->getAnimationNodeTokenMap().Get( nPrfx, rLocalName );
            switch( nNodeType )
            {
            case AnimationNodeType::SEQ:                pServiceName = "com.sun.star.animations.SequenceTimeContainer"; break;
            case AnimationNodeType::ITERATE:            pServiceName = "com.sun.star.animations.IterateContainer"; break;
            case AnimationNodeType::ANIMATE:            pServiceName = "com.sun.star.animations.Animate"; break;
            case AnimationNodeType::SET:                pServiceName = "com.sun.star.animations.AnimateSet"; break;
            case AnimationNodeType::ANIMATEMOTION:      pServiceName = "com.sun.star.animations.AnimateMotion"; break;
            case AnimationNodeType::ANIMATECOLOR:       pServiceName = "com.sun.star.animations.AnimateColor"; break;
            case AnimationNodeType::ANIMATETRANSFORM:   pServiceName = "com.sun.star.animations.AnimateTransform"; break;
            case AnimationNodeType::TRANSITIONFILTER:   pServiceName = "com.sun.star.animations.TransitionFilter"; break;
            case AnimationNodeType::AUDIO:              pServiceName = "com.sun.star.animations.Audio"; break;
            case AnimationNodeType::COMMAND:            pServiceName = "com.sun.star.animations.Command"; break;
            case AnimationNodeType::PAR:
                {
                    const sal_Int16 nCount = xAttrList.is() ? xAttrList->getLength() : 0;
                    sal_Int16 nAttribute;
                    for( nAttribute = 0; nAttribute < nCount; nAttribute++ )
                    {
                        OUString aLocalName;
                        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( nAttribute ), &aLocalName );
                        if( (nPrefix == XML_NAMESPACE_PRESENTATION) && IsXMLToken( aLocalName, XML_PRESET_ID ) )
                        {
                            const OUString& rValue = xAttrList->getValueByIndex( nAttribute );
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
                pServiceName = 0;
            }

            if( pServiceName )
            {
                Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

                mxNode = Reference< XAnimationNode >(
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
        OSL_FAIL( "xmloff::AnimationsImportImpl::AnimationsImportImpl(), RuntimeException caught!" );
    }
}

AnimationNodeContext::~AnimationNodeContext()
{
    if( mbRootContext )
        delete mpHelper;
}

void AnimationNodeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    // code of StartElement is moved to init_node that is now called
    // in c'tor before appending this node to its parent.
    // This is needed for random nodes that need the correct target
    // set when child nodes are appended.
}

void AnimationNodeContext::init_node(  const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    if( mxNode.is() ) try
    {
        const sal_Int16 nNodeType = mxNode->getType();

        // query for optional interfaces that are often used later
        Reference< XAnimate > xAnimate( mxNode, UNO_QUERY );
        Reference< XCommand > xCommand( mxNode, UNO_QUERY );
        Reference< XTransitionFilter > xTransitionFilter( mxNode, UNO_QUERY );
        Reference< XIterateContainer > xIter( mxNode, UNO_QUERY );

        std::list< NamedValue > aUserData;
        XMLTokenEnum meAttributeName = XML_TOKEN_INVALID;
        OUString aFrom, aBy, aTo, aValues;
        bool bHaveXmlId( false );
        OUString sXmlId;

        const sal_Int16 nCount = xAttrList.is() ? xAttrList->getLength() : 0;
        sal_uInt16 nEnum;
        sal_Int16 nAttribute;
        for( nAttribute = 0; nAttribute < nCount; nAttribute++ )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( nAttribute );
            const OUString& rValue = xAttrList->getValueByIndex( nAttribute );

            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
            switch( mpHelper->getAnimationNodeAttributeTokenMap().Get( nPrefix, aLocalName ) )
            {
            case ANA_Begin:
            {
                mxNode->setBegin( mpHelper->convertTiming( rValue ) );
            }
            break;
            case ANA_Dur:
            {
                mxNode->setDuration( mpHelper->convertTiming( rValue ) );
            }
            break;
            case ANA_End:
            {
                mxNode->setEnd( mpHelper->convertTiming( rValue ) );
            }
            break;
            case ANA_Fill:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_Fill) ) )
                    mxNode->setFill( (sal_Int16)nEnum );
            }
            break;
            case ANA_FillDefault:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_FillDefault) ) )
                    mxNode->setFillDefault( (sal_Int16)nEnum );
            }
            break;
            case ANA_Restart:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_Restart) ) )
                    mxNode->setRestart( (sal_Int16)nEnum );
            }
            break;
            case ANA_RestartDefault:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_RestartDefault) ) )
                    mxNode->setRestartDefault( (sal_Int16)nEnum );
            }
            break;
            case ANA_Accelerate:
            {
                if( isDouble( rValue ) )
                    mxNode->setAcceleration( rValue.toDouble() );
            }
            break;
            case ANA_Decelerate:
            {
                if( isDouble( rValue ) )
                    mxNode->setDecelerate( rValue.toDouble() );
            }
            break;
            case ANA_AutoReverse:
            {
                bool bTemp;
                if (::sax::Converter::convertBool( bTemp, rValue ))
                    mxNode->setAutoReverse( bTemp  );
            }
            break;
            case ANA_RepeatCount:
            {
                mxNode->setRepeatCount( mpHelper->convertTiming( rValue ) );
            }
            break;
            case ANA_RepeatDur:
            {
                mxNode->setRepeatDuration( mpHelper->convertTiming( rValue ) );
            }
            break;
            case ANA_EndSync:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_Endsync) ) )
                    mxNode->setEndSync( makeAny( (sal_Int16)nEnum ) );
            }
            break;
            case ANA_Node_Type:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_EffectNodeType) ) )
                    aUserData.push_back( NamedValue( GetXMLToken( XML_NODE_TYPE ), makeAny( (sal_Int16)nEnum ) ) );
            }
            break;
            case ANA_Preset_ID:
            {
                aUserData.push_back( NamedValue( GetXMLToken( XML_PRESET_ID ), makeAny( rValue ) ) );
            }
            break;
            case ANA_Preset_Sub_Type:
            {
                aUserData.push_back( NamedValue( GetXMLToken( XML_PRESET_SUB_TYPE ), makeAny( rValue ) ) );
            }
            break;
            case ANA_Preset_Class:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_EffectPresetClass) ) )
                    aUserData.push_back( NamedValue( GetXMLToken( XML_PRESET_CLASS ), makeAny( (sal_Int16)nEnum ) ) );
            }
            break;
            case ANA_After_Effect:
            {
                bool bTemp;
                if (::sax::Converter::convertBool( bTemp, rValue ))
                    aUserData.push_back( NamedValue( GetXMLToken( XML_AFTER_EFFECT ), makeAny( bTemp ) ) );
            }
            break;
            case ANA_XLink:
            {
                if( nNodeType == AnimationNodeType::AUDIO )
                {
                    Reference< XAudio > xAudio( mxNode, UNO_QUERY_THROW );
                    xAudio->setSource( makeAny( GetImport().GetAbsoluteReference( rValue ) ) );
                    break;
                }

            }
            // fall through intented!
            case ANA_Target:
            {
                {
                    Any aTarget( mpHelper->convertTarget( rValue ) );

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
            }
            break;

            case ANA_Volume:
            {
                if( nNodeType == AnimationNodeType::AUDIO )
                {
                    if( isDouble( rValue ) )
                    {
                        Reference< XAudio > xAudio( mxNode, UNO_QUERY_THROW );
                        xAudio->setVolume( rValue.toDouble() );
                    }
                }
            }
            break;

            case ANA_MasterElement:
            {
                Reference< XAnimationNode > xMaster( GetImport().getInterfaceToIdentifierMapper().getReference( rValue ), UNO_QUERY );
                aUserData.push_back( NamedValue( GetXMLToken( XML_MASTER_ELEMENT ), makeAny( xMaster ) ) );
            }
            break;

            case ANA_SubItem:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_SubItem) ) )
                {
                    if( xAnimate.is() )
                    {
                        xAnimate->setSubItem( (sal_Int16)nEnum );
                    }
                    else if( xIter.is() )
                    {
                        xIter->setSubItem( (sal_Int16)nEnum );
                    }
                }
            }
            break;

            case ANA_AttributeName:
            {
                if( xAnimate.is() )
                {
                    OUString aName( rValue );

                    const struct ImplAttributeNameConversion* p = getAnimationAttributeNamesConversionList();
                    while( p->mpAPIName )
                    {
                        if( IsXMLToken( aName, p->meXMLToken ) )
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

            case ANA_Values:
            {
                aValues = rValue;
            }
            break;

            case ANA_From:
            {
                aFrom = rValue;
            }
            break;

            case ANA_By:
            {
                aBy = rValue;
            }
            break;

            case ANA_To:
            {
                aTo = rValue;
            }
            break;

            case ANA_KeyTimes:
            {
                if( xAnimate.is() )
                    xAnimate->setKeyTimes( mpHelper->convertKeyTimes( rValue ) );
            }
            break;

            case ANA_Formula:
            {
                if( xAnimate.is() )
                    xAnimate->setFormula( rValue );
            }
            break;

            case ANA_ANIMID:
            {
                if (!bHaveXmlId) { sXmlId = rValue; }
            }
            break;
            case ANA_XMLID:
            {
                sXmlId = rValue;
                bHaveXmlId = true;
            }
            break;

            case ANA_CalcMode:
            {
                if( xAnimate.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_CalcMode) ) )
                        xAnimate->setCalcMode( (sal_Int16)nEnum );
                }
            }
            break;

            case ANA_Accumulate:
            {
                if( xAnimate.is() )
                    xAnimate->setAccumulate( IsXMLToken( rValue, XML_SUM ) );
            }
            break;

            case ANA_AdditiveMode:
            {
                if( xAnimate.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_AdditiveMode) ) )
                        xAnimate->setAdditive( (sal_Int16)nEnum );
                }
            }
            break;

            case ANA_KeySplines:
            {
                if( xAnimate.is() )
                    xAnimate->setTimeFilter( mpHelper->convertTimeFilter( rValue ) );
            }
            break;

            case ANA_Path:
            {
                Reference< XAnimateMotion > xAnimateMotion( mxNode, UNO_QUERY );
                if( xAnimateMotion.is() )
                    xAnimateMotion->setPath( mpHelper->convertPath( rValue ) );
            }
            break;

            case ANA_ColorSpace:
            {
                Reference< XAnimateColor > xAnimateColor( mxNode, UNO_QUERY );
                if( xAnimateColor.is() )
                    xAnimateColor->setColorInterpolation( IsXMLToken( rValue, XML_HSL ) ? AnimationColorSpace::HSL : AnimationColorSpace::RGB );
            }
            break;

            case ANA_ColorDirection:
            {
                Reference< XAnimateColor > xAnimateColor( mxNode, UNO_QUERY );
                if( xAnimateColor.is() )
                    xAnimateColor->setDirection( IsXMLToken( rValue, XML_CLOCKWISE ) );
            }
            break;

            case ANA_TransformType:
            {
                Reference< XAnimateTransform > xTransform( mxNode, UNO_QUERY );
                if( xTransform.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_TransformType) ) )
                    {
                        xTransform->setTransformType( (sal_Int16)nEnum );
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

            case ANA_TransitionType:
            {
                if( xTransitionFilter.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_TransitionType) ) )
                        xTransitionFilter->setTransition( (sal_Int16)nEnum );
                }
            }
            break;

            case ANA_TransitionSubType:
            {
                if( xTransitionFilter.is() )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_TransitionSubType) ) )
                        xTransitionFilter->setSubtype( (sal_Int16)nEnum );
                }
            }
            break;

            case ANA_Mode:
            {
                if( xTransitionFilter.is() )
                    xTransitionFilter->setMode( IsXMLToken( rValue, XML_IN ) );
            }
            break;

            case ANA_Direction:
            {
                if( xTransitionFilter.is() )
                    xTransitionFilter->setDirection( IsXMLToken( rValue, XML_FORWARD ) );
            }
            break;

            case ANA_FadeColor:
            {
                if( xTransitionFilter.is() )
                {
                    sal_Int32 nColor(0);
                    ::sax::Converter::convertColor(nColor, rValue);
                    xTransitionFilter->setFadeColor(nColor);
                }
            }
            break;

            case ANA_IterateType:
            {
                if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_IterateType) ) )
                {
                    if( xIter.is() )
                        xIter->setIterateType( (sal_Int16)nEnum );
                }
            }
            break;

            case ANA_IterateInterval:
            {
                if( xIter.is() )
                {
                    double fInterval = 0.0;
                    if( rValue.match("P") )
                    {
                        ::com::sun::star::util::Duration aDuration;
                        if (::sax::Converter::convertDuration(aDuration, rValue))
                        {
                            fInterval = ((((aDuration.Hours * 60)
                                + aDuration.Minutes) * 60) + aDuration.Seconds)
                              + (aDuration.NanoSeconds / 1000000000.0);
                        }
                    }
                    else
                    {
                        fInterval = rValue.toDouble();
                    }

                    xIter->setIterateInterval( fInterval );
                }
            }
            break;

            case ANA_Group_Id:
            {
                aUserData.push_back( NamedValue( aLocalName, makeAny( rValue.toInt32() ) ) );
            }
            break;

            case ANA_Command:
            {
                if( xCommand.is() && nNodeType == AnimationNodeType::COMMAND )
                {
                    if( SvXMLUnitConverter::convertEnum( nEnum, rValue, getAnimationsEnumMap(Animations_EnumMap_Command) ) )
                    {
                        xCommand->setCommand( (sal_Int16)nEnum );
                    }
                }
            }
            break;

            default:
                // push all unknown attributes within the presentation namespace as user data
                if( nPrefix == XML_NAMESPACE_PRESENTATION )
                {
                    aUserData.push_back( NamedValue( aLocalName, makeAny( rValue ) ) );
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
            std::list< NamedValue >::iterator aIter( aUserData.begin() );
            const std::list< NamedValue >::iterator aEnd( aUserData.end() );
            while( aIter != aEnd )
                *pData++ = (*aIter++);

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
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationNodeContext::StartElement(), RuntimeException caught!" );
    }
}

SvXMLImportContext * AnimationNodeContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( mxNode.is())
        return new AnimationNodeContext( mxNode, GetImport(), nPrefix, rLocalName, xAttrList, mpHelper );
    else
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

class AnimationsImport: public SvXMLImport, public XAnimationNodeSupplier
{
public:
    AnimationsImport( const Reference< XComponentContext > & rxContext );
    ~AnimationsImport() throw ();

    SvXMLImportContext* CreateContext(sal_uInt16 nPrefix, const OUString& rLocalName,   const Reference<XAttributeList>& xAttrList);

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type& aType ) throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XAnimationNodeSupplier
    Reference< XAnimationNode > SAL_CALL getAnimationNode() throw (RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

private:
    Reference< XAnimationNode > mxRootNode;
};

AnimationsImport::AnimationsImport( const Reference< XComponentContext > & rxContext )
: SvXMLImport( rxContext , true )
{
    // add namespaces
    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);

    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_SMIL),
        GetXMLToken(XML_N_SMIL),
        XML_NAMESPACE_SMIL);

    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_ANIMATION),
        GetXMLToken(XML_N_ANIMATION),
        XML_NAMESPACE_ANIMATION);

    mxRootNode = Reference<XAnimationNode>( SequenceTimeContainer::create(rxContext), UNO_QUERY_THROW );
}

AnimationsImport::~AnimationsImport() throw ()
{
}

// XInterface
Any SAL_CALL AnimationsImport::queryInterface( const Type& aType ) throw (RuntimeException)
{
    if ( aType == ::getCppuType((Reference<XAnimationNodeSupplier> *)0) )
    {
        return makeAny( Reference<XAnimationNodeSupplier>( this ) );
    }
    else
    {
        return SvXMLImport::queryInterface( aType );
    }
}

void SAL_CALL AnimationsImport::acquire() throw ()
{
    SvXMLImport::acquire();
}

void SAL_CALL AnimationsImport::release() throw ()
{
    SvXMLImport::release();
}

SvXMLImportContext *AnimationsImport::CreateContext(sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList)
{
    SvXMLImportContext* pContext = 0;

    if( (XML_NAMESPACE_ANIMATION == nPrefix) && IsXMLToken( rLocalName, XML_SEQ ) )
    {
         pContext = new AnimationNodeContext( mxRootNode, *this, nPrefix, rLocalName, xAttrList );
    }
    else
    {
        pContext = SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }

    return pContext;
}

// XAnimationNodeSupplier
Reference< XAnimationNode > SAL_CALL AnimationsImport::getAnimationNode() throw (RuntimeException)
{
    return mxRootNode;
}

void AnimationNodeContext::postProcessRootNode( SvXMLImport& /*rImport*/, const Reference< XAnimationNode >& xRootNode, Reference< XPropertySet >& xPageProps )
{
    if( xRootNode.is() && xPageProps.is() ) try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
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
                    Reference< XEnumeration > xChildEnumeration( xChildEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
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
                                xPageProps->setPropertyValue("Sound", Any(sal_True) );
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
                                    xPageProps->setPropertyValue("LoopSound", Any( sal_True ) );
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
        OSL_FAIL("xmloff::AnimationsImport::postProcessRootNode(), exception caught!");
    }
}

} // namespace xmloff

Sequence< OUString > SAL_CALL AnimationsImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( "com.sun.star.comp.Xmloff.AnimationsImport" );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL AnimationsImport_getImplementationName() throw()
{
    return OUString( "xmloff::AnimationsImport" );
}

Reference< XInterface > SAL_CALL AnimationsImport_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception )
{
    return (cppu::OWeakObject*)new xmloff::AnimationsImport( comphelper::getComponentContext(rSMgr) );
}

namespace xmloff
{

OUString SAL_CALL AnimationsImport::getImplementationName() throw(RuntimeException)
{
    return AnimationsImport_getImplementationName();
}

sal_Bool SAL_CALL AnimationsImport::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
    return ServiceName == "com.sun.star.comp.Xmloff.AnimationsImport";
}

Sequence< OUString > SAL_CALL AnimationsImport::getSupportedServiceNames() throw(RuntimeException)
{
    return AnimationsImport_getSupportedServiceNames();
}

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
