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

#include <tools/urlobj.hxx>
#include <tools/string.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <ucbhelper/content.hxx>
#include <vos/ref.hxx>
#include <vos/refernce.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <bf_svtools/pathoptions.hxx>
#include "comphelper/processfactory.hxx"
#include <vector>
#include <list>
#include <functional>
#include <algorithm>

//.........................................................................
namespace binfilter
{
//.........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::uno;

    //=====================================================================
    //= helpers
    //=====================================================================
    //---------------------------------------------------------------------
    SvStream& operator << ( SvStream& _rStorage, const util::DateTime& _rDate )
    {
        _rStorage << _rDate.HundredthSeconds;
        _rStorage << _rDate.Seconds;
        _rStorage << _rDate.Minutes;
        _rStorage << _rDate.Hours;
        _rStorage << _rDate.Day;
        _rStorage << _rDate.Month;
        _rStorage << _rDate.Year;

        return _rStorage;
    }

    //---------------------------------------------------------------------
    SvStream& operator >> ( SvStream& _rStorage, util::DateTime& _rDate )
    {
        _rStorage >> _rDate.HundredthSeconds;
        _rStorage >> _rDate.Seconds;
        _rStorage >> _rDate.Minutes;
        _rStorage >> _rDate.Hours;
        _rStorage >> _rDate.Day;
        _rStorage >> _rDate.Month;
        _rStorage >> _rDate.Year;

        return _rStorage;
    }

    //---------------------------------------------------------------------
    sal_Bool operator == ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return	_rLHS.HundredthSeconds == _rRHS.HundredthSeconds
            &&	_rLHS.Seconds	== _rRHS.Seconds
            &&	_rLHS.Minutes	== _rRHS.Minutes
            &&	_rLHS.Hours		== _rRHS.Hours
            &&	_rLHS.Day		== _rRHS.Day
            &&	_rLHS.Month		== _rRHS.Month
            &&	_rLHS.Year		== _rRHS.Year;
    }

    //---------------------------------------------------------------------
    sal_Bool operator != ( const util::DateTime& _rLHS, const util::DateTime& _rRHS )
    {
        return !( _rLHS == _rRHS );
    }

    //---------------------------------------------------------------------
    /// base class for functors which act an an SvStream
    struct StorageHelper
    {
    protected:
        SvStream&	m_rStorage;
        StorageHelper( SvStream& _rStorage ) : m_rStorage( _rStorage ) { }
    };

    //---------------------------------------------------------------------
    /// functor which allows storing a string
    struct StoreString
            :public ::std::unary_function< String, void >
            ,public StorageHelper
    {
        StoreString( SvStream& _rStorage ) : StorageHelper( _rStorage ) { }

        void operator() ( const String& _rString ) const
        {
            m_rStorage.WriteByteString( _rString );
        }
    };
//.........................................................................
}	// namespace sfx2
//.........................................................................

