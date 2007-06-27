/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletableprovider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:41:04 $
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

#ifndef _SVTOOLS_ACCESSIBLETABLEPROVIDER_HXX
#define _SVTOOLS_ACCESSIBLETABLEPROVIDER_HXX

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _SVTOOLS_ACCESSIBLEBROWSEBOXOBJTYPE_HXX
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

// ============================================================================

namespace svt
{

// ============================================================================

#define OFFSET_DEFAULT  ((sal_Int32)-1)
#define OFFSET_NONE     ((sal_Int32)0)

// ============================================================================

enum AccessibleTableChildIndex
{
    /** Child index of the column header bar (first row). Exists always. */
    BBINDEX_COLUMNHEADERBAR = 0,
    /** Child index of the row header bar ("handle column"). Exists always. */
    BBINDEX_ROWHEADERBAR    = 1,
    /** Child index of the data table. */
    BBINDEX_TABLE           = 2,
    /** Child index of the first additional control. */
    BBINDEX_FIRSTCONTROL    = 3
};

// ============================================================================

#define XACC ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >

/** This abstract class provides methods to implement an accessible table object.
*/
class IAccessibleTableProvider
{
public:
    /** @return  The count of the rows. */
    virtual long                    GetRowCount() const = 0;
    /** @return  The count of the columns. */
    virtual sal_uInt16              GetColumnCount() const = 0;

    /** @return  The position of the current row. */
    virtual sal_Int32               GetCurrRow() const = 0;
    /** @return  The position of the current column. */
    virtual sal_uInt16              GetCurrColumn() const = 0;

    /** @return  The description of a row.
        @param _nRow The row which description is in demand. */
    virtual ::rtl::OUString         GetRowDescription( sal_Int32 _nRow ) const = 0;
    /** @return  The description of a column.
        @param _nColumn The column which description is in demand. */
    virtual ::rtl::OUString         GetColumnDescription( sal_uInt16 _nColumnPos ) const = 0;

    /** @return  <TRUE/>, if the object has a row header. */
    virtual sal_Bool                HasRowHeader() const = 0; //GetColumnId
    /** @return  <TRUE/>, if the object can focus a cell. */
    virtual sal_Bool                IsCellFocusable() const = 0;
    virtual sal_Bool                GoToCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) = 0;

    virtual void                    SetNoSelection() = 0;
    virtual void                    SelectAll() = 0;
    virtual void                    SelectRow( long _nRow, BOOL _bSelect = TRUE, BOOL bExpand = TRUE ) = 0;
    virtual void                    SelectColumn( sal_uInt16 _nColumnPos, sal_Bool _bSelect = sal_True ) = 0;
    virtual sal_Int32               GetSelectedRowCount() const = 0;
    virtual sal_Int32               GetSelectedColumnCount() const = 0;
    /** @return  <TRUE/>, if the row is selected. */
    virtual bool                    IsRowSelected( long _nRow ) const = 0;
    virtual sal_Bool                IsColumnSelected( long _nColumnPos ) const = 0;
    virtual void                    GetAllSelectedRows( ::com::sun::star::uno::Sequence< sal_Int32 >& _rRows ) const = 0;
    virtual void                    GetAllSelectedColumns( ::com::sun::star::uno::Sequence< sal_Int32 >& _rColumns ) const = 0;

    /** @return  <TRUE/>, if the cell is visible. */
    virtual sal_Bool                IsCellVisible( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const = 0;
    virtual String                  GetAccessibleCellText( long _nRow, USHORT _nColumnPos ) const = 0;

    virtual Rectangle               calcHeaderRect( sal_Bool _bIsColumnBar, BOOL _bOnScreen = TRUE ) = 0;
    virtual Rectangle               calcTableRect( BOOL _bOnScreen = TRUE ) = 0;
    virtual Rectangle               GetFieldRectPixelAbs( sal_Int32 _nRow, sal_uInt16 _nColumnPos, BOOL _bIsHeader, BOOL _bOnScreen = TRUE ) = 0;

    virtual XACC                    CreateAccessibleCell( sal_Int32 _nRow, sal_uInt16 _nColumnPos ) = 0;
    virtual XACC                    CreateAccessibleRowHeader( sal_Int32 _nRow ) = 0;
    virtual XACC                    CreateAccessibleColumnHeader( sal_uInt16 _nColumnPos ) = 0;

    virtual sal_Int32               GetAccessibleControlCount() const = 0;
    virtual XACC                    CreateAccessibleControl( sal_Int32 _nIndex ) = 0;
    virtual sal_Bool                ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint ) = 0;

    virtual sal_Bool                ConvertPointToCellAddress( sal_Int32& _rnRow, sal_uInt16& _rnColPos, const Point& _rPoint ) = 0;
    virtual sal_Bool                ConvertPointToRowHeader( sal_Int32& _rnRow, const Point& _rPoint ) = 0;
    virtual sal_Bool                ConvertPointToColumnHeader( sal_uInt16& _rnColPos, const Point& _rPoint ) = 0;

    virtual ::rtl::OUString         GetAccessibleObjectName( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const = 0;
    virtual ::rtl::OUString         GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const = 0;

    virtual void                    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& _rStateSet, ::svt::AccessibleBrowseBoxObjType _eType ) const = 0;
    virtual void                    FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const = 0;
    virtual void                    GrabTableFocus() = 0;

    // OutputDevice
    virtual BOOL                    GetGlyphBoundRects( const Point& rOrigin, const String& rStr, int nIndex, int nLen, int nBase, MetricVector& rVector ) = 0;

    // Window
    virtual Rectangle               GetWindowExtentsRelative( Window *pRelativeWindow ) = 0;
    virtual void                    GrabFocus() = 0;
    virtual XACC                    GetAccessible( BOOL bCreate = TRUE ) = 0;
    virtual Window*                 GetAccessibleParentWindow() const = 0;
    virtual Window*                 GetWindowInstance() = 0;

    virtual Rectangle               GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) = 0;
    virtual sal_Int32               GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) = 0;
};

// ----------------------------------------------------------------------------

/** interface for an implementation of a table control's Accesible component
*/
class IAccessibleTabListBox
{
public:
    /** returns the XAccessible object itself

        The reference returned here can be used to control the life time of the
        IAccessibleTableImplementation object.

        The returned reference is guaranteed to not be <NULL/>.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself() = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleChild( sal_Int32 nChildIndex )
            throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException ) = 0;

    /** returns the accessible object for the row or the column header bar
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType ) = 0;
};

/** interface for an implementation of a browse box's Accesible component
*/
class IAccessibleBrowseBox
{
public:
    /** returns the XAccessible object itself

        The reference returned here can be used to control the life time of the
        IAccessibleTableImplementation object.

        The returned reference is guaranteed to not be <NULL/>.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself() = 0;

    /** disposes the accessible implementation, so that it becomes defunc
    */
    virtual void dispose() = 0;

    /** checks whether the accessible implementation, and its context, are still alive
        @return  <TRUE/>, if the object is not disposed or disposing.
    */
    virtual sal_Bool isAlive() const = 0;

    /** returns the accessible object for the row or the column header bar
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType ) = 0;

    /** returns the accessible object for the table representation
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTable() = 0;

    /** commits the event at all listeners of the column/row header bar
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    virtual void commitHeaderBarEvent(
        sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue,
        sal_Bool _bColumnHeaderBar
    ) = 0;

    /** commits the event at all listeners of the table
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    virtual void commitTableEvent(
        sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue
    ) = 0;

    /** Commits an event to all listeners. */
    virtual void commitEvent(
        sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue
    ) = 0;
};

// ----------------------------------------------------------------------------

// ============================================================================

} // namespace svt

// ============================================================================

#endif // _SVTOOLS_ACCESSIBLETABLEPROVIDER_HXX

