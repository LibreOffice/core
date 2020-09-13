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
#pragma once

#include "TableConnection.hxx"
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <vcl/vclptr.hxx>

namespace dbaui
{
    typedef ::cppu::ImplHelper2< css::accessibility::XAccessibleRelationSet,
                                 css::accessibility::XAccessible
                                            > OConnectionLineAccess_BASE;
    class OTableConnection;
    /** the class OConnectionLineAccess represents the accessible object for the connection between two table windows
        like they are used in the QueryDesign and the RelationDesign
    */
    class OConnectionLineAccess     :   public VCLXAccessibleComponent
                                    ,   public OConnectionLineAccess_BASE
    {
        VclPtr<const OTableConnection>             m_pLine; // the window which I should give accessibility to
    protected:
        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;

    public:
        OConnectionLineAccess(OTableConnection* _pLine);

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire(  ) throw () override
        { // here inline is allowed because we do not use this class outside this dll
            VCLXAccessibleComponent::acquire(  );
        }
        virtual void SAL_CALL release(  ) throw () override
        { // here inline is allowed because we do not use this class outside this dll
            VCLXAccessibleComponent::release(  );
        }

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
        virtual css::awt::Point SAL_CALL getLocation(  ) override;
        virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
        virtual css::awt::Size SAL_CALL getSize(  ) override;

        // XAccessibleRelationSet
        virtual sal_Int32 SAL_CALL getRelationCount(  ) override;
        virtual css::accessibility::AccessibleRelation SAL_CALL getRelation( sal_Int32 nIndex ) override;
        virtual sal_Bool SAL_CALL containsRelation( sal_Int16 aRelationType ) override;
        virtual css::accessibility::AccessibleRelation SAL_CALL getRelationByType( sal_Int16 aRelationType ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
