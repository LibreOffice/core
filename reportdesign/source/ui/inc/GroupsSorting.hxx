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
#ifndef RPTUI_GROUPS_SORTING_HXX
#define RPTUI_GROUPS_SORTING_HXX

#include <vcl/floatwin.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <com/sun/star/report/XGroups.hpp>
#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "GroupProperties.hxx"
#include <comphelper/propmultiplex.hxx>
#include "cppuhelper/basemutex.hxx"
#include <svtools/svmedit.hxx>
#include <rtl/ref.hxx>

#include <vector>

#include <dbaccess/ToolBoxHelper.hxx>

#include <vcl/ImageListProvider.hxx>

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
                           ,public dbaui::OToolBoxHelper
                           ,public vcl::IImageListProvider
{
    friend class OFieldExpressionControl;

    FixedLine                               m_aFL2;
    FixedText                               m_aMove;
//BTN   ImageButton                             m_aPB_Up;
//BTN   ImageButton                             m_aPB_Down;
//BTN   ImageButton                             m_aPB_Delete;
    ToolBox                                 m_aToolBox;

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
    sal_Bool                                m_bReadOnly;
private:
    DECL_LINK( OnControlFocusLost, Control* );
    DECL_LINK( OnControlFocusGot, Control* );
    DECL_LINK( LBChangeHdl, ListBox* );
//BTN   DECL_LINK( ClickHdl, ImageButton* );
    DECL_LINK( OnFormatAction,      ToolBox* );

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
    void showHelpText(sal_uInt16 _nResId);
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

        /** will be called when the controls need to be resized.
            @param  _rDiff
                Contains the difference of the old and new toolbox size.
        */
        virtual void resizeControls(const Size& _rDiff);

        /** will be called when the image list is needed.
            @param  _eSymbolsSize
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
    virtual ImageList getImageList(sal_Int16 _eSymbolsSize,sal_Bool _bHiContast) const;

    // ImageListProvider interface
    virtual ImageList getImageList(vcl::ImageListType) SAL_THROW ((com::sun::star::lang::IllegalArgumentException ));

};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_GROUPS_SORTING_HXX
