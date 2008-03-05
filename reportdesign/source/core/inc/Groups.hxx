#ifndef REPORTDESIGN_API_GROUPS_HXX
#define REPORTDESIGN_API_GROUPS_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Groups.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:55:18 $
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

#ifndef _COM_SUN_STAR_REPORT_XGROUPS_HPP_
#include <com/sun/star/report/XGroups.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#include <list>


namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper1< com::sun::star::report::XGroups> GroupsBase;
    /** \class OGroups Defines the implementation of a \interface com:::sun::star::report::XGroups
     * \ingroup reportdesign_api
     *
     */
    class OGroups : public comphelper::OBaseMutex,
                    public GroupsBase
    {
        typedef ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup > > TGroups;
        ::cppu::OInterfaceContainerHelper                                                       m_aContainerListeners;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XReportDefinition >     m_xParent;
        TGroups                                                                                 m_aGroups;
    private:
        OGroups& operator=(const OGroups&);
        OGroups(const OGroups&);
        void checkIndex(sal_Int32 _nIndex);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual ~OGroups();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual void SAL_CALL disposing();
    public:
        OGroups( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xParent
                ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        void copyGroups(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups >& _xSource);

    // XGroups
        // Attributes
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > SAL_CALL getReportDefinition() throw (::com::sun::star::uno::RuntimeException);
        // Methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup > SAL_CALL createGroup(  ) throw (::com::sun::star::uno::RuntimeException);
    // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
    // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    // XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif // REPORTDESIGN_API_GROUPS_HXX

