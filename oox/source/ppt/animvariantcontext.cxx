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

#include "animvariantcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "drawingml/colorchoicecontext.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

    bool convertMeasure( OUString& rString )
    {
        bool bRet = false;

        /* here we want to substitute all occurrences of
         * [#]ppt_[xyhw] with
         * x,y,height and width respectively
         */
        sal_Int32 nIndex = 0;
        sal_Int32 nLastIndex = 0;

        nIndex = rString.indexOf("ppt_");
        // bail out early if there is no substitution to be made
        if(nIndex >= 0)
        {
            OUStringBuffer sRes(rString.getLength());

            do
            {
                // copy the non matching interval verbatim
                if(nIndex > nLastIndex)
                {
                    sRes.append(rString.getStr() + nLastIndex, (nIndex - nLastIndex));
                }
                // we are searching for ppt_[xywh] so we need and extra char behind the match
                if(nIndex + 4 < rString.getLength())
                {
                    switch(rString[nIndex + 4])
                    {
                    case (sal_Unicode)'h': // we found ppt_h
                        // if it was #ppt_h we already copied the #
                        // which we do not want in the target, so remove it
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes.remove(sRes.getLength() - 1, 1);
                        }
                        sRes.append("height");
                        bRet = true;
                        break;
                    case (sal_Unicode)'w':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes.remove(sRes.getLength() - 1, 1);
                        }
                        sRes.append("width");
                        bRet = true;
                        break;
                    case (sal_Unicode)'x':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes[sRes.getLength() - 1] = (sal_Unicode)'x';
                        }
                        else
                        {
                            sRes.append('x');
                        }
                        bRet = true;
                        break;
                    case (sal_Unicode)'y':
                        if(nIndex && (rString[nIndex - 1] == (sal_Unicode)'#'))
                        {
                            sRes[sRes.getLength() - 1] = (sal_Unicode)'y';
                        }
                        else
                        {
                            sRes.append('y');
                        }
                        bRet = true;
                        break;
                    default:
                        // this was ppt_ without an interesting thing after that
                        // just copy it verbatim
                        sRes.append("ppt_");
                        // we are going to adjust for ppt_@ after the switch
                        // so compensate for the fact we did not really process
                        // an extra character after ppt_
                        nIndex -= 1;
                        break;
                    }
                }
                else
                {
                    sRes.append("ppt_");
                    nIndex += 4;
                    nLastIndex = nIndex;
                    break;
                }
                nIndex += 5;
                nLastIndex = nIndex;
            }
            while((nIndex = rString.indexOf("ppt_", nIndex)) > 0);
            // copy the non matching tail if any
            if(nLastIndex < rString.getLength())
            {
                sRes.append(rString.getStr() + nLastIndex, rString.getLength() - nLastIndex );
            }
            rString = sRes.makeStringAndClear();
        }
        return bRet;
    }

    AnimVariantContext::AnimVariantContext( FragmentHandler2& rParent, sal_Int32 aElement, Any & aValue )
        : FragmentHandler2( rParent )
            , mnElement( aElement )
            , maValue( aValue )
    {
    }

    AnimVariantContext::~AnimVariantContext( ) throw( )
    {
    }

    void AnimVariantContext::onEndElement()
    {
        if( isCurrentElement( mnElement ) && maColor.isUsed() )
        {
            maValue = makeAny( maColor.getColor( getFilter().getGraphicHelper() ) );
        }
    }

    ContextHandlerRef AnimVariantContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( boolVal ):
        {
            bool val = rAttribs.getBool( XML_val, false );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( clrVal ):
            return new ::oox::drawingml::ColorContext( *this, maColor );
            // we'll defer setting the Any until the end.
        case PPT_TOKEN( fltVal ):
        {
            double val = rAttribs.getDouble( XML_val, 0.0 );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( intVal ):
        {
            sal_Int32 val = rAttribs.getInteger( XML_val, 0 );
            maValue = makeAny( val );
            return this;
        }
        case PPT_TOKEN( strVal ):
        {
            OUString val = rAttribs.getString( XML_val, OUString() );
            convertMeasure( val ); // ignore success or failure if it fails, use as is
            maValue = makeAny( val );
            return this;
        }
        default:
            break;
        }

        return this;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
