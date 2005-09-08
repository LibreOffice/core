/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsenode_s.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:47:09 $
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
#ifndef CONNECTIVITY_PARSENODE_SIMPLE_HXX
#include "parsenode_s.hxx"
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleParseNode
    //================================================================
    //----------------------------------------------------------------
    OSimpleParseNode::OSimpleParseNode(const OSQLParseNode* _pNode, sal_Bool _bTakeOwnership)
        :m_pFullNode(_pNode)
        ,m_bOwner(_bTakeOwnership)
    {
        OSL_ENSURE(m_pFullNode, "OSimpleParseNode::OSimpleParseNode: invalid node given!");
    }

    //----------------------------------------------------------------
    OSimpleParseNode::~OSimpleParseNode()
    {
        if (m_bOwner)
            delete m_pFullNode;
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToStr(::rtl::OUString& _rString, const Reference< XDatabaseMetaData >& _rxMeta,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToStr(_rString, _rxMeta,_pContext);
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToPredicateStr(::rtl::OUString& _rString, const Reference< XDatabaseMetaData >& _rxMeta,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField,
        const Locale& _rIntl, const sal_Char _cDecSeparator,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToPredicateStr(_rString, _rxMeta, _rxFormatter, _rxField, _rIntl, _cDecSeparator,_pContext);
    }

//........................................................................
}   // namespace connectivity
//........................................................................


