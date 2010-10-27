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
#ifndef DBACCESS_JACCESS_HXX
#define DBACCESS_JACCESS_HXX

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
        static ::rtl::OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );

        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);

        // XAccessible
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException,::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);

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
#endif // DBACCESS_JACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
