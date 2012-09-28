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

    int  import( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, const DffRecordHeader& rProgTagContentHd );

private:
    int  importAnimationContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xParent );
    int  importTimeContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    int  importAnimationNodeContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    void importAnimateSetContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateFilterContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateScaleContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateColorContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateRotationContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importAnimateMotionContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void importCommandContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    int  importAudioContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

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
    void dump( const char * pText, sal_Int64 nInt );
    void dump( const char * pText, double fDouble );
    void dump( const char * pText, const char * pText2 );
    void dump( const char * pText, const rtl::OUString& rString );
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
