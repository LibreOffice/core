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


#include "streamwrap.hxx"
#include <osl/file.hxx>

namespace foo
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::osl;


//= OOutputStreamWrapper


void SAL_CALL OOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException, std::exception )
{
    sal_uInt64 nWritten = 0;
    rStream.write(aData.getConstArray(),aData.getLength(),nWritten);
    if (nWritten != (sal_uInt64)aData.getLength())
    {
        throw stario::BufferSizeExceededException(OUString(),static_cast<staruno::XWeak*>(this));
    }
}


void SAL_CALL OOutputStreamWrapper::flush() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException, std::exception )
{
}


void SAL_CALL OOutputStreamWrapper::closeOutput() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException, std::exception )
{
}

} // namespace utl


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
