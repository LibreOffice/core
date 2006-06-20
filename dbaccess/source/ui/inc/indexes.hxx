/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexes.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:17:40 $
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

#ifndef _DBAUI_INDEXES_HXX_
#define _DBAUI_INDEXES_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

//......................................................................
namespace dbaui
{
//......................................................................
    //==================================================================
    //= OIndexField
    //==================================================================
    struct OIndexField
    {
        String              sFieldName;
        sal_Bool            bSortAscending;

        OIndexField() : bSortAscending(sal_True) { }
    };

    DECLARE_STL_VECTOR( OIndexField, IndexFields );

    //==================================================================
    //= OIndex
    //==================================================================
    struct GrantIndexAccess
    {
        friend class OIndexCollection;
    private:
        GrantIndexAccess() { }
    };

    //..................................................................
    struct OIndex
    {
    protected:
        ::rtl::OUString     sOriginalName;
        sal_Bool            bModified;

    public:
        ::rtl::OUString     sName;
        ::rtl::OUString     sDescription;
        sal_Bool            bPrimaryKey;
        sal_Bool            bUnique;
        IndexFields         aFields;

    public:
        OIndex(const ::rtl::OUString& _rOriginalName)
            : sOriginalName(_rOriginalName), bModified(sal_False), sName(_rOriginalName), bPrimaryKey(sal_False), bUnique(sal_False)
        {
        }

        const ::rtl::OUString& getOriginalName() const { return sOriginalName; }

        sal_Bool    isModified() const { return bModified; }
        void        setModified(sal_Bool _bModified) { bModified = _bModified; }
        void        clearModified() { setModified(sal_False); }

        sal_Bool    isNew() const { return 0 == getOriginalName().getLength(); }
        void        flagAsNew(const GrantIndexAccess&) { sOriginalName = ::rtl::OUString(); }
        void        flagAsCommitted(const GrantIndexAccess&) { sOriginalName = sName; }


    private:
        OIndex();   // not implemented
    };

    DECLARE_STL_VECTOR( OIndex, Indexes );

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXES_HXX_

