/*************************************************************************
 *
 *  $RCSfile: SalGtkPicker.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-08-05 15:14:39 $
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
 *  Contributor(s): Anil Bhatia
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _FPSERVICEINFO_HXX_
#include <FPServiceInfo.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SALGTKPICKER_HXX_
#include "SalGtkPicker.hxx"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <stdio.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

rtl::OUString SalGtkPicker::uritounicode(const gchar* pIn)
{
    rtl::OUString sURL( const_cast<const sal_Char *>(pIn), strlen(pIn),
        RTL_TEXTENCODING_UTF8 );
    INetURLObject aURL(sURL);
    if (INET_PROT_FILE == aURL.GetProtocol())
    {
        gchar *pEncodedFileName = g_filename_from_uri(pIn, NULL, NULL);
        rtl::OUString sEncoded(pEncodedFileName, strlen(pEncodedFileName),
            osl_getThreadTextEncoding());
        INetURLObject aCurrentURL(sEncoded, INetURLObject::FSYS_UNX);
        aCurrentURL.SetHost(aURL.GetHost());
        sURL = aCurrentURL.getExternalURL();
    }
    return sURL;
}

rtl::OString SalGtkPicker::unicodetouri(const rtl::OUString &rURL)
{
    OString sURL = OUStringToOString(rURL, RTL_TEXTENCODING_UTF8);
    INetURLObject aURL(rURL);
    if (INET_PROT_FILE == aURL.GetProtocol())
    {
        rtl::OUString sOUURL = aURL.getExternalURL(INetURLObject::DECODE_WITH_CHARSET, osl_getThreadTextEncoding());
        sURL = OUStringToOString( sOUURL, osl_getThreadTextEncoding());
    }
    return sURL;
}

gboolean rundialog(RunDialog *pDialog)
{
    pDialog->run();
    return false;
}

void RunDialog::run()
{
    mnStatus = gtk_dialog_run( GTK_DIALOG( m_pDialog ) );
    gtk_widget_hide( m_pDialog );

    maLock.acquire();
    mbFinished = true;
    maLock.release();

    Application::EndYield();
}

gint RunDialog::runandwaitforresult()
{
    g_timeout_add_full(G_PRIORITY_HIGH_IDLE, 0, (GSourceFunc)rundialog, this, NULL);
    while (1)
    {
    maLock.acquire();
        if (mbFinished)
            break;
    maLock.release();
        Application::Yield();
    }
    return mnStatus;
}

SalGtkPicker::~SalGtkPicker()
{
    if (m_pDialog)
        gtk_widget_destroy(m_pDialog);
}

void SAL_CALL SalGtkPicker::implsetDisplayDirectory( const rtl::OUString& aDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OString aTxt = unicodetouri(aDirectory);

    if( aTxt.lastIndexOf('/') == aTxt.getLength() - 1 )
        aTxt = aTxt.copy( 0, aTxt.getLength() - 1 );

    OSL_TRACE( "setting path to %s\n", aTxt.getStr() );

    gtk_file_chooser_set_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ),
                         aTxt.getStr() );
}

rtl::OUString SAL_CALL SalGtkPicker::implgetDisplayDirectory() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    gchar* pCurrentFolder =
        gtk_file_chooser_get_current_folder_uri( GTK_FILE_CHOOSER( m_pDialog ) );
    ::rtl::OUString aCurrentFolderName = uritounicode(pCurrentFolder);
    g_free( pCurrentFolder );

    return aCurrentFolderName;
}

void SAL_CALL SalGtkPicker::implsetTitle( const rtl::OUString& aTitle ) throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ::rtl::OString aWindowTitle = OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 );
    gtk_window_set_title( GTK_WINDOW( m_pDialog ), aWindowTitle.getStr() );
}
