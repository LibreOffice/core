/*************************************************************************
 *
 *  $RCSfile: charmapacc.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:58:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#ifndef COMPHELPER_ACCESSIBLE_SELECTION_HELPER_HXX
#include <comphelper/accessibleselectionhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#endif

#include <vector>
class SvxShowCharSet;

namespace svx
{
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                                >   OAccessibleHelper_Base_2;

    class SvxShowCharSetAcc;
    /** The class SvxShowCharSetVirtualAcc is used as a virtual class which contains the table and the scrollbar.
        In the vcl control, the table and the scrollbar exists in one class. This is not feasible for the accessibility api.
    */
    class SvxShowCharSetVirtualAcc : public ::comphelper::OAccessibleComponentHelper,
                                     public OAccessibleHelper_Base_2
    {
        SvxShowCharSet*     mpParent; // the vcl control
        SvxShowCharSetAcc*  m_pTable; // the table, which holds the characters shown by the vcl control
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAcc; // the ref to the table
    protected:
        virtual ~SvxShowCharSetVirtualAcc();

        virtual void SAL_CALL disposing();

        virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    public:
        SvxShowCharSetVirtualAcc( SvxShowCharSet* pParent );

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException) { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);


        // call the fireEvent method from the table when it exists.
        void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                );

        // simple access methods
        inline SvxShowCharSetAcc*   getTable() const { return m_pTable; }
        inline SvxShowCharSet* getCharSetControl() const { return mpParent; }
    };


    class SvxShowCharSetItemAcc;
    // ----------------
    // - SvxShowCharSetItem -
    // ----------------
    /** Simple struct to hold some information about the single items of the table.
    */
    struct SvxShowCharSetItem
    {
        SvxShowCharSet&             mrParent;
        USHORT                      mnId;
        XubString                   maText;
        Rectangle                   maRect;
        SvxShowCharSetItemAcc*      m_pItem;
        SvxShowCharSetAcc*          m_pParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xAcc;

        SvxShowCharSetItem( SvxShowCharSet& rParent,SvxShowCharSetAcc*  _pParent,USHORT _nPos );
        ~SvxShowCharSetItem();

        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >    GetAccessible();
         void                                                                                       ClearAccessible();
    };

    // -----------------------------------------------------------------------------

    typedef ::cppu::ImplHelper2 <   ::com::sun::star::accessibility::XAccessible,
                                    ::com::sun::star::accessibility::XAccessibleTable
                                >   OAccessibleHelper_Base;
    // ---------------
    // - SvxShowCharSetAcc -
    // ---------------
    /** The table implemtentation of the vcl control.
    */

    class SvxShowCharSetAcc : public ::comphelper::OAccessibleSelectionHelper,
                              public OAccessibleHelper_Base
    {
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > m_aChildren;
        SvxShowCharSetVirtualAcc* m_pParent; // the virtual parent
    protected:
        virtual void SAL_CALL disposing();
    public:
        SvxShowCharSetAcc( SvxShowCharSetVirtualAcc* _pParent );

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException) { return this; }
        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleTable
        virtual sal_Int32 SAL_CALL getAccessibleRowCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleColumnCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleRowHeaders(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL getAccessibleColumnHeaders(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleCaption(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleSummary(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);


        inline void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    protected:

        virtual ~SvxShowCharSetAcc();

        // OCommonAccessibleSelection
        // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual sal_Bool
            implIsSelected( sal_Int32 nAccessibleChildIndex )
            throw (::com::sun::star::uno::RuntimeException);

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
        virtual void
            implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect )
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    // ----------------
    // - SvxShowCharSetItemAcc -
    // ----------------
    /** The child implementation of the table.
    */
    class SvxShowCharSetItemAcc : public ::comphelper::OAccessibleComponentHelper,
                                  public OAccessibleHelper_Base_2
    {
    private:
        SvxShowCharSetItem* mpParent;
    protected:
        virtual ~SvxShowCharSetItemAcc();

        // OCommonAccessibleComponent
        /// implements the calculation of the bounding rectangle - still waiting to be overwritten
        virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);
    public:

        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent );

        void    ParentDestroyed();

        // XAccessibleComponent
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);

        //OAccessibleContextHelper
        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException) { return this; }

        virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException) { return mpParent->m_pParent->getForeground(); }
        virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException) { return mpParent->m_pParent->getBackground(); }

        inline void SAL_CALL fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
        {
            NotifyAccessibleEvent(_nEventId,_rOldValue,_rNewValue);
        }
    };
}


