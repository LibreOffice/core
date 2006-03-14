/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pcrcommon.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:27:40 $
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

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#define _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_

#define EDITOR_LIST_APPEND              (sal_uInt16)-1
#define EDITOR_LIST_REPLACE_EXISTING    (sal_uInt16)-1

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
/** === end UNO includes === **/

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    #define OWN_PROPERTY_ID_INTROSPECTEDOBJECT      0x0010
    #define OWN_PROPERTY_ID_CURRENTPAGE             0x0011
    #define OWN_PROPERTY_ID_CONTROLCONTEXT          0x0012
    #define OWN_PROPERTY_ID_TABBINGMODEL            0x0013

    //========================================================================
    //= types
    //========================================================================
    typedef ::comphelper::OSimpleListenerContainer  <   ::com::sun::star::beans::XPropertyChangeListener
                                                    ,   ::com::sun::star::beans::PropertyChangeEvent
                                                    >   PropertyChangeListeners;

    //========================================================================
    //= helper
    //========================================================================
    // small helper to make the "swap" call on an STL container a single-line call, which
    // in it's canonic form "aFoo.swap( Container() )" doesn't compile with GCC
    template< class CONTAINER >
    void clearContainer( CONTAINER& _rContainer )
    {
        CONTAINER aEmpty;
        _rContainer.swap( aEmpty );
    }

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static sal_uInt32 getHelpId( const ::rtl::OUString& _rHelpURL );
        static ::rtl::OUString getHelpURL( sal_uInt32 _nHelpId );
    };

    //========================================================================
    //= MeasurementUnitConversion
    //========================================================================
    /** small helper to convert between <type>MeasurementUnit</type> and
        <type>FieldUnit</type>
    */
    class MeasurementUnitConversion
    {
    public:
        static sal_Int16 convertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _rFieldToUNOValueFactor );
        static FieldUnit convertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor );
    };

    //====================================================================
    //= StlSyntaxSequence
    //====================================================================
    template< class ELEMENT >
    class StlSyntaxSequence : public ::com::sun::star::uno::Sequence< ELEMENT >
    {
    private:
        typedef ::com::sun::star::uno::Sequence< ELEMENT >  UnoBase;

    public:
        inline StlSyntaxSequence() : UnoBase() { }
        inline StlSyntaxSequence( const UnoBase& rSeq ) : UnoBase( rSeq ) { }
        inline StlSyntaxSequence( const ELEMENT* pElements, sal_Int32 len ) : UnoBase( pElements, len ) { }
        inline StlSyntaxSequence( sal_Int32 len ) : UnoBase( len ) { }

        operator const UnoBase&() const { return *this; }
        operator       UnoBase&()       { return *this; }

        typedef const ELEMENT* const_iterator;
        typedef       ELEMENT* iterator;

        inline const_iterator begin() const { return UnoBase::getConstArray(); }
        inline const_iterator end() const { return UnoBase::getConstArray() + UnoBase::getLength(); }

        inline iterator begin() { return UnoBase::getArray(); }
        inline iterator end() { return UnoBase::getArray() + UnoBase::getLength(); }

        inline sal_Int32 size() const { return UnoBase::getLength(); }
        inline bool empty() const { return UnoBase::getLength() == 0; }

        inline void resize( size_t _newSize ) { UnoBase::realloc( _newSize ); }

        inline iterator erase( iterator _pos )
        {
            iterator loop = end();
            while ( --loop != _pos )
                *( loop - 1 ) = *loop;
            resize( size() - 1 );
        }
    };

    //========================================================================
    //= UNO helpers
    //========================================================================
#define DECLARE_XCOMPONENT() \
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

#define IMPLEMENT_FORWARD_XCOMPONENT( classname, baseclass ) \
    void SAL_CALL classname::dispose(  ) throw (::com::sun::star::uno::RuntimeException) \
    { \
    baseclass::WeakComponentImplHelperBase::dispose(); \
    } \
    void SAL_CALL classname::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        baseclass::WeakComponentImplHelperBase::addEventListener( _Listener ); \
    } \
    void SAL_CALL classname::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& _Listener ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        baseclass::WeakComponentImplHelperBase::removeEventListener( _Listener ); \
    } \

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_

