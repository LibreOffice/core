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

#ifndef INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLELISTBOX_HXX
#define INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLELISTBOX_HXX

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <vcl/vclevent.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <map>

// class AccessibleListBox -----------------------------------------------

class SvTreeListBox;
class SvTreeListEntry;

namespace accessibility
{
    class AccessibleListBoxEntry;

    /** the class OAccessibleListBoxEntry represents the base class for an accessible object of a listbox entry
    */
    class AccessibleListBox final :
        public cppu::ImplHelper2<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleSelection>,
        public VCLXAccessibleComponent
    {

        css::uno::Reference< css::accessibility::XAccessible > m_xParent;

        virtual ~AccessibleListBox() override;

        // OComponentHelper overridables
        /** this function is called upon disposing the component */
        virtual void SAL_CALL   disposing() override;

        // VCLXAccessibleComponent
        virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
        virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent ) override;
        virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

        VclPtr< SvTreeListBox > getListBox() const;

        void            RemoveChildEntries(SvTreeListEntry*);

        sal_Int32 GetRoleType();

    public:
        /** OAccessibleBase needs a valid view
            @param  _rListBox
                is the box for which we implement an accessible object
            @param  _xParent
                is our parent accessible object
        */
        AccessibleListBox( SvTreeListBox const & _rListBox,
                           const css::uno::Reference< css::accessibility::XAccessible >& _xParent );

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XInterface
        DECLARE_XINTERFACE()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;

        // XAccessibleSelection
        void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
        sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
        void SAL_CALL clearAccessibleSelection(  ) override;
        void SAL_CALL selectAllAccessibleChildren(  ) override;
        sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
        css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
        void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) override;

    private:

        typedef std::map< SvTreeListEntry*, css::uno::Reference< css::accessibility::XAccessible > > MAP_ENTRY;
        MAP_ENTRY m_mapEntry;

        css::uno::Reference< css::accessibility::XAccessible > m_xFocusedChild;

        accessibility::AccessibleListBoxEntry* GetCurEventEntry( const VclWindowEvent& rVclWindowEvent );

    };


}// namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_EXTENDED_ACCESSIBLELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
