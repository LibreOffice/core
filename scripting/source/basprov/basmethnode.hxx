/*************************************************************************
 *
 *  $RCSfile: basmethnode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-10-29 15:00:45 $
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

#ifndef SCRIPTING_BASMETHNODE_HXX
#define SCRIPTING_BASMETHNODE_HXX

#ifndef _DRAFTS_COM_SUN_STAR_SCRIPT_BROWSE_XBROWSENODE_HPP_
#include <drafts/com/sun/star/script/browse/XBrowseNode.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <hash_map>

class SbMethod;


//.........................................................................
namespace basprov
{
//.........................................................................

    //  ----------------------------------------------------
    //  class BasicMethodNodeImpl
    //  ----------------------------------------------------

    typedef ::cppu::WeakImplHelper2<
        ::drafts::com::sun::star::script::browse::XBrowseNode,
        com::sun::star::beans::XPropertySet > BasicMethodNodeImpl_BASE;

    typedef ::std::hash_map < ::rtl::OUString, com::sun::star::uno::Any, ::rtl::OUStringHash,
        ::std::equal_to< ::rtl::OUString > > Props_hash;

    class BasicMethodNodeImpl : public BasicMethodNodeImpl_BASE
    {
    private:
        SbMethod* m_pMethod;
        Props_hash m_hProps;
    bool m_bIsAppScript;
    public:
        BasicMethodNodeImpl( SbMethod* pMethod, bool isAppScript = true );
        virtual ~BasicMethodNodeImpl();

        // XBrowseNode
        virtual ::rtl::OUString SAL_CALL getName(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getType(  )
            throw (::com::sun::star::uno::RuntimeException);
    // XPropertySet implementation
    virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo( )
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName,
        const com::sun::star::uno::Any& aValue )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::beans::PropertyVetoException,
            com::sun::star::lang::IllegalArgumentException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener(
        const ::rtl::OUString& aPropertyName,
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener(
        const ::rtl::OUString& PropertyName,
        const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw ( com::sun::star::beans::UnknownPropertyException,
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException );

    };

//.........................................................................
}   // namespace basprov
//.........................................................................

#endif // SCRIPTING_BASMETHNODE_HXX
