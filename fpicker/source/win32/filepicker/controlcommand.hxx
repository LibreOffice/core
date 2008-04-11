/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlcommand.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _CONTROLCOMMAND_HXX_
#define _CONTROLCOMMAND_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

//---------------------------------------------
//
//---------------------------------------------

class CFilePickerState;
class CControlCommandRequest;
class CControlCommandResult;

//---------------------------------------------
//
//---------------------------------------------

class CControlCommand
{
public:
    CControlCommand( sal_Int16 aControlId );
    virtual ~CControlCommand( );

    virtual void SAL_CALL exec( CFilePickerState* aFilePickerState ) = 0;

    // the client inherits the ownership of the returned
    // CControlCommandResult and has to delete it or he may
    // use the auto_ptr template for automatic deletion
    virtual CControlCommandResult* SAL_CALL handleRequest( CControlCommandRequest* aRequest );

    // clients of this method should use the returned
    // pointer only temporary because it's not ref-counted
    // and the ownerhsip belongs to this instance
    CControlCommand* SAL_CALL getNextCommand( ) const;

    // transfers the ownership to this class
    void SAL_CALL setNextCommand( CControlCommand* nextCommand );

protected:
    sal_Int16 SAL_CALL getControlId( ) const;

private:
    CControlCommand* m_NextCommand;
    sal_Int16        m_aControlId;
};

//---------------------------------------------
//
//---------------------------------------------

class CValueControlCommand : public CControlCommand
{
public:
    CValueControlCommand(
        sal_Int16 aControlId,
        sal_Int16 aControlAction,
        const ::com::sun::star::uno::Any& aValue );

    virtual void SAL_CALL exec( CFilePickerState* aFilePickerState );

    virtual CControlCommandResult* SAL_CALL handleRequest( CControlCommandRequest* aRequest );

    sal_Int16 SAL_CALL getControlAction( ) const;

    ::com::sun::star::uno::Any SAL_CALL getValue( ) const;

private:
    sal_Int16                  m_aControlAction;
    ::com::sun::star::uno::Any m_aValue;
};

//---------------------------------------------
//
//---------------------------------------------

class CLabelControlCommand : public CControlCommand
{
public:
    CLabelControlCommand(
        sal_Int16 aControlId,
        const rtl::OUString& aLabel );

    virtual void SAL_CALL exec( CFilePickerState* aFilePickerState );

    virtual CControlCommandResult* SAL_CALL handleRequest( CControlCommandRequest* aRequest );

    rtl::OUString SAL_CALL getLabel( ) const;

private:
    rtl::OUString m_aLabel;
};

//---------------------------------------------
//
//---------------------------------------------

class CEnableControlCommand : public CControlCommand
{
public:
    CEnableControlCommand(
        sal_Int16 controlId,
        sal_Bool bEnable );

    virtual void SAL_CALL exec( CFilePickerState* aFilePickerState );

private:
    sal_Bool m_bEnable;
};

#endif
