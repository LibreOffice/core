/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WinFOPImpl.hxx,v $
 * $Revision: 1.3 $
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


#ifndef _WINFOPIMPL_HXX_
#define _WINFOPIMPL_HXX_


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "MtaFop.hxx"

//------------------------------------------------------------------------
// forward
//------------------------------------------------------------------------

class CFolderPicker;

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

class CWinFolderPickerImpl : public CMtaFolderPicker
{
public:
    CWinFolderPickerImpl( CFolderPicker* aFolderPicker );

    //-----------------------------------------------------------------------------------------
    // XExecutableDialog
    //-----------------------------------------------------------------------------------------

    virtual sal_Int16 SAL_CALL execute(  )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------
    // XFolderPicker
    //-----------------------------------------------------

    virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& aDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDisplayDirectory( )
        throw( com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDirectory( )
        throw( com::sun::star::uno::RuntimeException );

protected:
    virtual void SAL_CALL onSelChanged( const rtl::OUString& aNewPath );

private:
    CFolderPicker*  m_pFolderPicker;
    sal_Int16       m_nLastDlgResult;
};

#endif
