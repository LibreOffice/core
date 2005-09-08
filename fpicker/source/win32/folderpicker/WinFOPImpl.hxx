/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WinFOPImpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:50:37 $
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


#ifndef _WINFOPIMPL_HXX_
#define _WINFOPIMPL_HXX_


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _MTAFOP_HXX_
#include "MtaFop.hxx"
#endif

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
