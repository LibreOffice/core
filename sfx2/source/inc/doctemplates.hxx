/*************************************************************************
 *
 *  $RCSfile: doctemplates.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dv $ $Date: 2001-03-09 14:48:50 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SFX_DOCTEMPLATES_HXX_
#define _SFX_DOCTEMPLATES_HXX_

#ifndef  _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef  _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef  _COM_SUN_STAR_FRAME_XDOCUMENTTEMPLATES_HPP_
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#endif
#ifndef  _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef  _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif
#ifndef  _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef  _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif

#ifndef  _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#ifndef _SFX_SFXUNO_HXX
#include <sfxuno.hxx>
#endif

//--------------------------------------------------------------------------------------------------------

#define LOCALE                      ::com::sun::star::lang::Locale
#define REFERENCE                   ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION            ::com::sun::star::uno::RuntimeException
#define XCONTENT                    ::com::sun::star::ucb::XContent
#define XDOCUMENTTEMPLATES          ::com::sun::star::frame::XDocumentTemplates
#define XINTERFACE                  ::com::sun::star::uno::XInterface
#define XLOCALIZABLE                ::com::sun::star::lang::XLocalizable
#define XSERVICEINFO                ::com::sun::star::lang::XServiceInfo
#define XSTORABLE                   ::com::sun::star::frame::XStorable

//--------------------------------------------------------------------------------------------------------

class SfxDocTplService_Impl;

class SfxDocTplService: public ::cppu::WeakImplHelper3< XLOCALIZABLE, XDOCUMENTTEMPLATES, XSERVICEINFO >
{
    SfxDocTplService_Impl       *pImp;

public:
                                    SFX_DECL_XSERVICEINFO

                                    SfxDocTplService( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
                                   ~SfxDocTplService();

    // --- XLocalizable ---
    void SAL_CALL                   setLocale( const LOCALE & eLocale ) throw( RUNTIMEEXCEPTION );
    LOCALE SAL_CALL                 getLocale() throw( RUNTIMEEXCEPTION );

    // --- XDocumentTemplates ---
    REFERENCE< XCONTENT > SAL_CALL  getContent() throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               storeTemplate( const ::rtl::OUString& GroupName,
                                                   const ::rtl::OUString& TemplateName,
                                                   const REFERENCE< XSTORABLE >& Storable ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               addTemplate( const ::rtl::OUString& GroupName,
                                                 const ::rtl::OUString& TemplateName,
                                                 const ::rtl::OUString& SourceURL ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               removeTemplate( const ::rtl::OUString& GroupName,
                                                    const ::rtl::OUString& TemplateName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               addGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    sal_Bool SAL_CALL               removeGroup( const ::rtl::OUString& GroupName ) throw( RUNTIMEEXCEPTION );
    void SAL_CALL                   update() throw( RUNTIMEEXCEPTION );
};

#endif
