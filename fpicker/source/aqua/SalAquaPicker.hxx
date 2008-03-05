/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SalAquaPicker.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:38:55 $
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

#ifndef _SALAQUAFPICKER_HXX_
#define _SALAQUAFPICKER_HXX_

//_____________________________________________________________________________
//  includes of other projects
//_____________________________________________________________________________

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HXX_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HXX_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _CONTROLHELPER_HXX_
#include "ControlHelper.hxx"
#endif

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalAquaPicker
{
public:
    // constructor
    SalAquaPicker();
    virtual ~SalAquaPicker();

    int run();
    int runandwaitforresult();

    inline rtl::OUString getDisplayDirectory() { return m_sDisplayDirectory; }

    inline ControlHelper* getControlHelper() const {
        return m_pControlHelper;
    }

protected:

    rtl::OUString m_sDisplayDirectory;

    NSSavePanel *m_pDialog;

    ControlHelper *m_pControlHelper;

    osl::Mutex m_rbHelperMtx;

    // The type of dialog
    enum NavigationServices_DialogType {
        NAVIGATIONSERVICES_OPEN,
        NAVIGATIONSERVICES_SAVE,
        NAVIGATIONSERVICES_DIRECTORY
    };

    NavigationServices_DialogType m_nDialogType;

    void implsetTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    void implsetDisplayDirectory( const rtl::OUString& rDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    rtl::OUString implgetDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    void implInitialize( );

};

#endif
