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

#ifndef _SD_PPT_INANIMATIONS_HXX
#define _SD_PPT_INANIMATIONS_HXX

#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include "pptanimations.hxx"
#include <animations.hxx>

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif
#include <filter/msfilter/svdfppt.hxx>

#include <list>

class DffRecordHeader;
class SdPage;
class SvStream;
class ImplSdPPTImport;

namespace ppt
{
class PropertySet;
class Atom;

class AnimationImporter
{
public:
    AnimationImporter( ImplSdPPTImport* pPPTImport, SvStream& rStCtrl );

    void import( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, const DffRecordHeader& rProgTagContentHd );

private:
    void importAnimationContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xParent );
    void importTimeContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimationNodeContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    void importAnimateSetContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateFilterContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateScaleContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateColorContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateRotationContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateMotionContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importCommandContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAudioContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    void importAnimationEvents( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimationValues( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimationActions( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateAttributeTargetContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    void importAnimateKeyPoints( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importPropertySetContainer( const Atom* pAtom,PropertySet& rSet );
    bool importAttributeValue( const Atom* pAtom, com::sun::star::uno::Any& rAny );
    bool importAttributeNamesContainer( const Atom* pAtom, rtl::OUString& rAttributeNames );
    sal_Int32 importTargetElementContainer( const Atom* pAtom, ::com::sun::star::uno::Any& rTarget, sal_Int16& nSubType );

    void fillNode( ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xTiming, const AnimationNode& rNode, const PropertySet& rSet );
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > createNode( const Atom* pAtom, const AnimationNode& rNode );

    bool convertAnimationNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xParent );
    bool convertAnimationValue( MS_AttributeNames eAttribute, com::sun::star::uno::Any& rValue );

    void fixMainSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void fixInteractiveSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    void processAfterEffectNodes();

    ::com::sun::star::uno::Any  implGetColorAny( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC );
    sal_Int16                   implGetColorSpace( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxRootNode;

    ImplSdPPTImport* mpPPTImport;
    SvStream&   mrStCtrl;

    sd::AfterEffectNodeList maAfterEffectNodes;

#ifdef DBG_ANIM_LOG
    FILE * mpFile;
    void dump_anim_group( const Atom* pAtom, const AnimationNode& rNode, const PropertySet& rSet, bool bOpen );
    void dump( const rtl::OUString& rString );
    void dump( sal_uInt32 nLen, bool bNewLine = true );
#endif

    void dump_atom_header( const Atom* pAtom, bool bOpen, bool bAppend );
    void dump_atom( const Atom* pAtom, bool bNewLine = true );
    void dump_target( ::com::sun::star::uno::Any& rAny );
    void dump( ::com::sun::star::uno::Any& rAny );
    void dump( const PropertySet& rSet );
    void dump( const AnimationNode& rNode );
    void dump( const char * pText );
    void dump( const char * pText, sal_Int32 nInt );
    void dump( const char * pText, double fDouble );
    void dump( const char * pText, const char * pText2 );
    void dump( const char * pText, const rtl::OUString& rString );
};

} // namespace ppt

#endif
