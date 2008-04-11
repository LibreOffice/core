/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessibletoolbox.hxx,v $
 * $Revision: 1.3 $
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
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX

#include <map>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

//  ----------------------------------------------------
//  class VCLXAccessibleToolBox
//  ----------------------------------------------------

typedef ::cppu::ImplHelper1 < ::com::sun::star::accessibility::XAccessible > VCLXAccessibleToolBox_BASE;

typedef std::map< sal_Int32, com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > ToolBoxItemsMap;

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
    void                        UpdateItem_Impl( sal_Int32 _nPos, sal_Bool _bItemAdded );
    void                        UpdateAllItems_Impl();
    void                        UpdateItemName_Impl( sal_Int32 _nPos );
    void                        UpdateItemEnabled_Impl( sal_Int32 _nPos );
    void                        HandleSubToolBarEvent( const VclWindowEvent& rVclWindowEvent, bool _bShow );
    void                        ReleaseSubToolBox( ToolBox* _pSubToolBox );

protected:
    virtual ~VCLXAccessibleToolBox();

    virtual void    FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    virtual void    ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetChildAccessible( const VclWindowEvent& rVclWindowEvent );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetItemWindowAccessible( const VclWindowEvent& rVclWindowEvent );

    // XComponent
    virtual void SAL_CALL disposing();

public:
    VCLXAccessibleToolBox( VCLXWindow* pVCLXWindow );

    // XInterface
    DECLARE_XINTERFACE( )

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);

private:
    void implReleaseToolboxItem(
            ToolBoxItemsMap::iterator& _rMapPos,
            bool _bNotifyRemoval,
            bool _bDispose
        );
};

#endif // ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX

