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

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "../ppt/pptanimations.hxx"
#include <pptexsoundcollection.hxx>
#include <filter/msfilter/escherex.hxx>

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif

#include <memory>

#include <list>

class SvStream;

namespace ppt
{

    struct AfterEffectNode
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxNode;
        ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxMaster;

        AfterEffectNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xMaster )
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
    ::com::sun::star::uno::Any aTarget;

    static void writeZString( SvStream& rStrm, const OUString& rVal );
    static bool getColorAny( const ::com::sun::star::uno::Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC );
    static bool exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const ::com::sun::star::uno::Any& rAny, const TranslateMode eTranslateMode );
    static void exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const OUString& rVal, const TranslateMode eTranslateMode );
    static void exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal, const TranslateMode eTranslateMode );
    static void exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal, const TranslateMode eTranslateMode );
    static void exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal, const TranslateMode eTranslateMode );

    /** if available exportAnimPropertySet
       @return the ::com::sun::star::presentation::EffectNodeType*/
    static sal_Int16 exportAnimPropertySet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    static void exportAnimNode( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* pParent, const sal_Int32 nGroupLevel, const sal_Int16 nFillDefault );
    void exportAnimate( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateTarget( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_uInt32 nForceAttributeName = 0, int nAfterEffectType = AFTEREFFECT_NONE );
    void exportAnimateSet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >&  xNode, int nAfterEffectType );
    static void exportAnimAction( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimEvent( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Int32 nFlags = 0 );
    void exportNode( SvStream& rStrm, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* xParent,
                            const sal_uInt16 nContainerRecType, const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const bool bTakeBackInteractiveSequenceTiming,
                                const sal_Int16 nFillDefault );
    void exportAnimateTargetElement( SvStream& rStrm, const ::com::sun::star::uno::Any& rAny, const bool bCreate2b01Atom );
    static void exportAnimateKeyPoints( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimate >& xAnimate );
    static void exportAnimValue( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const bool bExportAlways );
    void exportTransitionFilter( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateMotion( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateTransform( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateColor( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, int nAfterEffectType );
    void exportIterate( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    const EscherSolverContainer& mrSolverContainer;
    ppt::ExSoundCollection& mrExSoundCollection;
    void processAfterEffectNodes( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    bool isAfterEffectNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) const;
    bool hasAfterEffectNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAfterEffectNode ) const;
    bool isEmptyNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) const;

    static ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > createAfterEffectNodeClone( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    std::list< AfterEffectNodePtr > maAfterEffectNodes;

public:
    AnimationExporter( const EscherSolverContainer& rSolverContainer, ppt::ExSoundCollection& rExSoundCollection );

    void doexport( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, SvStream& rStrm );

    sal_Int32 mnCurrentGroup;

        // helper methods also used in ooxml export
    static ::com::sun::star::uno::Any convertAnimateValue( const ::com::sun::star::uno::Any& rSource, const OUString& rAttributeName );
        static bool GetNodeType( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, sal_Int16& nType );
        static sal_Int16 GetFillMode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Int16 nFillDefault );
        static void GetUserData( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rUserData, const ::com::sun::star::uno::Any ** pAny, sal_Size nLen );
        static sal_uInt32 TranslatePresetSubType( const sal_uInt32 nPresetClass, const sal_uInt32 nPresetId, const OUString& rPresetSubType );
        static sal_uInt32 GetPresetID( const OUString& rPreset, sal_uInt32 nAPIPresetClass, bool& bPresetId );
        static sal_uInt32 GetValueTypeForAttributeName( const OUString& rAttributeName );

    static const sal_Char* FindTransitionName( const sal_Int16 nType, const sal_Int16 nSubType, const bool bDirection );
    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTargetElementShape( const ::com::sun::star::uno::Any& rAny, sal_Int32& rBegin, sal_Int32& rEnd, bool& rParagraphTarget );
};
} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
