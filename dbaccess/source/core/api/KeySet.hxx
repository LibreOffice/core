/*************************************************************************
 *
 *  $RCSfile: KeySet.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-01-22 07:38:23 $
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

#ifndef DBACCESS_CORE_API_KEYSET_HXX
#define DBACCESS_CORE_API_KEYSET_HXX

#ifndef DBACCESS_CORE_API_CACHESET_HXX
#include "CacheSet.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _MAP_
#include <map>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

namespace dbaccess
{
    typedef ::std::map<sal_Int32,::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XUnoTunnel> > OBookmarkMap;
    class OKeySet;
    class OKeySetBookmark : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XUnoTunnel>
    {
        OKeySet*    m_pKeySet;  // to parent
        OBookmarkMap::iterator m_aPosIter;      //

    public:
        OKeySetBookmark(OKeySet* _pKeySet,const OBookmarkMap::iterator& _aPosIter)
            : m_pKeySet(_pKeySet)
            , m_aPosIter(_aPosIter)
        {
        }
        ~OKeySetBookmark();

        void removeKeySet();

        sal_Int32 getRealPos() const;
        OBookmarkMap::iterator getPosIterator() { return m_aPosIter; }
        void setPosIterator(const OBookmarkMap::iterator& _rIter);

        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    };

    // is used when the source supports keys
    class OKeySet : public OCacheSet
    {
        OBookmarkMap m_aBookmarkMap;
        ::std::vector<sal_Int32> m_aDeletedRows;
    public:
        OKeySet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDriverSet);
        ~OKeySet();

        void RemoveBookmarkPos(OBookmarkMap::iterator& _rIter);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_bUpdated;
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_bInserted;
        }
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
        {
            return m_bDeleted;
        }
        // ::com::sun::star::sdbcx::XRowLocate
        virtual ::com::sun::star::uno::Any SAL_CALL getBookmark( const ORowSetRow& _rRow ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL compareBookmarks( const ::com::sun::star::uno::Any& first, const ::com::sun::star::uno::Any& second ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        virtual sal_Int32 SAL_CALL hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // -------------------------------------------------------------------------
        // ::com::sun::star::sdbcx::XDeleteRows
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL deleteRows( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rows ,const connectivity::OSQLTable& _xTable) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::sdbc::XResultSetUpdate
        virtual void SAL_CALL updateRow(const ORowSetRow& _rInsertRow,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL deleteRow(const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable   ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_KEYSET_HXX
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2000/10/17 10:18:12  oj
    some changes for the rowset

    Revision 1.2  2000/10/11 11:18:10  fs
    replace unotools with comphelper

    Revision 1.1.1.1  2000/09/19 00:15:38  hr
    initial import

    Revision 1.2  2000/09/18 14:52:46  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:19:40  oj
    rowset addons

    Revision 1.0 01.08.2000 09:07:41  oj
------------------------------------------------------------------------*/

