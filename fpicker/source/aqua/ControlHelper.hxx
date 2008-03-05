/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:35:07 $
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

#ifndef _CONTROLHELPER_HXX_
#define _CONTROLHELPER_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <list>
#include <map>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#ifndef _SALAQUACONSTANTS_H_
#include "SalAquaConstants.h"
#endif

#ifndef _FILTERHELPER_HXX_
#include "FilterHelper.hxx"
#endif

#ifndef _AQUAFILEPICKERDELEGATE_HXX_
#include "AquaFilePickerDelegate.hxx"
#endif

using namespace com::sun::star;
using namespace rtl;

class ControlHelper {

public:

    //------------------------------------------------------------------------------------
    // Constructor / Destructor
    //------------------------------------------------------------------------------------
    ControlHelper();
    virtual ~ControlHelper();

    //------------------------------------------------
    // XInitialization delegate
    //------------------------------------------------
    void       initialize( sal_Int16 templateId );

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess function delegates
    //------------------------------------------------------------------------------------
    void       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue );
    uno::Any   getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const;
    void       enableControl( sal_Int16 nControlId, sal_Bool bEnable ) const;
    OUString   getLabel( sal_Int16 nControlId );
    void       setLabel( sal_Int16 nControlId, const NSString* aLabel );

    //------------------------------------------------------------------------------------
    // other stuff
    //------------------------------------------------------------------------------------
    void       updateFilterUI();

    //------------------------------------------------------------------------------------
    // Type definitions
    //------------------------------------------------------------------------------------
    enum ToggleType {
        AUTOEXTENSION, //but autoextension is handled differently on MacOSX
        PASSWORD,
        FILTEROPTIONS,
        READONLY,
        LINK,
        PREVIEW,
        SELECTION,
        TOGGLE_LAST
    };

    enum ListType {
        VERSION,
        TEMPLATE,
        IMAGE_TEMPLATE,
        LIST_LAST
    };

    //------------------------------------------------------------------------------------
    // inline functions
    //------------------------------------------------------------------------------------
    inline NSView* getUserPane() {
        if (m_bIsUserPaneLaidOut == false) {
            createUserPane();
        }
        return m_pUserPane;
    }

    inline bool    getVisibility(ToggleType tToggle) {
        return m_bToggleVisibility[tToggle];
    }

    inline void    setFilterControlNeeded(bool bNeeded) {
        m_bIsFilterControlNeeded = bNeeded;
        if (bNeeded == true) {
            m_bUserPaneNeeded = true;
        }
    }

    inline void    setFilterHelper(FilterHelper* pFilterHelper) {
        m_pFilterHelper = pFilterHelper;
    }

    inline void    setFilePickerDelegate(AquaFilePickerDelegate* pDelegate) {
        m_pDelegate = pDelegate;
    }

    inline bool    isAutoExtensionEnabled() {
        return m_bAutoFilenameExtension;
    }

private:
    //------------------------------------------------------------------------------------
    // private member variables
    //------------------------------------------------------------------------------------

    /** the native view object */
    NSView*    m_pUserPane;

    /** the checkbox controls */
    NSControl* m_pToggles[ TOGGLE_LAST ];

    /** the visibility flags for the checkboxes */
    bool m_bToggleVisibility[TOGGLE_LAST];

    /** the special filter control */
    NSPopUpButton *m_pFilterControl;

    /** the popup menu controls (except for the filter control) */
    NSControl* m_pListControls[ LIST_LAST ];

    /** a map to store a control's label text */
    ::std::map<NSControl *, NSString *> m_aMapListLabels;

    /** a map to store a popup menu's label text field */
    ::std::map<NSPopUpButton *, NSTextField *> m_aMapListLabelFields;

    /** the visibility flags for the popup menus */
    bool m_bListVisibility[ LIST_LAST ];

    /** indicates if a user pane is needed */
    bool m_bUserPaneNeeded;

    /** indicates if the user pane was laid out already */
    bool m_bIsUserPaneLaidOut;

    /** indicates if a filter control is needed */
    bool m_bIsFilterControlNeeded;

    /*
     * With issue #78852 the GUI option to set automatic filename extension was removed but
     * so far the setting is still in effect and so the value for has to be saved and possibly be restored
     * also.
     * But with issue #FILLME users were starting to get confused on Windows so we decided to have
     * autoextension always on and have it selectable for the user again.
     */
    /** indicates if a filename extension is to automatically be appended to a user entered file name */
    bool m_bAutoFilenameExtension;

    /** a list with all actively used controls */
    ::std::list<NSControl*> m_aActiveControls;

    /** the filter helper */
    FilterHelper *m_pFilterHelper;

    /** the save or open panel's delegate */
    AquaFilePickerDelegate *m_pDelegate;

    //------------------------------------------------------------------------------------
    // private methods
    //------------------------------------------------------------------------------------
    void HandleSetListValue(const NSControl* pControl, const sal_Int16 nControlAction, const uno::Any& rValue);
    uno::Any HandleGetListValue(const NSControl* pControl, const sal_Int16 nControlAction) const;

    void         createControls();
    void         createFilterControl();
    void         createUserPane();
    NSTextField* createLabelWithString(const NSString* label);

    int          getControlElementName(const Class clazz, const int nControlId) const;
    NSControl*   getControl( const sal_Int16 nControlId ) const;
    static int   getVerticalDistance(const NSControl* first, const NSControl* second);

    void         layoutControls();
};

#endif //_CONTROLHELPER_HXX_
