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

#ifndef _SD_PPT_EXANIMATIONS_HXX
#define _SD_PPT_EXANIMATIONS_HXX

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

#include <boost/shared_ptr.hpp>

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

    typedef boost::shared_ptr< AfterEffectNode > AfterEffectNodePtr;

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

    void writeZString( SvStream& rStrm, const rtl::OUString& rVal );
    sal_Bool getColorAny( const ::com::sun::star::uno::Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC ) const;
    sal_Bool exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const ::com::sun::star::uno::Any& rAny, const TranslateMode eTranslateMode );
    void exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const rtl::OUString& rVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal, const TranslateMode eTranslateMode );

    /** if available exportAnimPropertySet
       @return the ::com::sun::star::presentation::EffectNodeType*/
    sal_Int16 exportAnimPropertySet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimNode( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* pParent, const sal_Int32 nGroupLevel, const sal_Int16 nFillDefault );
    void exportAnimate( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateTarget( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_uInt32 nForceAttributeName = 0, int nAfterEffectType = AFTEREFFECT_NONE );
    void exportAnimateSet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >&  xNode, int nAfterEffectType );
    void exportAnimAction( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimEvent( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Int32 nFlags = 0 );
    void exportNode( SvStream& rStrm, ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* xParent,
                            const sal_uInt16 nContainerRecType, const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const sal_Bool bTakeBackInteractiveSequenceTiming,
                                const sal_Int16 nFillDefault );
    void exportAnimateTargetElement( SvStream& rStrm, const ::com::sun::star::uno::Any aAny, const sal_Bool bCreate2b01Atom );
    void exportAnimateKeyPoints( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimate >& xAnimate );
    void exportAnimValue( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Bool bExportAlways );
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

    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > createAfterEffectNodeClone( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode ) const;

    std::list< AfterEffectNodePtr > maAfterEffectNodes;

public:
    AnimationExporter( const EscherSolverContainer& rSolverContainer, ppt::ExSoundCollection& rExSoundCollection );

    void doexport( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, SvStream& rStrm );

    sal_Int32 mnCurrentGroup;

        // helper methods also used in ooxml export
    static ::com::sun::star::uno::Any convertAnimateValue( const ::com::sun::star::uno::Any& rSource, const rtl::OUString& rAttributeName );
        static sal_Bool GetNodeType( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, sal_Int16& nType );
        static sal_Int16 GetFillMode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Int16 nFillDefault );
        static void GetUserData( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rUserData, const ::com::sun::star::uno::Any ** pAny, sal_Size nLen );
        static sal_uInt32 TranslatePresetSubType( const sal_uInt32 nPresetClass, const sal_uInt32 nPresetId, const rtl::OUString& rPresetSubType );
        static sal_uInt32 GetPresetID( const rtl::OUString& rPreset, sal_uInt32 nAPIPresetClass, sal_Bool& bPresetId );
        static sal_uInt32 GetValueTypeForAttributeName( const rtl::OUString& rAttributeName );

    static const sal_Char* FindTransitionName( const sal_Int16 nType, const sal_Int16 nSubType, const sal_Bool bDirection );
    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTargetElementShape( const ::com::sun::star::uno::Any& rAny, sal_Int32& rBegin, sal_Int32& rEnd, sal_Bool& rParagraphTarget );
};
} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
