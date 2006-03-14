/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propeventtranslation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:31:19 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyEventTranslation
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   PropertyEventTranslation_Base;

    class PropertyEventTranslation : public PropertyEventTranslation_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >
                m_xDelegator;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                m_xTranslatedEventSource;

    public:
        /** constructs the object
            @throws NullPointerException
                if <arg>_rxDelegator</arg> is <NULL/>
        */
        PropertyEventTranslation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxDelegator,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxTranslatedEventSource
        );

        inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >&
            getDelegator() const { return m_xDelegator; }

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        PropertyEventTranslation();                                             // never implemented
        PropertyEventTranslation( const PropertyEventTranslation& );            // never implemented
        PropertyEventTranslation& operator=( const PropertyEventTranslation& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX

