/*************************************************************************
 *
 *  $RCSfile: clipboardmanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 12:08:30 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DTRANS_CLIPBOARDMANAGER_HXX_
#define _DTRANS_CLIPBOARDMANAGER_HXX_

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDMANAGER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include <map>

// ------------------------------------------------------------------------

#define CLIPBOARDMANAGER_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.ClipboardManager"

// ------------------------------------------------------------------------

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > > ClipboardMap;

// ------------------------------------------------------------------------

class OClipboardManager : public ::cppu::WeakComponentImplHelper3 < \
::com::sun::star::datatransfer::clipboard::XClipboardManager, \
::com::sun::star::lang::XEventListener, \
::com::sun::star::lang::XServiceInfo >
{
    ClipboardMap m_aClipboardMap;
    ::osl::Mutex m_aMutex;

    const ::rtl::OUString m_defaultName;

    virtual ~OClipboardManager();

public:

    OClipboardManager();

    /*
     * XServiceInfo
     */

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    /*
     * XComponent
     */

    virtual void SAL_CALL dispose()
        throw(::com::sun::star::uno::RuntimeException);

    /*
     * XEventListener
     */

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw(::com::sun::star::uno::RuntimeException);

    /*
     * XClipboardManager
     */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const ::rtl::OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard )
        throw(::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeClipboard( const ::rtl::OUString& aName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL listClipboardNames(  )
        throw(::com::sun::star::uno::RuntimeException);


};

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClipboardManager_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClipboardManager_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);


#endif





































































