/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibletoolbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:29:16 $
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
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX
#define ACCESSIBILITY_STANDARD_VCLXACCESSIBLETOOLBOX_HXX

#include <map>

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif

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

