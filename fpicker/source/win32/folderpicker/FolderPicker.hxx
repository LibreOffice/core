/*************************************************************************
 *
 *  $RCSfile: FolderPicker.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:15:44 $
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

#ifndef _FOLDERPICKER_HXX_
#define _FOLDERPICKER_HXX_

//---------------------------------------------------------
//  includes of other projects
//---------------------------------------------------------

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif

#include <memory>

#ifndef _FPIMPLBASE_HXX_
#include "WinFOPImpl.hxx"
#endif

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CFolderPicker :
    public  cppu::WeakImplHelper2<
                com::sun::star::ui::dialogs::XFolderPicker,
                com::sun::star::lang::XServiceInfo >
{
public:

    // ctor/dtor
    CFolderPicker( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

    //------------------------------------------------------------------------------------
    // XExecutableDialog
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setTitle( const rtl::OUString& aTitle )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFolderPicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& aDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDirectory( )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDescription( const rtl::OUString& aDescription )
        throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // overwrite base class method, which is called
    // by base class dispose function
    //------------------------------------------------

    virtual void SAL_CALL disposing();

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;
    std::auto_ptr< CWinFolderPickerImpl >   m_pFolderPickerImpl;
    osl::Mutex  m_aMutex;

// prevent copy and assignment
private:
    CFolderPicker( const CFolderPicker& );
    CFolderPicker& operator=( const CFolderPicker&  );
};

#endif
