/*************************************************************************
 *
 *  $RCSfile: cellvaluebinding.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:50:58 $
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

#ifndef SC_CELLVALUEBINDING_HXX
#define SC_CELLVALUEBINDING_HXX

#ifndef _DRAFTS_COM_SUN_STAR_FORM_XVALUEBINDING_HPP_
#include <drafts/com/sun/star/form/XValueBinding.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELL_HPP_
#include <com/sun/star/table/XCell.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif


//.........................................................................
namespace calc
{
//.........................................................................

    //=====================================================================
    //= OCellValueBinding
    //=====================================================================
    class OCellValueBinding;
    // the base for our interfaces
    typedef ::cppu::WeakAggComponentImplHelper5 <   ::drafts::com::sun::star::form::XValueBinding
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::util::XModifyBroadcaster
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCellValueBinding_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellValueBinding_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellValueBinding >
                                                    OCellValueBinding_PABase;

    class OCellValueBinding :public ::comphelper::OBaseMutex
                            ,public OCellValueBinding_Base      // order matters! before OCellValueBinding_PBase, so rBHelper gets initialized
                            ,public OCellValueBinding_PBase
                            ,public OCellValueBinding_PABase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                    m_xCell;                /// the cell we're bound to, for double value access
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                    m_xCellText;            /// the cell we're bound to, for text access
        ::cppu::OInterfaceContainerHelper
                    m_aModifyListeners;     /// our modify listeners
        sal_Bool    m_bInitialized;         /// has XInitialization::initialize been called?
        sal_Bool    m_bListPos;             /// constructed as ListPositionCellBinding?

    public:
        OCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& _rxDocument,
            sal_Bool _bListPos
        );

    protected:
        ~OCellValueBinding( );

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XValueBinding
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getSupportedValueTypes(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsType( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getValue( const ::com::sun::star::uno::Type& aType ) throw (::drafts::com::sun::star::form::IncompatibleTypesException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& aValue ) throw (::drafts::com::sun::star::form::IncompatibleTypesException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // OComponentHelper/XComponent
        virtual void SAL_CALL disposing();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;

        // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        void    checkDisposed( ) const
                    SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );
        void    checkValueType( const ::com::sun::star::uno::Type& _rType ) const
                    SAL_THROW( ( ::drafts::com::sun::star::form::IncompatibleTypesException ) );
        void    checkInitialized()
                    SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

        /** notifies our modify listeners
            @precond
                our mutex is <em>not</em> locked
        */
        void    notifyModified();

        void    setBooleanFormat();

    private:
        OCellValueBinding();                                        // never implemented
        OCellValueBinding( const OCellValueBinding& );              // never implemented
        OCellValueBinding& operator=( const OCellValueBinding& );   // never implemented

#ifdef DBG_UTIL
    private:
        static  const char* checkConsistency_static( const void* _pThis );
                const char* checkConsistency( ) const;
#endif
    };

//.........................................................................
}   // namespace calc
//.........................................................................

#endif // SC_CELLVALUEBINDING_HXX
