/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVX_DBCHARSETHELPER_HXX
#define SVX_DBCHARSETHELPER_HXX
#include "svx/svxdllapi.h"
#include "svx/dbtoolsclient.hxx"

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= ODataAccessCharsetHelper
    //====================================================================
    class SVX_DLLPUBLIC ODataAccessCharsetHelper : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessCharSet >  m_xCharsetHelper;

    protected:
        virtual bool ensureLoaded() const;

    public:
        ODataAccessCharsetHelper( );

        inline sal_Int32    getSupportedTextEncodings( ::std::vector< rtl_TextEncoding >& _rEncs ) const
        {
            if ( ensureLoaded() )
                return m_xCharsetHelper->getSupportedTextEncodings( _rEncs );
            return 0;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_DBCHARSETCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
