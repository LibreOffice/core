/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xfm_addcondition.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:14:52 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SOURCE_FORM_XFM_ADDCONDITION_HXX
#include "xfm_addcondition.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _SVX_DATANAVI_HXX
#include "datanavi.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

//........................................................................
namespace svxform
{
//........................................................................

#define PROPERTY_ID_BINDING             5724
#define PROPERTY_ID_FORM_MODEL          5725
#define PROPERTY_ID_FACET_NAME          5726
#define PROPERTY_ID_CONDITION_VALUE     5727

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddConditionDialog_Create( const Reference< XMultiServiceFactory > & _rxORB )
    {
        return OAddConditionDialog::Create( _rxORB );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OAddConditionDialog_GetSupportedServiceNames()
    {
        ::comphelper::StringSequence aSupported( 1 );
        aSupported.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xforms.ui.dialogs.AddCondition" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddConditionDialog_GetImplementationName()
    {
        return ::rtl::OUString::createFromAscii( "org.openoffice.comp.svx.OAddConditionDialog" );
    }

    //====================================================================
    //= OAddConditionDialog
    //====================================================================
    //--------------------------------------------------------------------
    OAddConditionDialog::OAddConditionDialog( const Reference< XMultiServiceFactory >& _rxORB )
        :OAddConditionDialogBase( _rxORB )
    {
        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Binding" ) ),
            PROPERTY_ID_BINDING,
            PropertyAttribute::TRANSIENT,
            &m_xBinding,
            ::getCppuType( &m_xBinding )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FacetName" ) ),
            PROPERTY_ID_FACET_NAME,
            PropertyAttribute::TRANSIENT,
            &m_sFacetName,
            ::getCppuType( &m_sFacetName )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConditionValue" ) ),
            PROPERTY_ID_CONDITION_VALUE,
            PropertyAttribute::TRANSIENT,
            &m_sConditionValue,
            ::getCppuType( &m_sConditionValue )
        );

        registerProperty(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormModel" ) ),
            PROPERTY_ID_FORM_MODEL,
            PropertyAttribute::TRANSIENT,
            &m_xWorkModel,
            ::getCppuType( &m_xWorkModel )
        );
    }

    //-------------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OAddConditionDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //-------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddConditionDialog::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new OAddConditionDialog( _rxFactory ) );
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddConditionDialog::getImplementationName() throw(RuntimeException)
    {
        return OAddConditionDialog_GetImplementationName();
    }

    //-------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OAddConditionDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return OAddConditionDialog_GetSupportedServiceNames();
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OAddConditionDialog::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OAddConditionDialog::getInfoHelper()
    {
        return *const_cast< OAddConditionDialog* >( this )->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OAddConditionDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProperties;
        describeProperties( aProperties );
        return new ::cppu::OPropertyArrayHelper( aProperties );
    }

    //------------------------------------------------------------------------------
    Dialog* OAddConditionDialog::createDialog(Window* _pParent)
    {
        if ( !m_xBinding.is() || !m_sFacetName.getLength() )
            throw RuntimeException( ::rtl::OUString(), *this );

        return new AddConditionDialog( _pParent, m_sFacetName, m_xBinding );
    }

    //------------------------------------------------------------------------------
    void OAddConditionDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        OAddConditionDialogBase::executedDialog( _nExecutionResult );
        if ( _nExecutionResult == RET_OK )
            m_sConditionValue = static_cast< AddConditionDialog* >( m_pDialog )->GetCondition();
    }

//........................................................................
} // namespace svxformv
//........................................................................

