/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef RPTUI_PROPERTYSETFORWARD_HXX
#define RPTUI_PROPERTYSETFORWARD_HXX

#include "dllapi.h"
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/compbase1.hxx>
#include "cppuhelper/basemutex.hxx"
#include "RptDef.hxx"


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
