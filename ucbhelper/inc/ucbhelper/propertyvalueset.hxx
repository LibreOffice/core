/*************************************************************************
 *
 *  $RCSfile: propertyvalueset.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#define _UCBHELPER_PROPERTYVALUESET_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCOLUMNLOCATE_HPP_
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    class XPropertySet;
} } } }

namespace ucb {

class PropertyValues;

//=========================================================================

/**
  * This class implements the interface XRow. After construction of a valueset
  * the user can append properties ( incl. its values ) to the set. This class
  * is useful when implementing the command "getPropertyValues", because the
  * values to return can easyly appended to a valueset object. That object can
  * directly be returned by the implementation of the command.
  */
class PropertyValueSet :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::sdbc::XRow,
                public com::sun::star::sdbc::XColumnLocate
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                     m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter >
                                     m_xTypeConverter;
    vos::OMutex     m_aMutex;
    PropertyValues* m_pValues;
    sal_Bool        m_bWasNull;
    sal_Bool        m_bTriedToGetTypeConverter;

private:
    const com::sun::star::uno::Reference<
            com::sun::star::script::XTypeConverter >&
    getTypeConverter();

public:
    PropertyValueSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );
    virtual ~PropertyValueSet();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull()
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const ::com::sun::star::uno::Reference<
                   ::com::sun::star::container::XNameAccess >& typeMap )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );

    // XColumnLocate
    virtual sal_Int32 SAL_CALL
    findColumn( const ::rtl::OUString& columnName )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns the number of elements of the value set.
      *
      * @return the number of elements of the value set.
      */
    sal_Int32 getLength() const;

    /**
      * This method appends a string to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendString   ( const ::com::sun::star::beans::Property& rProp,
                          const ::rtl::OUString& rValue );

    /**
      * This method appends a boolean to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBoolean  ( const ::com::sun::star::beans::Property& rProp,
                          sal_Bool bValue );

    /**
      * This method appends a byte to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendByte     ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int8 nValue );

    /**
      * This method appends a short to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendShort    ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int16 nValue );

    /**
      * This method appends an int to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendInt      ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int32 nValue );

    /**
      * This method appends a long to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendLong     ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int64 nValue );

    /**
      * This method appends a float to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendFloat    ( const ::com::sun::star::beans::Property& rProp,
                          float nValue );

    /**
      * This method appends a double to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendDouble   ( const ::com::sun::star::beans::Property& rProp,
                          double nValue );

    /**
      * This method appends a byte sequence to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBytes    ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Sequence<
                              sal_Int8 >& rValue );

    /**
      * This method appends a date to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendDate     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::Date& rValue );

    /**
      * This method appends a time to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendTime     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::Time& rValue );

    /**
      * This method appends a timestamp to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendTimestamp( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::DateTime& rValue );

    /**
      * This method appends a binary stream to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBinaryStream   ( const ::com::sun::star::beans::Property& rProp,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::io::XInputStream >&
                                        rValue );

    /**
      * This method appends a character stream to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendCharacterStream( const ::com::sun::star::beans::Property& rProp,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::io::XInputStream >&
                                        rValue );

    /**
      * This method appends an object ( any ) to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendObject   ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Any& rValue );

    /**
      * This method appends a ref to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendRef      ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XRef >& rValue );

    /**
      * This method appends a blob to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBlob     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XBlob >& rValue );

    /**
      * This method appends a clob to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendClob     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XClob >& rValue );

    /**
      * This method appends an array to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendArray    ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XArray >& rValue );

    /**
      * This method appends a void value ( a "hole" ) to the value set. This
      * is useful, since void values indicate errors, like non-existing
      * property (-values) etc.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendVoid     ( const ::com::sun::star::beans::Property& rProp );

    /**
      * This method tries to append all property values contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
      */
    void appendPropertySet( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& rSet );

    /** This method tries to append a single property value contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
       *    @param  rProperty is the property for that the value shall be obtained
      *         from the given property set.
       *    @return False, if the property value cannot be obtained from the
      *         given property pet. True, otherwise.
       */
    sal_Bool appendPropertySetValue(
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& rSet,
                        const ::com::sun::star::beans::Property& rProperty );
};

}

#endif /* !_UCBHELPER_PROPERTYVALUESET_HXX */
