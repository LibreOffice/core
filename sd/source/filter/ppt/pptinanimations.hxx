/*************************************************************************
 *
 *  $RCSfile: pptinanimations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:52:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_PPT_INANIMATIONS_HXX
#define _SD_PPT_INANIMATIONS_HXX

#ifndef _COM_SUN_STAR_ANIMATIONS_XTIMECONTAINER_HPP_
#include <com/sun/star/animations/XTimeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef _SD_PPTANIMATIONS_HXX
#include "pptanimations.hxx"
#endif

#ifndef _SD_ANIMATIONS_HXX_
#include <animations.hxx>
#endif

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif

#ifndef _SVDFPPT_HXX
#include <svx/svdfppt.hxx>
#endif

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
    void importAnimationSubContainer( const Atom* pAtom, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

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
#endif

    void dump_atom_header( const Atom* pAtom, bool bOpen, bool bAppend );
    void dump_atom( const Atom* pAtom, bool bNewLine = true );
    void dump( sal_uInt32 nLen, bool bNewLine = true );
    void dump_anim_group( const Atom* pAtom, const AnimationNode& rNode, const PropertySet& rSet, bool bOpen );
    void dump_target( ::com::sun::star::uno::Any& rAny );
    void dump( ::com::sun::star::uno::Any& rAny );
    void dump( const PropertySet& rSet );
    void dump( const AnimationNode& rNode );
    void dump( const char * pText );
    void dump( const rtl::OUString& rString );
    void dump( const char * pText, sal_Int32 nInt );
    void dump( const char * pText, double fDouble );
    void dump( const char * pText, const char * pText2 );
    void dump( const char * pText, const rtl::OUString& rString );
};

}; // namespace ppt

#endif
