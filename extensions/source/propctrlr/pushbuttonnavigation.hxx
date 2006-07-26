/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pushbuttonnavigation.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 08:00:44 $
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

#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#define EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= PushButtonNavigation
    //========================================================================
    class PushButtonNavigation
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xControlModel;
        sal_Bool    m_bIsPushButton;

    public:
        /** ctor
            @param _rxControlModel
                the control model which is or will be bound
        */
        PushButtonNavigation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel
        );

        /** returns the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        ::com::sun::star::uno::Any
                getCurrentButtonType() const SAL_THROW(());

        /** sets the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        void    setCurrentButtonType( const ::com::sun::star::uno::Any& _rValue ) const SAL_THROW(());

        /** retrieves the current state of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        ::com::sun::star::beans::PropertyState
                getCurrentButtonTypeState( ) const SAL_THROW(());

        /** returns the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        ::com::sun::star::uno::Any
                getCurrentTargetURL() const SAL_THROW(());

        /** sets the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        void    setCurrentTargetURL( const ::com::sun::star::uno::Any& _rValue ) const SAL_THROW(());

        /** retrieves the current state of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        ::com::sun::star::beans::PropertyState
                getCurrentTargetURLState( ) const SAL_THROW(());

        /** determines whether the current button tpye is FormButtonType_URL
        */
        bool    currentButtonTypeIsOpenURL() const;

        /** determines whether the TargetURL property does currently denote a non-empty string
        */
        bool    hasNonEmptyCurrentTargetURL() const;

    private:
        sal_Int32 implGetCurrentButtonType() const SAL_THROW(( ::com::sun::star::uno::Exception ));
    };

//............................................................................
}   // namespace pcr
//............................................................................

#endif // EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
