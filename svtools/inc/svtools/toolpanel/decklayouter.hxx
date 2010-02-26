/*************************************************************************
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

#ifndef DECKLAYOUTER_HXX
#define DECKLAYOUTER_HXX

#include <boost/shared_ptr.hpp>

class Rectangle;

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= IDeckLayouter
    //====================================================================
    class IDeckLayouter
    {
    public:
        /** re-arranges the elements of the tool deck, taking into account the
            available space for the complete deck.

            @param i_rDeckPlayground
                the playground for the complete tool panel deck
            @return
                the content area for the single tool panels
        */
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground ) = 0;

        virtual ~IDeckLayouter()
        {
        }
    };

    typedef ::boost::shared_ptr< IDeckLayouter >    PDeckLayouter;

//........................................................................
} // namespace svt
//........................................................................

#endif // DECKLAYOUTER_HXX
