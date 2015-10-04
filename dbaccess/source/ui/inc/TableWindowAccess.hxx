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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWACCESS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWACCESS_HXX

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <cppuhelper/implbase.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <vcl/vclptr.hxx>

namespace dbaui
{
    typedef ::cppu::ImplHelper < css::accessibility::XAccessibleRelationSet,
                                 css::accessibility::XAccessible
                                            > OTableWindowAccess_BASE;
    class OTableWindow;
    /** the class OTableWindowAccess represents the accessible object for table windows
        like they are used in the QueryDesign and the RelationDesign
    */
    class OTableWindowAccess    :   public VCLXAccessibleComponent
                                ,   public OTableWindowAccess_BASE
    {
        VclPtr<OTableWindow>   m_pTable; // the window which I should give accessibility to

        css::uno::Reference< css::accessibility::XAccessible > getParentChild(sal_Int32 _nIndex);
    protected:
        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;

        virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    public:
        OTableWindowAccess( OTableWindow* _pTable);

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire(  ) throw () override
        { // here inline is allowed because we do not use this class outside this dll
            VCLXAccessibleComponent::acquire(  );
        }
        virtual void SAL_CALL release(  ) throw () override
        { // here inline is allowed because we do not use this class outside this dll
            VCLXAccessibleComponent::release(  );
        }

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException,css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleExtendedComponent
        virtual OUString SAL_CALL getTitledBorderText(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleRelationSet
        virtual sal_Int32 SAL_CALL getRelationCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::AccessibleRelation SAL_CALL getRelation( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL containsRelation( sal_Int16 aRelationType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::accessibility::AccessibleRelation SAL_CALL getRelationByType( sal_Int16 aRelationType ) throw (css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
