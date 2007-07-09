#ifndef RPTUI_GROUPS_SORTING_HXX
#define RPTUI_GROUPS_SORTING_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GroupsSorting.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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

#ifndef _SV_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/imagebtn.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XGROUPS_HPP_
#include <com/sun/star/report/XGroups.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XGROUP_HPP_
#include <com/sun/star/report/XGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef RPT_SHARED_GROUPS_PROPERTIES_HXX
#include "GroupProperties.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <vector>

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}
namespace rptui
{
class OFieldExpressionControl;
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OGroupsSortingDialog :    public FloatingWindow
                            ,   public ::cppu::BaseMutex
                            ,   public ::comphelper::OPropertyChangeListener
{
    friend class OFieldExpressionControl;

    FixedLine                               m_aFL2;
    FixedText                               m_aMove;
    ImageButton                             m_aUp;
    ImageButton                             m_aDown;
    FixedLine                               m_aFL3;
    FixedText                               m_aOrder;
    ListBox                                 m_aOrderLst;
    FixedText                               m_aHeader;
    ListBox                                 m_aHeaderLst;
    FixedText                               m_aFooter;
    ListBox                                 m_aFooterLst;
    FixedText                               m_aGroupOn;
    ListBox                                 m_aGroupOnLst;
    FixedText                               m_aGroupInterval;
    NumericField                            m_aGroupIntervalEd;
    FixedText                               m_aKeepTogether;
    ListBox                                 m_aKeepTogetherLst;
    FixedLine                               m_aFL;
    FixedText                               m_aHelpWindow;

    OFieldExpressionControl*                m_pFieldExpression;
    ::rptui::OReportController*             m_pController;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pCurrentGroupListener;
    ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                       m_pReportListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups>            m_xGroups;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xColumns;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >          m_xHoldAlive;
    sal_Bool                                m_bReadOnly;
private:
    DECL_LINK( OnControlFocusLost, Control* );
    DECL_LINK( OnControlFocusGot, Control* );
    DECL_LINK( LBChangeHdl, ListBox* );
    DECL_LINK( ClickHdl, ImageButton* );

    /** returns the groups
        @return the groups which now have to check which one changes
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups>& getGroups() { return m_xGroups; }

    ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup> getGroup(sal_Int32 _nPos)
    {
        OSL_ENSURE(_nPos >= 0 && _nPos < m_xGroups->getCount(),"Invalid count!");
        return ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>(m_xGroups->getByIndex(_nPos),::com::sun::star::uno::UNO_QUERY);
    }

    /** updates the listboxes with the new group properties
        @param  _nRow   the new group pos
    */
    void DisplayData( sal_Int32 _nRow );

    /** saves the values from the listboxes into the group at position _nRow
        @param  _nRow   the group pos to store in
    */
    void SaveData( sal_Int32 _nRow );

    /** returns <TRUE/> when the dialog should be read only
    */
    sal_Bool isReadOnly( ) const;

    /** returns the data type for the given column name
        @param _sColumnName
    */
    sal_Int32 getColumnDataType(const ::rtl::OUString& _sColumnName);

    /** shows the text given by the id in the multiline edit
        @param  _nResId the string id
    */
    void showHelpText(USHORT _nResId);
    /** display the group props
        @param  _xGroup the group to display
    */
    void displayGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup);

    /** enables or diables the up and down button
        @param  _nRow   the row which will be active
    */
    void checkButtons(sal_Int32 _nRow);

    /** clears the m_xColumns member and reset the fields
    *
    */
    void fillColumns();
    OGroupsSortingDialog(OGroupsSortingDialog&);
    void operator =(OGroupsSortingDialog&);
protected:
    // window
    virtual void    Resize();
    // OPropertyChangeListener
    virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
public:
    OGroupsSortingDialog( Window* pParent
                        ,sal_Bool _bReadOnly
                        ,::rptui::OReportController* _pController);
    virtual ~OGroupsSortingDialog();

    /** sets the newe columns at the groups dialog.
        @param  _xColumns the new columns
    */
    void setColumns(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xColumns);

    /* updates the current view
    */
    void UpdateData( );
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_GROUPS_SORTING_HXX
