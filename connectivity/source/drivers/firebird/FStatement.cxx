/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include "FStatement.hxx"
#include "FConnection.hxx"
#include "FResultSet.hxx"
#include "Util.hxx"

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace connectivity::firebird;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::std;

// ---- XBatchExecution - UNSUPPORTED ----------------------------------------
void SAL_CALL OStatement::addBatch(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    (void) sql;
}

void SAL_CALL OStatement::clearBatch() throw(SQLException, RuntimeException)
{
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch() throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 >();
}

IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");

void SAL_CALL OStatement::acquire() throw()
{
    OStatement_Base::acquire();
}

void SAL_CALL OStatement::release() throw()
{
    OStatement_Base::release();
}

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatement_Base::queryInterface(rType);
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
