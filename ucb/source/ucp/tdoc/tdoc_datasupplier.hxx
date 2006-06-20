/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_datasupplier.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:31:56 $
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

#ifndef INCLUDED_TDOC_DATASUPPLIER_HXX
#define INCLUDED_TDOC_DATASUPPLIER_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif

#ifndef _UCBHELPER_RESULTSET_HXX
#include <ucbhelper/resultset.hxx>
#endif

namespace tdoc_ucp {

struct DataSupplier_Impl;
class Content;

class ResultSetDataSupplier : public ucb::ResultSetDataSupplier
{
    DataSupplier_Impl*  m_pImpl;

private:
    bool queryNamesOfChildren();
    ::rtl::OUString assembleChildURL( const ::rtl::OUString& aName );

public:
    ResultSetDataSupplier(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
        const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode = com::sun::star::ucb::OpenMode::ALL );
    virtual ~ResultSetDataSupplier();

    virtual rtl::OUString queryContentIdentifierString( sal_Int32 nIndex );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >
    queryContentIdentifier( sal_Int32 nIndex );
    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
    queryContent( sal_Int32 nIndex );

    virtual sal_Bool getResult( sal_Int32 nIndex );

    virtual sal_Int32 totalCount();
    virtual sal_Int32 currentCount();
    virtual sal_Bool   isCountFinal();

    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
    queryPropertyValues( sal_Int32 nIndex  );
    virtual void releasePropertyValues( sal_Int32 nIndex );

    virtual void close();

    virtual void validate()
        throw( com::sun::star::ucb::ResultSetException );
};

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_DATASUPPLIER_HXX */
