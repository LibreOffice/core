/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_ACCESSIBLEFILTERTOPWINDOW_HXX
#define SC_ACCESSIBLEFILTERTOPWINDOW_HXX

//#include "AccessibleContextBase.hxx"
#include "AccessibleFilterMenu.hxx"
#include "cppuhelper/implbase1.hxx"

class ScDPFieldPopupWindow;
class ScDocument;

class ScAccessibleFilterTopWindow : public ScAccessibleFilterMenu
{
public:
    ScAccessibleFilterTopWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScDPFieldPopupWindow* pWin, const ::rtl::OUString& rName, ScDocument* pDoc);
    virtual ~ScAccessibleFilterTopWindow();

    // XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // Non-UNO Methods

    enum ChildControlType {
        LISTBOX, TOGGLE_ALL, SINGLE_ON_BTN, SINGLE_OFF_BTN, OK_BTN, CANCEL_BTN
    };
    void setAccessibleChild(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rAccessible,
        ChildControlType eType);

private:
    /** The top menu part */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccMenu;

    /** check list box for field member visibility */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccListBox;

    /** check box for toggling all field member's visibility. */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccToggleAll;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOnBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOffBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccOkBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccCancelBtn;

    ScDPFieldPopupWindow* mpWindow;
    ScDocument* mpDoc;
};

#endif
