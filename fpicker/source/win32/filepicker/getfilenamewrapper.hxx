/*************************************************************************
 *
 *  $RCSfile: getfilenamewrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-10-04 11:08:05 $
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


#ifndef _GETFILENAMEWRAPPER_HXX_
#define _GETFILENAMEWRAPPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <systools/win32/comdlg9x.h>

/*
    A simple wrapper around the GetOpenFileName/GetSaveFileName API.
    This wrapper serves as base class because currently the Win32
    API functions GetOpenFileName/GetSaveFileName work only properly
    in an STA. If this changes in the future we simply use the base
    class else we use an derived class which calls the API functions
    from within an STA.
*/

class CGetFileNameWrapper
{
public:
    virtual ~CGetFileNameWrapper( ) { };

    virtual BOOL SAL_CALL getOpenFileName( LPOPENFILENAMEW lpofn )
    {
        return ::GetOpenFileNameW( lpofn );
    }

    virtual BOOL SAL_CALL getSaveFileName( LPOPENFILENAMEW lpofn )
    {
        return ::GetSaveFileNameW( lpofn );
    }

    virtual DWORD SAL_CALL commDlgExtendedError( )
    {
        return ::CommDlgExtendedError( );
    }

    // a factory method, so it's possible to create
    // different instances of this class depending
    // on the apartment model of the calling thread,
    // the OS version etc.
    // the client owns the returned instance and
    // should destroy it using delete
    static CGetFileNameWrapper* create( );

protected:

    // let instances only be created through
    // the create method
    CGetFileNameWrapper( ) { };
};

/*
    The wrapper always calls the GetOpenFileName/GetSaveFileName in
    a STA thread because a lot of features of the system file dialogs
    are disabled when the calling thread is an MTA thread
    (this is a MS Bug at least that they don't document this).
    This class is not thread-safe.
*/

class CSTAGetFileNameWrapper : public CGetFileNameWrapper
{
public:
    virtual ~CSTAGetFileNameWrapper( );

    virtual BOOL SAL_CALL getOpenFileName( LPOPENFILENAMEW lpofn );

    virtual BOOL SAL_CALL getSaveFileName( LPOPENFILENAMEW lpofn );

    virtual DWORD SAL_CALL commDlgExtendedError( );

protected:
    CSTAGetFileNameWrapper( );

private:
    void SAL_CALL executeGetFileName( );
    void SAL_CALL executeGetFileName( BOOL bFileOpenDialog, LPOPENFILENAMEW lpofn );
    bool SAL_CALL threadExecuteGetFileName( );

    static unsigned __stdcall threadProc( void * pParam );

private:
    DWORD           m_LastError;
    BOOL            m_bFileOpenDialog;
    BOOL            m_bResult;
    LPOPENFILENAMEW m_lpofn;

// prevent copy/assignment
private:
    CSTAGetFileNameWrapper( const CSTAGetFileNameWrapper& );
    CSTAGetFileNameWrapper& operator=( const CSTAGetFileNameWrapper& );

    friend class CGetFileNameWrapper;
};

#endif
