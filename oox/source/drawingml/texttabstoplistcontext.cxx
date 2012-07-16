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

#include <list>
#include <algorithm>

#include <rtl/ustring.hxx>

#include "oox/drawingml/drawingmltypes.hxx"
#include "texttabstoplistcontext.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

        TextTabStopListContext::TextTabStopListContext( ContextHandler& rParent, std::list< TabStop >  & aTabList )
            : ContextHandler( rParent )
            , maTabList( aTabList )
        {
        }

        TextTabStopListContext::~TextTabStopListContext()
        {
        }

        void SAL_CALL TextTabStopListContext::endFastElement( ::sal_Int32 /*Element*/ )
            throw ( SAXException, RuntimeException)
        {
        }


    Reference< ::XFastContextHandler > TextTabStopListContext::createFastChildContext( ::sal_Int32 aElement,
                                                                                                                                                                             const Reference< XFastAttributeList >& xAttribs )
            throw (SAXException, RuntimeException)
        {
            Reference< XFastContextHandler > xRet;
            switch( aElement )
            {
            case A_TOKEN( tab ):
            {
                OUString sValue;
                TabStop aTabStop;
                sValue = xAttribs->getOptionalValue( XML_pos );
                if( !sValue.isEmpty() )
                {
                    aTabStop.Position = GetCoordinate( sValue );
                }
                sal_Int32 aToken = xAttribs->getOptionalValueToken( XML_algn, 0 );
                if( aToken != 0 )
                {
                    aTabStop.Alignment = GetTabAlign( aToken );
                }
                maTabList.push_back(aTabStop);
                break;
            }
            default:
                break;
            }
            if ( !xRet.is() )
                xRet.set( this );
            return xRet;
        }


} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
