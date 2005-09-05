/*************************************************************************
 *
 *  $RCSfile: RowSetCacheIterator.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-05 08:58:21 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBACCESS_ROWSETCACHEITERATOR_HXX
#define DBACCESS_ROWSETCACHEITERATOR_HXX

#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#include "RowSetRow.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

namespace dbaccess
{
    typedef struct
    {
        ORowSetMatrix::iterator     aIterator;
        ::com::sun::star::uno::Any  aBookmark;
    } ORowSetCacheIterator_Helper;

    DECLARE_STL_STDKEY_MAP(sal_Int32,ORowSetCacheIterator_Helper,ORowSetCacheMap);

    class ORowSetCache;
    class ORowSetCacheIterator
    {
        friend class ORowSetCache;
        ORowSetCacheMap::iterator   m_aIter;
        ORowSetCache*               m_pCache;
    protected:
        ORowSetCacheIterator(const ORowSetCacheMap::iterator& _rIter,ORowSetCache* _pCache)
            : m_aIter(_rIter)
            ,m_pCache(_pCache)
        {
        }
    public:
        ORowSetCacheIterator() :m_pCache(NULL),m_aIter(){}
        ORowSetCacheIterator(const ORowSetCacheIterator& _rRH);
        ORowSetCacheIterator& operator =(const ORowSetCacheIterator&);

        sal_Bool isNull() const;
        ORowSetCacheIterator& operator =(const ORowSetMatrix::iterator&);
        operator ORowSetMatrix::iterator();

        ORowSetRow& operator *();
        const ORowSetRow& operator *() const;

        ORowSetMatrix::iterator& operator ->();
        const ORowSetMatrix::iterator& operator ->() const;

        bool operator <=(const ORowSetMatrix::iterator& _rRH) const;
        bool operator !=(const ORowSetMatrix::iterator& _rRH) const;
        bool operator ==(const ORowSetMatrix::iterator& _rRH) const;

        void setBookmark(const ::com::sun::star::uno::Any&  _rBookmark);
        ::com::sun::star::uno::Any getBookmark() const { return m_aIter->second.aBookmark; }
    };
}
#endif // DBACCESS_ROWSETCACHEITERATOR_HXX


