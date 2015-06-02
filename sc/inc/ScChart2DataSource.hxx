/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INC_SCCHART2DATASOURCE_HXX_
#define INC_SCCHART2DATASOURCE_HXX_

#include "sal/types.h"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/chart2/data/XDataSource.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "cppuhelper/implbase2.hxx"
#include "rtl/ustring.hxx"
#include "svl/lstner.hxx"

#include <list>

class ScDocument;

// DataSource
class ScChart2DataSource : public
                ::cppu::WeakImplHelper2<
                    ::com::sun::star::chart2::data::XDataSource,
                    ::com::sun::star::lang::XServiceInfo>,
                SfxListener
{
public:
    explicit ScChart2DataSource( ScDocument* pDoc);
    virtual ~ScChart2DataSource();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    // XDataSource
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL
        getDataSequences() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(
            ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL supportsService( const OUString&
            rServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw(
                ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // implementation

    void AddLabeledSequence(const com::sun::star::uno::Reference < com::sun::star::chart2::data::XLabeledDataSequence >& xNew);

private:

    ScDocument*                 m_pDocument;
    typedef std::list < com::sun::star::uno::Reference< com::sun::star::chart2::data::XLabeledDataSequence > >  LabeledList;
    LabeledList                 m_aLabeledSequences;

};




#endif /* INC_SCCHART2DATASOURCE_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
