#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#define RPTUI_PROPERTYSETFORWARD_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyForward.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:02 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#include "dllapi.h"

//........................................................................
namespace rptui
{
//........................................................................
    typedef ::cppu::WeakComponentImplHelper1<   ::com::sun::star::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;

    /** \class OPropertyMediator
     * \brief This class ensures the communication between two XPropertySet instances.
     * Identical properties will be set at the other propertyset.
     * \ingroup reportdesign_source_ui_misc
     */
    class REPORTDESIGN_DLLPUBLIC OPropertyMediator : public ::cppu::BaseMutex
                            ,public OPropertyForward_Base
    {
        TPropertyNamePair                                                               m_aNameMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xSource;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>    m_xSourceInfo;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>        m_xDest;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>    m_xDestInfo;
        sal_Bool                                                                        m_bInChange;
        OPropertyMediator(OPropertyMediator&);
        void operator =(OPropertyMediator&);
    protected:
        virtual ~OPropertyMediator();

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing();
    public:
        OPropertyMediator(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSource
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDest
                        ,const TPropertyNamePair& _aNameMap
                        ,sal_Bool _bReverse = sal_False);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        /** stop the listening mode.
         */
        void stopListening();

        /** starts the listening mode again.
         */
        void startListening();
    };
//........................................................................
}   // namespace rptui
//........................................................................
#endif // RPTUI_PROPERTYSETFORWARD_HXX

