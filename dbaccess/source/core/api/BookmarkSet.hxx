/*************************************************************************
 *
 *  $RCSfile: BookmarkSet.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBACCESS_CORE_API_BOOKMARKSET_HXX
#define DBACCESS_CORE_API_BOOKMARKSET_HXX

#ifndef DBACCESS_CORE_API_CACHESET_HXX
#include "CacheSet.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif

namespace dbaccess
{
    // this set is used when we have a bookmarkable set from the driver
    class OBookmarkSet : public OCacheSet
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate> m_xRowLocate;

        void updateColumn(sal_Int32 nPos,::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate > _xParameter,const ORowSetValue& _rValue);
    public:
        OBookmarkSet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet,
                    const connectivity::OSQLParseTreeIterator*  _pIterator)
            : OCacheSet(_xDriverSet,_pIterator)
            ,m_xRowLocate(_xDriverSet,::com::sun::star::uno::UNO_QUERY)
        {}

        virtual void fillValueRow(ORowSetRow& _rRow);
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark( const ORowSetRow& _rRow ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->getBookmark();
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->moveToBookmark(bookmark);
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->moveRelativeToBookmark(bookmark,rows);
        }
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->compareBookmarks(first,second);
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->hasOrderedBookmarks();
        }
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_xRowLocate->hashBookmark(bookmark);
        }
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XDeleteRows> xDeleteRow(m_xRowLocate,::com::sun::star::uno::UNO_QUERY);
            if(xDeleteRow.is())
            {
                return xDeleteRow->deleteRows(rows);
            }
            return ::com::sun::star::uno::Sequence< sal_Int32 >();
        }
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_BOOKMARKSET_HXX
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/09/18 14:52:45  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:20:52  oj
    rowset addons

    Revision 1.0 01.08.2000 08:55:50  oj
------------------------------------------------------------------------*/

