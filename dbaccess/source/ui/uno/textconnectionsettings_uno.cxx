/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconnectionsettings_uno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:25:55 $
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
#include "precompiled_dbaccess.hxx"

#include "textconnectionsettings.hxx"
#include "dbu_reghelper.hxx"
#include "moduledbu.hxx"
#include "apitools.hxx"
#include "unoadmin.hxx"
#include "dbustrings.hrc"
#include "propertystorage.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <svtools/genericunodialog.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;
    /** === end UNO using === **/
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //====================================================================
    //= OTextConnectionSettingsDialog
    //====================================================================

    class OTextConnectionSettingsDialog;
    typedef ODatabaseAdministrationDialog                                               OTextConnectionSettingsDialog_BASE;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTextConnectionSettingsDialog >    OTextConnectionSettingsDialog_PBASE;

    class OTextConnectionSettingsDialog
            :public OTextConnectionSettingsDialog_BASE
            ,public OTextConnectionSettingsDialog_PBASE
    {
        OModuleClient   m_aModuleClient;
        PropertyValues  m_aPropertyValues;

    protected:
        OTextConnectionSettingsDialog( const ::comphelper::ComponentContext& _rContext );
        virtual ~OTextConnectionSettingsDialog();

    public:
        DECLARE_IMPLEMENTATION_ID( );
        DECLARE_SERVICE_INFO_STATIC( );
        DECLARE_PROPERTYCONTAINER_DEFAULTS( );

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw(Exception);
        virtual sal_Bool SAL_CALL convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) throw(IllegalArgumentException);
        virtual void SAL_CALL getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const;

    protected:
        // OGenericUnoDialog overridables
        virtual Dialog* createDialog( Window* _pParent );
        virtual void implInitialize( const com::sun::star::uno::Any& _rValue );
    };

    //====================================================================
    //= OTextConnectionSettingsDialog
    //====================================================================
    //--------------------------------------------------------------------
    OTextConnectionSettingsDialog::OTextConnectionSettingsDialog( const ::comphelper::ComponentContext& _rContext )
        :OTextConnectionSettingsDialog_BASE( _rContext.getLegacyServiceFactory() )
    {
        TextConnectionSettingsDialog::bindItemStorages( *m_pDatasourceItems, m_aPropertyValues );
    }

    //---------------------------------------------------------------------
    OTextConnectionSettingsDialog::~OTextConnectionSettingsDialog()
    {
    }

    //---------------------------------------------------------------------
    IMPLEMENT_IMPLEMENTATION_ID( OTextConnectionSettingsDialog )

    //---------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC( OTextConnectionSettingsDialog, "com.sun.star.comp.dbaccess.OTextConnectionSettingsDialog", "com.sun.star.sdb.TextConnectionSettings" )

    //---------------------------------------------------------------------
    Reference< XPropertySetInfo >  SAL_CALL OTextConnectionSettingsDialog::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OTextConnectionSettingsDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OTextConnectionSettingsDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );

        // in addition to the properties registered by the base class, we have
        // more properties which are not even handled by the PropertyContainer implementation,
        // but whose values are stored in our item set
        sal_Int32 nProp = aProps.getLength();
        aProps.realloc( nProp + 6 );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HeaderLine" ) ),
            PROPERTY_ID_HEADER_LINE,
            ::cppu::UnoType< sal_Bool >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FieldDelimiter" ) ),
            PROPERTY_ID_FIELD_DELIMITER,
            ::cppu::UnoType< ::rtl::OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StringDelimiter" ) ),
            PROPERTY_ID_STRING_DELIMITER,
            ::cppu::UnoType< ::rtl::OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DecimalDelimiter" ) ),
            PROPERTY_ID_DECIMAL_DELIMITER,
            ::cppu::UnoType< ::rtl::OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ThousandDelimiter" ) ),
            PROPERTY_ID_THOUSAND_DELIMITER,
            ::cppu::UnoType< ::rtl::OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        aProps[ nProp++ ] = Property(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CharSet" ) ),
            PROPERTY_ID_ENCODING,
            ::cppu::UnoType< ::rtl::OUString >::get(),
            PropertyAttribute::TRANSIENT
        );

        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //---------------------------------------------------------------------
    Dialog* OTextConnectionSettingsDialog::createDialog(Window* _pParent)
    {
        return new TextConnectionSettingsDialog( _pParent, *m_pDatasourceItems );
    }

    //---------------------------------------------------------------------
    void OTextConnectionSettingsDialog::implInitialize(const Any& _rValue)
    {
        OTextConnectionSettingsDialog_BASE::implInitialize( _rValue );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw(Exception)
    {
        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            pos->second->setPropertyValue( _rValue );
        }
        else
        {
            OTextConnectionSettingsDialog::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OTextConnectionSettingsDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue) throw(IllegalArgumentException)
    {
        sal_Bool bModified = sal_False;

        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            // we're lazy here ...
            _rConvertedValue = _rValue;
            pos->second->getPropertyValue( _rOldValue );
            bModified = sal_True;
        }
        else
        {
            bModified = OTextConnectionSettingsDialog::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        }

        return bModified;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OTextConnectionSettingsDialog::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        PropertyValues::const_iterator pos = m_aPropertyValues.find( _nHandle );
        if ( pos != m_aPropertyValues.end() )
        {
            pos->second->getPropertyValue( _rValue );
        }
        else
        {
            OTextConnectionSettingsDialog::getFastPropertyValue( _rValue, _nHandle );
        }
    }

//........................................................................
} // namespace dbaui
//........................................................................

extern "C" void SAL_CALL createRegistryInfo_OTextConnectionSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OTextConnectionSettingsDialog > aAutoRegistration;
}

