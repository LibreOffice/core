/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FRM_RESOURCE_HXX_
#define _FRM_RESOURCE_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
class SimpleResMgr;
namespace binfilter {

//.........................................................................
namespace frm
{

#define FRM_RES_STRING(id) ResourceManager::loadString(id)

    //==================================================================
    //= ResourceManager
    //= handling ressources within the FormLayer library
    //==================================================================
    class ResourceManager
    {
        static SimpleResMgr*	m_pImpl;

    private:
        // no instantiation allowed
        ResourceManager() { }
        ~ResourceManager() { }

        // we'll instantiate one static member of the following class, which, in it's dtor,
        // ensures that m_pImpl will be deleted
        class EnsureDelete
        {
        public:
            EnsureDelete() { }
            ~EnsureDelete();
        };
        friend class EnsureDelete;

    protected:
        static void ensureImplExists();

    public:
        /** loads the string with the specified resource id from the FormLayer resource file
        */
        static ::rtl::OUString loadString(sal_uInt16 _nResId);
    };

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_RESOURCE_HXX_

