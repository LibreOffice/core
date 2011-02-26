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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include "modsizeexceeded.hxx"

#include <framework/interaction.hxx>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

using namespace com::sun::star;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;

ModuleSizeExceeded::ModuleSizeExceeded( const uno::Sequence< ::rtl::OUString >& sModules )
{
    script::ModuleSizeExceededRequest aReq;
    aReq.Names = sModules;

    m_aRequest <<= aReq;

    m_xAbort.set( uno::Reference< task::XInteractionAbort >(new framework::ContinuationAbort), uno::UNO_QUERY );
    m_xApprove.set( uno::Reference< task::XInteractionApprove >(new framework::ContinuationApprove ), uno::UNO_QUERY );
    m_lContinuations.realloc( 2 );
    m_lContinuations[0] =  m_xApprove;
    m_lContinuations[1] = m_xAbort;
}

sal_Bool
ModuleSizeExceeded::isAbort() const
{
    framework::ContinuationAbort* pBase = static_cast< framework::ContinuationAbort* >( m_xAbort.get() );
    return pBase->isSelected();
}

sal_Bool
ModuleSizeExceeded::isApprove() const
{
    framework::ContinuationApprove* pBase = static_cast< framework::ContinuationApprove* >( m_xApprove.get() );
    return pBase->isSelected();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
