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

#include <oox/ppt/animationspersist.hxx>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/animations/Event.hpp>

#include <oox/drawingml/shape.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/addtosequence.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::text;

namespace oox
{

Any addToSequence( const Any& rOldValue, const Any& rNewValue )
{
    if( !rNewValue.hasValue() )
    {
        return rOldValue;
    }
    else if( !rOldValue.hasValue() )
    {
        return rNewValue;
    }
    else
    {
        Sequence< Any > aNewSeq;
        if( rOldValue >>= aNewSeq )
        {
            sal_Int32 nSize = aNewSeq.getLength();
            aNewSeq.realloc(nSize+1);
            aNewSeq[nSize] = rNewValue;
        }
        else
        {
            aNewSeq.realloc(2);
            aNewSeq[0] = rOldValue;
            aNewSeq[1] = rNewValue;
        }
        return makeAny( aNewSeq );
    }
}

} // namespace oox

namespace oox { namespace ppt {

    void ShapeTargetElement::convert( css::uno::Any & rTarget, sal_Int16 & rSubType ) const
    {
        switch(mnType)
        {
        case XML_subSp:
            rSubType = ShapeAnimationSubType::AS_WHOLE;
            break;
        case XML_bg:
            rSubType = ShapeAnimationSubType::ONLY_BACKGROUND;
            break;
        case XML_txEl:
        {
            ParagraphTarget aParaTarget;
            Reference< XShape > xShape;
            rTarget >>= xShape;
            aParaTarget.Shape = xShape;
            rSubType = ShapeAnimationSubType::ONLY_TEXT;

            Reference< XText > xText( xShape, UNO_QUERY );
            if( xText.is() )
            {
                switch(mnRangeType)
                {
                case XML_charRg:
                    // TODO calculate the corresponding paragraph for the text range....
                    SAL_INFO("oox.ppt", "OOX: TODO calculate the corresponding paragraph for the text range..." );
                    break;
                case XML_pRg:
                    aParaTarget.Paragraph = static_cast< sal_Int16 >( maRange.start );
                    // TODO what to do with more than one.
                    SAL_INFO("oox.ppt", "OOX: TODO what to do with more than one" );
                    break;
                }
                rTarget <<= aParaTarget;
            }
            break;
        }
        default:
            break;
        }
    }

    Any AnimTargetElement::convert(const SlidePersistPtr & pSlide, sal_Int16 & nSubType) const
    {
        Any aTarget;
        // see sd/source/files/ppt/pptinanimations.cxx:3191 (in importTargetElementContainer())
        switch(mnType)
        {
        case XML_inkTgt:
            // TODO
            SAL_INFO("oox.ppt", "OOX: TODO inkTgt" );
            break;
        case XML_sldTgt:
            // TODO
            SAL_INFO("oox.ppt", "OOX: TODO sldTgt" );
            break;
        case XML_sndTgt:
            aTarget <<= msValue;
            break;
        case XML_spTgt:
        {
            OUString sShapeName = msValue;

            // bnc#705982 - catch referenced diagram fallback shapes
            if( maShapeTarget.mnType == XML_dgm )
                sShapeName = maShapeTarget.msSubShapeId;

            Any rTarget;
            ::oox::drawingml::ShapePtr pShape = pSlide->getShape( sShapeName );
            SAL_WARN_IF( !pShape, "oox.ppt", "failed to locate Shape" );

            if( !pShape && maShapeTarget.mnType == XML_dgm )
            {
                pShape = pSlide->getShape( msValue );
            }

            if( pShape )
            {
                Reference< XShape > xShape( pShape->getXShape() );
                SAL_WARN_IF( !xShape.is(), "oox.ppt", "fail to get XShape from shape" );
                if( xShape.is() )
                {
                    rTarget <<= xShape;
                    maShapeTarget.convert(rTarget, nSubType);
                    aTarget = rTarget;
                }
            }
            break;
        }
        default:
            break;
        }
        return aTarget;
    }

    // Convert a time node condition to XAnimation.Begin or XAnimation.End
    Any AnimationCondition::convert(const SlidePersistPtr & pSlide) const
    {
        Any aAny;
        Event aEvent;
        if(mpTarget && (maValue >>= aEvent))
        {
            sal_Int16 nSubType;
            aAny = mpTarget->convert( pSlide, nSubType );
            aEvent.Source = aAny;
            aAny <<= aEvent;
        }
        else
        {
            aAny = maValue;
        }
        return aAny;
    }

    Any AnimationCondition::convertList(const SlidePersistPtr & pSlide, const AnimationConditionList & l)
    {
        Any aAny;

        if (l.size() == 1)
            return l[0].convert(pSlide);

        for (auto const& elem : l)
        {
            aAny = addToSequence( aAny, elem.convert(pSlide) );
        }
        return aAny;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
