/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsenode_s.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:26:45 $
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

#ifndef CONNECTIVITY_PARSENODE_SIMPLE_HXX
#define CONNECTIVITY_PARSENODE_SIMPLE_HXX

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef CONNECTIVITY_DBTOOLS_REFBASE_HXX
#include "refbase.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    class OSQLParseNode;
    //================================================================
    //= OSimpleParseNode
    //================================================================
    class OSimpleParseNode
            :public simple::ISQLParseNode
            ,public ORefBase
    {
    protected:
        const OSQLParseNode*    m_pFullNode;
        sal_Bool                m_bOwner;

    public:
        OSimpleParseNode(const OSQLParseNode* _pNode, sal_Bool _bTakeOwnership = sal_True);
        ~OSimpleParseNode();

        // ISQLParseNode
        virtual void parseNodeToStr(::rtl::OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const IParseContext* _pContext
        ) const;

        virtual void parseNodeToPredicateStr(::rtl::OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
            const ::com::sun::star::lang::Locale& _rIntl,
            const sal_Char _cDecSeparator,
            const IParseContext* _pContext
        ) const;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_PARSENODE_SIMPLE_HXX


