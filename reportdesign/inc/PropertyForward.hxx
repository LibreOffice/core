/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_REPORTDESIGN_INC_PROPERTYFORWARD_HXX
#define INCLUDED_REPORTDESIGN_INC_PROPERTYFORWARD_HXX

#include "dllapi.h"
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include "RptDef.hxx"



namespace rptui
{

    typedef ::cppu::WeakComponentImplHelper<   css::beans::XPropertyChangeListener
                                    >   OPropertyForward_Base;

    /** \class OPropertyMediator
     * \brief This class ensures the communication between two XPropertySet instances.
     * Identical properties will be set at the other propertyset.
     * \ingroup reportdesign_source_ui_misc
     */
    class REPORTDESIGN_DLLPUBLIC OPropertyMediator : public ::cppu::BaseMutex
                            ,public OPropertyForward_Base
    {
        TPropertyNamePair                                     m_aNameMap;
        css::uno::Reference< css::beans::XPropertySet>        m_xSource;
        css::uno::Reference< css::beans::XPropertySetInfo>    m_xSourceInfo;
        css::uno::Reference< css::beans::XPropertySet>        m_xDest;
        css::uno::Reference< css::beans::XPropertySetInfo>    m_xDestInfo;
        bool                                                  m_bInChange;
        OPropertyMediator(OPropertyMediator&) = delete;
        void operator =(OPropertyMediator&) = delete;
    protected:
        virtual ~OPropertyMediator();

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;
    public:
        OPropertyMediator(const css::uno::Reference< css::beans::XPropertySet>& _xSource
                        ,const css::uno::Reference< css::beans::XPropertySet>& _xDest
                        ,const TPropertyNamePair& _aNameMap
                        ,bool _bReverse = false);

        // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& _rSource ) throw (css::uno::RuntimeException, std::exception) override;

        /** stop the listening mode.
         */
        void stopListening();

        /** starts the listening mode again.
         */
        void startListening();
    };

}   // namespace rptui

#endif // INCLUDED_REPORTDESIGN_INC_PROPERTYFORWARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
