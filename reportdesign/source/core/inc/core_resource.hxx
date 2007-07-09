#ifndef REPORTDESIGN_CORE_RESOURCE_HXX_
#define REPORTDESIGN_CORE_RESOURCE_HXX_
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: core_resource.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:15 $
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


#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif

class SimpleResMgr;
//.........................................................................
namespace reportdesign
{

#define RPT_RESSTRING(id,_rM) ResourceManager::loadString(id,_rM)

    //==================================================================
    //= ResourceManager
    //= handling ressources within the DBA-Core library
    //==================================================================
    class ResourceManager
    {
        static SimpleResMgr*    m_pImpl;

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
        static void ensureImplExists(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >& _rM);

    public:
        /** loads the string with the specified resource id from the FormLayer resource file
        */
        static ::rtl::OUString loadString(sal_uInt16 _nResId,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory >& _rM);
    };

//.........................................................................
}
//.........................................................................

#endif // REPORTDESIGN_CORE_RESOURCE_HXX_

