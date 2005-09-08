/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parser_s.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:47:45 $
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

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef CONNECTIVITY_DBTOOLS_PARSER_SIMPLE_HXX
#include "parser_s.hxx"
#endif
#ifndef CONNECTIVITY_PARSENODE_SIMPLE_HXX
#include "parsenode_s.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleSQLParser
    //================================================================
    //----------------------------------------------------------------
    OSimpleSQLParser::OSimpleSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext)
        :m_aFullParser(_rxServiceFactory,_pContext)
    {
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    const IParseContext& OSimpleSQLParser::getContext() const
    {
        return m_aFullParser.getContext();
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::ISQLParseNode > OSimpleSQLParser::predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField) const
    {
        OSimpleParseNode* pReturn = NULL;
        OSQLParseNode* pFullNode = const_cast<OSimpleSQLParser*>(this)->m_aFullParser.predicateTree(rErrorMessage, rStatement, _rxFormatter, _rxField);
        if (pFullNode)
            pReturn = new OSimpleParseNode(pFullNode, sal_True);
        return pReturn;
    }

//........................................................................
}   // namespace connectivity
//........................................................................


