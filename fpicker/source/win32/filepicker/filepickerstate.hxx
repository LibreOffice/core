/*************************************************************************
 *
 *  $RCSfile: filepickerstate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:56:25 $
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

#ifndef _FILEPICKERSTATE_HXX_
#define _FILEPICKERSTATE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CONTROLCOMMAND_HXX_
#include "controlcommand.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//---------------------------------------------
//
//---------------------------------------------

class CControlCommand;
class CFileOpenDialog;

//---------------------------------------------
// declaration
//---------------------------------------------

class CFilePickerState
{
public:
    virtual ~CFilePickerState( );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue ) = 0;

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) = 0;

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable ) = 0;

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel ) = 0;

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId ) = 0;

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog ) = 0;

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog ) = 0;
};

//---------------------------------------------
// this class is not thread-safe
//---------------------------------------------

class CNonExecuteFilePickerState : public CFilePickerState
{
public:
    CNonExecuteFilePickerState( );

    virtual ~CNonExecuteFilePickerState( );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel );

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog );

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog );

    void SAL_CALL reset( );

    CControlCommand* SAL_CALL getControlCommand( ) const;

protected:
    void SAL_CALL addControlCommand( CControlCommand* aControlCommand );

private:
    CControlCommand* m_FirstControlCommand;
};

//---------------------------------------------
// this class is not thread-safe
//---------------------------------------------

class CExecuteFilePickerState : public CFilePickerState
{
public:
    CExecuteFilePickerState( HWND hwndDlg = NULL );

    virtual void SAL_CALL setValue( sal_Int16 aControlId, sal_Int16 aControlAction, const ::com::sun::star::uno::Any& aValue );

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( sal_Int16 aControlId, sal_Int16 aControlAction );

    virtual void SAL_CALL enableControl( sal_Int16 aControlId, sal_Bool bEnable );

    virtual void SAL_CALL setLabel( sal_Int16 aControlId, const ::rtl::OUString& aLabel );

    virtual ::rtl::OUString SAL_CALL getLabel( sal_Int16 aControlId );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFiles( CFileOpenDialog* aFileOpenDialog );

    virtual rtl::OUString SAL_CALL getDisplayDirectory( CFileOpenDialog* aFileOpenDialog );

    void SAL_CALL initFilePickerControls( CControlCommand* firstControlCommand );

    void SAL_CALL cacheControlState( HWND hwndControl, CFilePickerState* aFilePickerState );

    void SAL_CALL setHwnd( HWND hwndDlg );

private:

    inline sal_Bool SAL_CALL IsListboxControl( HWND hwndControl ) const;

    inline sal_Int16 SAL_CALL ListboxIdToListboxLabelId( sal_Int16 aListboxId ) const;

    inline HWND SAL_CALL GetListboxLabelItem( sal_Int16 aControlId ) const;

    // returns a hwnd for a control if successful
    // if bIncludeStdCtrls is false, the standard file dialog
    // controls like OK button, etc. will not be considered
    // the function return 0 on failure
    HWND SAL_CALL GetHwndDlgItem( sal_Int16 aControlId, sal_Bool bIncludeStdCtrls = sal_True ) const;

    HWND m_hwndDlg;
};

#endif
