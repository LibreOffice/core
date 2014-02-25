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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_JACCESS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_JACCESS_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <cppuhelper/implbase1.hxx>

namespace dbaui
{
    class OJoinTableView;
    typedef ::cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessible
                                            > OJoinDesignViewAccess_BASE;
    /** the class OJoinDesignViewAccess represents the accessible object for join views
        like the QueryDesign and the RelationDesign
    */
    class OJoinDesignViewAccess     :   public VCLXAccessibleComponent, public OJoinDesignViewAccess_BASE
    {
        OJoinTableView* m_pTableView; // the window which I should give accessibility to

    protected:
        /** isEditable returns the current editable state
            @return true if the controller is not readonly otherwise false
        */
        virtual sal_Bool isEditable() const;
    public:
        /** OJoinDesignViewAccess needs a valid view
        */
        OJoinDesignViewAccess(  OJoinTableView* _pTableView);

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XServiceInfo - static methods
        static OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );

        virtual OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException, std::exception);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException, std::exception);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        OJoinTableView* getTableView() const { return m_pTableView; }

        void notifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }

        void clearTableView();
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_JACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
