/*************************************************************************
 *
 *  $RCSfile: scripttypedetector.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_SCRIPTTYPEDETECTOR_HXX_
#define _I18N_SCRIPTTYPEDETECTOR_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include <drafts/com/sun/star/i18n/XScriptTypeDetector.hpp>

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx> // helper for implementations
#endif

//  ----------------------------------------------------
//  class ScriptTypeDetector
//  ----------------------------------------------------
class ScriptTypeDetector : public cppu::WeakImplHelper2
<
    ::drafts::com::sun::star::i18n::XScriptTypeDetector,
    ::com::sun::star::lang::XServiceInfo
>
{
public:
    ScriptTypeDetector();
    virtual ~ScriptTypeDetector();

    // Methods
    virtual sal_Int32 SAL_CALL beginOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL beginOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );
};

#endif
