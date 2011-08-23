/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _NUMUNO_HXX
#define _NUMUNO_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

namespace binfilter
{

class SvNumberFormatter;
class SvNumFmtSuppl_Impl;

//------------------------------------------------------------------

//	SvNumberFormatterServiceObj must be registered as service somewhere

com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL
    SvNumberFormatterServiceObj_NewInstance(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory>& rSMgr );

//------------------------------------------------------------------

//	SvNumberFormatsSupplierObj: aggregate to document,
//	construct with SvNumberFormatter

class  SvNumberFormatsSupplierObj : public cppu::WeakAggImplHelper2<
                                    com::sun::star::util::XNumberFormatsSupplier,
                                    com::sun::star::lang::XUnoTunnel>
{
private:
    SvNumFmtSuppl_Impl*	pImpl;

public:
                                SvNumberFormatsSupplierObj();
                                SvNumberFormatsSupplierObj(SvNumberFormatter* pForm);
    virtual						~SvNumberFormatsSupplierObj();

    void						SetNumberFormatter(SvNumberFormatter* pNew);
    SvNumberFormatter*			GetNumberFormatter() const;

                                // ueberladen, um Attribute im Dokument anzupassen
    virtual void				NumberFormatDeleted(sal_uInt32 nKey);
                                // ueberladen, um evtl. neu zu formatieren
    virtual void				SettingsChanged();

                                // XNumberFormatsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                                getNumberFormatSettings()
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > SAL_CALL
                                getNumberFormats()
                                    throw(::com::sun::star::uno::RuntimeException);

                                // XUnoTunnel
    virtual sal_Int64 SAL_CALL	getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                        throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static SvNumberFormatsSupplierObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::util::XNumberFormatsSupplier> xObj );
};

}

#endif // #ifndef _NUMUNO_HXX


