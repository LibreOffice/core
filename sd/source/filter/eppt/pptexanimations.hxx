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

#ifndef INCLUDED_SD_SOURCE_FILTER_EPPT_PPTEXANIMATIONS_HXX
#define INCLUDED_SD_SOURCE_FILTER_EPPT_PPTEXANIMATIONS_HXX

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>

#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace animations { class XAnimate; } } } }
namespace com { namespace sun { namespace star { namespace animations { class XAnimationNode; } } } }
namespace com { namespace sun { namespace star { namespace beans { struct NamedValue; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace ppt { class ExSoundCollection; }

class SvStream;
class EscherSolverContainer;

namespace ppt
{

    struct AfterEffectNode
    {
        css::uno::Reference< css::animations::XAnimationNode > mxNode;
        css::uno::Reference< css::animations::XAnimationNode > mxMaster;

        AfterEffectNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode,
                         const css::uno::Reference< css::animations::XAnimationNode >& xMaster )
                         : mxNode( xNode ), mxMaster( xMaster ) {}
    };

    typedef std::shared_ptr< AfterEffectNode > AfterEffectNodePtr;

typedef sal_uInt32 TranslateMode;
#define TRANSLATE_NONE              0
#define TRANSLATE_VALUE             1
#define TRANSLATE_ATTRIBUTE         2
#define TRANSLATE_MEASURE           4
#define TRANSLATE_NUMBER_TO_STRING  8

const int AFTEREFFECT_NONE = 0;
const int AFTEREFFECT_COLOR = 1;
const int AFTEREFFECT_SET = 2;

class AnimationExporter
{
    css::uno::Any aTarget;
    const EscherSolverContainer& mrSolverContainer;
    ppt::ExSoundCollection& mrExSoundCollection;
    std::vector< AfterEffectNodePtr > maAfterEffectNodes;
    sal_Int32 mnCurrentGroup;

    static void writeZString( SvStream& rStrm, const OUString& rVal );
    static bool getColorAny( const css::uno::Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC );
    static bool exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const css::uno::Any& rAny, const TranslateMode eTranslateMode );
    static void exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const OUString& rVal, const TranslateMode eTranslateMode );
    static void exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal );
    static void exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal );
    static void exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal );

    /** if available exportAnimPropertySet
       @return the css::presentation::EffectNodeType*/
    static sal_Int16 exportAnimPropertySet( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    static void exportAnimNode( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode,
                        const sal_Int16 nFillDefault );
    void exportAnimate( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void exportAnimateTarget( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode, const sal_uInt32 nForceAttributeName = 0, int nAfterEffectType = AFTEREFFECT_NONE );
    void exportAnimateSet( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >&  xNode, int nAfterEffectType );
    static void exportAnimAction( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void exportAnimEvent( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode, const sal_Int32 nFlags = 0 );
    void exportNode( SvStream& rStrm, css::uno::Reference< css::animations::XAnimationNode > const & xNode,
                     const sal_uInt16 nContainerRecType, const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const bool bTakeBackInteractiveSequenceTiming,
                     const sal_Int16 nFillDefault );
    void exportAnimateTargetElement( SvStream& rStrm, const css::uno::Any& rAny, const bool bCreate2b01Atom );
    static void exportAnimateKeyPoints( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimate >& xAnimate );
    static void exportAnimValue( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode, const bool bExportAlways );
    void exportTransitionFilter( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void exportAnimateMotion( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void exportAnimateTransform( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void exportAnimateColor( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode, int nAfterEffectType );
    void exportIterate( SvStream& rStrm, const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    void processAfterEffectNodes( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    bool isAfterEffectNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode ) const;
    bool hasAfterEffectNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode, css::uno::Reference< css::animations::XAnimationNode >& xAfterEffectNode ) const;
    bool isEmptyNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode ) const;

    static css::uno::Reference< css::animations::XAnimationNode > createAfterEffectNodeClone( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

public:
    AnimationExporter( const EscherSolverContainer& rSolverContainer, ppt::ExSoundCollection& rExSoundCollection );

    void doexport( const css::uno::Reference< css::drawing::XDrawPage >& xPage, SvStream& rStrm );

        // helper methods also used in ooxml export
    static css::uno::Any convertAnimateValue( const css::uno::Any& rSource, const OUString& rAttributeName );
        static bool GetNodeType( const css::uno::Reference< css::animations::XAnimationNode >& xNode, sal_Int16& nType );
        static sal_Int16 GetFillMode( const css::uno::Reference< css::animations::XAnimationNode >& xNode, const sal_Int16 nFillDefault );
        static void GetUserData( const css::uno::Sequence< css::beans::NamedValue >& rUserData, const css::uno::Any ** pAny, std::size_t nLen );
        static sal_uInt32 TranslatePresetSubType( const sal_uInt32 nPresetClass, const sal_uInt32 nPresetId, const OUString& rPresetSubType );
        static sal_uInt32 GetPresetID( const OUString& rPreset, sal_uInt32 nAPIPresetClass, bool& bPresetId );
        static sal_uInt32 GetValueTypeForAttributeName( const OUString& rAttributeName );

    static const sal_Char* FindTransitionName( const sal_Int16 nType, const sal_Int16 nSubType, const bool bDirection );
    static css::uno::Reference< css::drawing::XShape > getTargetElementShape( const css::uno::Any& rAny, sal_Int32& rBegin, sal_Int32& rEnd, bool& rParagraphTarget );
};
} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
