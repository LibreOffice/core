/*************************************************************************
 *
 *  $RCSfile: SvFilterOptionsDialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sj $ $Date: 2002-06-18 15:10:13 $
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

#ifndef _SV_FILTER_OPTIONS_DIALOG_HXX_
#define _SV_FILTER_OPTIONS_DIALOG_HXX_

#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

class SvFilterOptionsDialog : public cppu::WeakImplHelper5
<
    com::sun::star::document::XExporter,
    com::sun::star::ui::dialogs::XExecutableDialog,
    com::sun::star::beans::XPropertyAccess,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &
        rxMgr;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        aMediaDescriptor;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        aFilterDataSequence;
    rtl::OUString   aDialogTitle;
    FieldUnit       eFieldUnit;

public:

    SvFilterOptionsDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );
    ~SvFilterOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // XPropertyAccess
    virtual com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};


#endif // _SV_FILTER_OPTIONS_DIALOG_HXX_

