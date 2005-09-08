/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numuno.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:01:27 $
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
#ifndef _NUMUNO_HXX
#define _NUMUNO_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
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

class SvNumberFormatter;
class SvNumFmtSuppl_Impl;

//------------------------------------------------------------------

//  SvNumberFormatterServiceObj must be registered as service somewhere

com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL
    SvNumberFormatterServiceObj_NewInstance(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory>& rSMgr );

//------------------------------------------------------------------

//  SvNumberFormatsSupplierObj: aggregate to document,
//  construct with SvNumberFormatter

class SVT_DLLPUBLIC SvNumberFormatsSupplierObj : public cppu::WeakAggImplHelper2<
                                    com::sun::star::util::XNumberFormatsSupplier,
                                    com::sun::star::lang::XUnoTunnel>
{
private:
    SvNumFmtSuppl_Impl* pImpl;

public:
                                SvNumberFormatsSupplierObj();
                                SvNumberFormatsSupplierObj(SvNumberFormatter* pForm);
    virtual                     ~SvNumberFormatsSupplierObj();

    void                        SetNumberFormatter(SvNumberFormatter* pNew);
    SvNumberFormatter*          GetNumberFormatter() const;

                                // ueberladen, um Attribute im Dokument anzupassen
    virtual void                NumberFormatDeleted(sal_uInt32 nKey);
                                // ueberladen, um evtl. neu zu formatieren
    virtual void                SettingsChanged();

                                // XNumberFormatsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                                getNumberFormatSettings()
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > SAL_CALL
                                getNumberFormats()
                                    throw(::com::sun::star::uno::RuntimeException);

                                // XUnoTunnel
    virtual sal_Int64 SAL_CALL  getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                        throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static SvNumberFormatsSupplierObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::util::XNumberFormatsSupplier> xObj );
};

#endif // #ifndef _NUMUNO_HXX


