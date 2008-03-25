/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resultsetforquery.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:23:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _RESULTSETFORQUERY_HXX
#define _RESULTSETFORQUERY_HXX

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif

#include "resultsetbase.hxx"
#include "urlparameter.hxx"

namespace chelp {

    class Databases;

    class ResultSetForQuery
        : public ResultSetBase
    {
    public:

        ResultSetForQuery( const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>&  xMSF,
                           const com::sun::star::uno::Reference<com::sun::star::ucb::XContentProvider>&  xProvider,
                           sal_Int32 nOpenMode,
                           const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq,
                           const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >& seqSort,
                           URLParameter& aURLParameter,
                           Databases* pDatabases );


    private:

        Databases*   m_pDatabases;
        URLParameter m_aURLParameter;
    };

}


#endif
