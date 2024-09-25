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

#pragma once

#include <o3tl/any.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <tools/helpers.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>

#include <vector>

/* Declaration and definition of AnimationNode helper */

namespace anim
{
    // TODO(Q1): this could possibly be implemented with a somewhat
    // more lightweight template, by having the actual worker receive
    // only a function pointer, and a thin templated wrapper around
    // that which converts member functions into that.

    /** pushes the given node to the given vector and recursively calls itself for each child node.
    */
    inline void create_deep_vector( const css::uno::Reference< css::animations::XAnimationNode >& xNode,
                                std::vector< css::uno::Reference< css::animations::XAnimationNode > >& rVector )
    {
        rVector.push_back( xNode );

        try
        {
            // get an XEnumerationAccess to the children
            css::uno::Reference< css::container::XEnumerationAccess >
                  xEnumerationAccess( xNode, css::uno::UNO_QUERY );

            if( xEnumerationAccess.is() )
            {
                css::uno::Reference< css::container::XEnumeration >
                      xEnumeration = xEnumerationAccess->createEnumeration();

                if( xEnumeration.is() )
                {
                    while( xEnumeration->hasMoreElements() )
                    {
                        css::uno::Reference< css::animations::XAnimationNode >
                            xChildNode( xEnumeration->nextElement(),
                                        css::uno::UNO_QUERY_THROW );

                        create_deep_vector( xChildNode, rVector );
                    }
                }
            }
        }
        catch( css::uno::Exception& )
        {
        }
    }

    inline bool getVisibilityPropertyForAny(css::uno::Any const& rAny)
    {
        bool bVisible = false;
        css::uno::Any aAny(rAny);

        // try to extract bool value
        if (!(aAny >>= bVisible))
        {
            // try to extract string
            OUString aString;
            if (aAny >>= aString)
            {
                // we also take the strings "true" and "false",
                // as well as "on" and "off" here
                if (aString.equalsIgnoreAsciiCase("true") ||
                    aString.equalsIgnoreAsciiCase("on"))
                {
                    bVisible = true;
                }
                if (aString.equalsIgnoreAsciiCase("false") ||
                    aString.equalsIgnoreAsciiCase("off"))
                {
                    bVisible = false;
                }
            }
        }
        return bVisible;
    }

    inline bool getVisibilityProperty(
        const css::uno::Reference< css::animations::XAnimate >& xAnimateNode, bool& bReturn)
    {
        if (xAnimateNode->getAttributeName().equalsIgnoreAsciiCase("visibility"))
        {
            css::uno::Any aAny(xAnimateNode->getTo());
            bReturn = getVisibilityPropertyForAny(aAny);
            return true;
        }

        return false;
    }

    inline css::uno::Reference<css::uno::XInterface> getParagraphTarget(
        const css::presentation::ParagraphTarget& pTarget)
    {
        try
        {
            css::uno::Reference<css::container::XEnumerationAccess> xParaEnumAccess(
                pTarget.Shape, css::uno::UNO_QUERY_THROW);

            css::uno::Reference<css::container::XEnumeration> xEnumeration(
                                                xParaEnumAccess->createEnumeration(),
                                                css::uno::UNO_SET_THROW);
            sal_Int32 nParagraph = pTarget.Paragraph;

            while (xEnumeration->hasMoreElements())
            {
                css::uno::Reference<css::uno::XInterface> xRef(
                    xEnumeration->nextElement(), css::uno::UNO_QUERY);
                if (nParagraph-- == 0)
                    return xRef;
            }
        }
        catch (const css::uno::RuntimeException&)
        {
            SAL_WARN("animations", "getParagraphTarget");
        }

        css::uno::Reference<css::uno::XInterface> xRef;
        return xRef;
    }

    inline void convertTarget(OStringBuffer& sTmp, const css::uno::Any& rTarget)
    {
        if (!rTarget.hasValue())
            return;

        css::uno::Reference<css::uno::XInterface> xRef;
        if (!(rTarget >>= xRef))
        {
            if (auto pt = o3tl::tryAccess<css::presentation::ParagraphTarget>(rTarget))
            {
                xRef = getParagraphTarget(*pt);
            }
        }

        SAL_WARN_IF(!xRef.is(), "animations", "convertTarget(), invalid target type!");
        if (xRef.is())
        {
            const std::string aIdentifier(GetInterfaceHash(xRef));
            if (!aIdentifier.empty())
                sTmp.append(aIdentifier);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
