/*************************************************************************
 *
 *  $RCSfile: controlcommand.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:53:38 $
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

#ifndef _CONTROLCOMMAND_HXX_
#define _CONTROLCOMMAND_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
