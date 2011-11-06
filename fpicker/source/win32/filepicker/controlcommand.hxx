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
