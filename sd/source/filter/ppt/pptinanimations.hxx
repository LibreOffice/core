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

#ifndef INCLUDED_SD_SOURCE_FILTER_PPT_PPTINANIMATIONS_HXX
#define INCLUDED_SD_SOURCE_FILTER_PPT_PPTINANIMATIONS_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <vector>
#include <animations.hxx>

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif

namespace com { namespace sun { namespace star { namespace animations { class XAnimationNode; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace ppt { struct AnimationNode; }

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

    int  import( const css::uno::Reference< css::drawing::XDrawPage >& xPage, const DffRecordHeader& rProgTagContentHd );

private:
    int  importAnimationContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xParent );
    int  importTimeContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    int  importAnimationNodeContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    void importAnimateSetContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateFilterContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateScaleContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateColorContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateRotationContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateMotionContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importCommandContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    int  importAudioContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    void importAnimationEvents( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimationValues( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimationActions( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importAnimateAttributeTargetContainer( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    void importAnimateKeyPoints( const Atom* pAtom, const css::uno::Reference< css::animations::XAnimationNode >& xNode );
    void importPropertySetContainer( const Atom* pAtom,PropertySet& rSet );
    bool importAttributeValue( const Atom* pAtom, css::uno::Any& rAny );
    void importAttributeNamesContainer( const Atom* pAtom, OUString& rAttributeNames );
    void importTargetElementContainer( const Atom* pAtom, css::uno::Any& rTarget, sal_Int16& nSubType );

    static void fillNode( css::uno::Reference< css::animations::XAnimationNode > const & xTiming, const AnimationNode& rNode, const PropertySet& rSet );
    static css::uno::Reference< css::animations::XAnimationNode > createNode( const Atom* pAtom, const AnimationNode& rNode );

    bool convertAnimationNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode, const css::uno::Reference< css::animations::XAnimationNode >& xParent );
    css::uno::Any  implGetColorAny( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC );
    static sal_Int16            implGetColorSpace( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC );

private:
    css::uno::Reference< css::animations::XAnimationNode > mxRootNode;

    ImplSdPPTImport* mpPPTImport;
    SvStream&   mrStCtrl;

    std::vector< sd::AfterEffectNode > maAfterEffectNodes;

#ifdef DBG_ANIM_LOG
    FILE * mpFile;
    void dump_anim_group( const Atom* pAtom, const AnimationNode& rNode, const PropertySet& rSet, bool bOpen );
    void dump( const OUString& rString );
    void dump( sal_uInt32 nLen, bool bNewLine = true );
#endif

    static void dump_atom_header( const Atom* pAtom, bool bOpen, bool bAppend );
    static void dump_atom( const Atom* pAtom, bool bNewLine = true );
    static void dump_target( css::uno::Any& rAny );
    static void dump( css::uno::Any& rAny );
    static void dump( const PropertySet& rSet );
    static void dump( const AnimationNode& rNode );
    static void dump( const char * pText );
    static void dump( const char * pText, sal_Int32 nInt );
    void dump( const char * pText, sal_Int64 nInt );
    static void dump( const char * pText, double fDouble );
    static void dump( const char * pText, const char * pText2 );
    static void dump( const char * pText, const OUString& rString );
};

} // namespace ppt

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
