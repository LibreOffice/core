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
#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX

#include <map>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>


//  class VCLXAccessibleToolBox


typedef ::cppu::ImplHelper < css::accessibility::XAccessibleSelection > VCLXAccessibleToolBox_BASE;

typedef std::map< sal_Int32, css::uno::Reference< css::accessibility::XAccessible > > ToolBoxItemsMap;

class VCLXAccessibleToolBoxItem;
class ToolBox;

class VCLXAccessibleToolBox : public VCLXAccessibleComponent, public VCLXAccessibleToolBox_BASE
{
private:
    ToolBoxItemsMap             m_aAccessibleChildren;

    VCLXAccessibleToolBoxItem*  GetItem_Impl( sal_Int32 _nPos, bool _bMustHaveFocus );

    void                        UpdateFocus_Impl();
    void                        ReleaseFocus_Impl( sal_Int32 _nPos );
    void                        UpdateChecked_Impl( sal_Int32 _nPos );
    void                        UpdateIndeterminate_Impl( sal_Int32 _nPos );
    void                        UpdateItem_Impl( sal_Int32 _nPos, bool _bItemAdded );
    void                        UpdateAllItems_Impl();
    void                        UpdateItemName_Impl( sal_Int32 _nPos );
    void                        UpdateItemEnabled_Impl( sal_Int32 _nPos );
    void                        UpdateCustomPopupItemp_Impl( vcl::Window* pWindow, bool bOpen );
    void                        HandleSubToolBarEvent( const VclWindowEvent& rVclWindowEvent, bool _bShow );
    void                        ReleaseSubToolBox( ToolBox* _pSubToolBox );

protected:
    virtual ~VCLXAccessibleToolBox();

    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent ) override;
    css::uno::Reference< css::accessibility::XAccessible > GetItemWindowAccessible( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing() override;

public:
    VCLXAccessibleToolBox( VCLXWindow* pVCLXWindow );

    // XInterface
    DECLARE_XINTERFACE( )

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

private:
    void implReleaseToolboxItem(
            ToolBoxItemsMap::iterator& _rMapPos,
            bool _bNotifyRemoval,
            bool _bDispose
        );
};

#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
