/*************************************************************************
 *
 *  $RCSfile: indexes.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-19 06:02:34 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            : sOriginalName(_rOriginalName), sName(_rOriginalName), bPrimaryKey(sal_False), bUnique(sal_False), bModified(sal_False)
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 19.03.01 06:35:23  fs
 ************************************************************************/

