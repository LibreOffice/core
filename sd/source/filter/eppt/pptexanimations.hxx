/*************************************************************************
 *
 *  $RCSfile: pptexanimations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:50:33 $
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

#ifndef _SD_PPT_EXANIMATIONS_HXX
#define _SD_PPT_EXANIMATIONS_HXX

#ifndef _COM_SUN_STAR_ANIMATIONS_XTIMECONTAINER_HPP_
#include <com/sun/star/animations/XTimeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATE_HPP_
#include <com/sun/star/animations/XAnimate.hpp>
#endif
#ifndef _SD_PPTANIMATIONS_HXX
#include "../ppt/pptanimations.hxx"
#endif
#ifndef _SVX_ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif

#ifdef DBG_ANIM_LOG
#include <stdio.h>
#endif

#include <list>

class DffRecordHeader;
class SdPage;
class SvStream;

namespace ppt
{

typedef sal_uInt32 TranslateMode;
#define TRANSLATE_NONE              0
#define TRANSLATE_VALUE             1
#define TRANSLATE_ATTRIBUTE         2
#define TRANSLATE_MEASURE           4
#define TRANSLATE_NUMBER_TO_STRING  8

struct ExContainer
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    ExContainer( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0 );
    ~ExContainer();
};
struct ExAtom
{
    sal_uInt32  nContPos;
    SvStream&   rStrm;

    ExAtom( SvStream& rSt, const sal_uInt16 nRecType, const sal_uInt16 nInstance = 0, const sal_uInt8 nVersion = 0 );
    ~ExAtom();
};

class AnimationExporter
{
    ::com::sun::star::uno::Any aTarget;

    void writeZString( SvStream& rStrm, const rtl::OUString& rVal );
    sal_Bool getColorAny( const ::com::sun::star::uno::Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC ) const;
    sal_Bool exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const ::com::sun::star::uno::Any& rAny, const TranslateMode eTranslateMode );
    ::com::sun::star::uno::Any convertAnimateValue( const ::com::sun::star::uno::Any& rSource, const rtl::OUString& rAttributeName ) const;
    void exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const rtl::OUString& rVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal, const TranslateMode eTranslateMode );
    void exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal, const TranslateMode eTranslateMode );

    // if available exportAnimPropertySet returns the ::com::sun::star::presentation::EffectNodeType
    sal_Int16 exportAnimPropertySet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimNode( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* pParent, const sal_Int32 nGroupLevel, const sal_Int16 nFillDefault );
    void exportAnimate( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateTarget( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_uInt32 nForceAttributeName = 0 );
    void exportAnimateSet( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >&  xNode );
    void exportAnimAction( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimEvent( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Int32 nFlags = 0 );
    void exportNode( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >* xParent,
                            const sal_uInt16 nContainerRecType, const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const sal_Bool bTakeBackInteractiveSequenceTiming,
                                const sal_Int16 nFillDefault );
    void exportAnimateTargetElement( SvStream& rStrm, const ::com::sun::star::uno::Any aAny, const sal_Bool bCreate2b01Atom );
    void exportAnimateKeyPoints( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimate >& xAnimate );
    void exportAnimValue( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const sal_Bool bExportAlways );
    void exportTransitionFilter( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateMotion( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateTransform( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportAnimateColor( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    void exportIterate( SvStream& rStrm, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    const EscherSolverContainer& mrSolverContainer;

public:
    AnimationExporter( const EscherSolverContainer& rSolverContainer );

    void doexport( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage, SvStream& rStrm );
};

}; // namespace ppt

#endif
