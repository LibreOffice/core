/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TOXHLP_HXX
#define _TOXHLP_HXX

#include <tools/solar.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.hxx>


namespace drafts { namespace com { namespace sun { namespace star {
namespace i18n {
    class XExtendedIndexEntrySupplier;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}};
namespace binfilter {



 class IndexEntrySupplierWrapper
 {
    STAR_NMSPC::lang::Locale aLcl;
     STAR_NMSPC::uno::Reference < ::com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;
 
 public:
    IndexEntrySupplierWrapper(
            const STAR_NMSPC::lang::Locale& rLcl,
            STAR_REFERENCE( lang::XMultiServiceFactory )& rxMSF );
    ~IndexEntrySupplierWrapper();
 
     STAR_NMSPC::uno::Sequence< ::rtl::OUString >
    GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const;
 
     sal_Bool LoadAlgorithm( const STAR_NMSPC::lang::Locale& rLcl,
                             const String& sSortAlgorithm, long nOptions ) const;
 
     sal_Int16 CompareIndexEntry( const String& rTxt1, const String& rTxtReading1,
                                  const STAR_NMSPC::lang::Locale& rLcl1,
                                  const String& rTxt2, const String& rTxtReading2,
                                  const STAR_NMSPC::lang::Locale& rLcl2 ) const;
 };

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
