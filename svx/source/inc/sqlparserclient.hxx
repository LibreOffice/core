/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlparserclient.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:00:46 $
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

#ifndef SVX_SQLPARSERCLIENT_HXX
#define SVX_SQLPARSERCLIENT_HXX

#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif
#ifndef SVX_QUERYDESIGNCONTEXT_HXX
#include "ParseContext.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OSQLParserClient
    //====================================================================
    class OSQLParserClient : public ODbtoolsClient
                            ,public ::svxform::OParseContextClient
    {
    private:
        //add by BerryJia for fixing Bug97420 Time:2002-9-12-11:00(PRC time)
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;

    protected:
        mutable ::rtl::Reference< ::connectivity::simple::ISQLParser >  m_xParser;

    protected:
        OSQLParserClient(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        virtual bool ensureLoaded() const;

    protected:
        inline ::rtl::Reference< ::connectivity::simple::ISQLParseNode > predicateTree(
                ::rtl::OUString& _rErrorMessage,
                const ::rtl::OUString& _rStatement,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField
            ) const
        {
            ::rtl::Reference< ::connectivity::simple::ISQLParseNode > xReturn;
            if ( ensureLoaded() )
                xReturn = m_xParser->predicateTree(_rErrorMessage, _rStatement, _rxFormatter, _rxField);
            return xReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_SQLPARSERCLIENT_HXX


