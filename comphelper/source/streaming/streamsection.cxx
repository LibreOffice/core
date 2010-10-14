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
#include "precompiled_comphelper.hxx"
#include <comphelper/streamsection.hxx>
#include <osl/diagnose.h>

namespace comphelper
{

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataInputStream >& _rxInput)
    :m_xMarkStream(_rxInput, ::com::sun::star::uno::UNO_QUERY)
    ,m_xInStream(_rxInput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xInStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xInStream.is() && m_xMarkStream.is())
    {
        m_nBlockLen = _rxInput->readLong();
        m_nBlockStart = m_xMarkStream->createMark();
    }
}

//-------------------------------------------------------------------------
OStreamSection::OStreamSection(const staruno::Reference< stario::XDataOutputStream >& _rxOutput, sal_Int32 _nPresumedLength)
    :m_xMarkStream(_rxOutput, ::com::sun::star::uno::UNO_QUERY)
    ,m_xOutStream(_rxOutput)
    ,m_nBlockStart(-1)
    ,m_nBlockLen(-1)
{
    OSL_ENSURE(m_xOutStream.is() && m_xMarkStream.is(), "OStreamSection::OStreamSection : invalid argument !");
    if (m_xOutStream.is() && m_xMarkStream.is())
    {
        m_nBlockStart = m_xMarkStream->createMark();
        // a placeholder where we will write the overall length (within the destructor)
        if (_nPresumedLength > 0)
            m_nBlockLen = _nPresumedLength + sizeof(m_nBlockLen);
            // as the caller did not consider - of course - the placeholder we are going to write
        else
            m_nBlockLen = 0;
        m_xOutStream->writeLong(m_nBlockLen);
    }
}

//-------------------------------------------------------------------------
OStreamSection::~OStreamSection()
{
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        // handling routing
        if (m_xInStream.is() &&  m_xMarkStream.is())
        {   // we're working on an input stream
            m_xMarkStream->jumpToMark(m_nBlockStart);
            m_xInStream->skipBytes(m_nBlockLen);
            m_xMarkStream->deleteMark(m_nBlockStart);
        }
        else if (m_xOutStream.is() && m_xMarkStream.is())
        {
            sal_Int32 nRealBlockLength = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
            if (m_nBlockLen && (m_nBlockLen == nRealBlockLength))
                // nothing to do : the estimation the caller gave us (in the ctor) was correct
                m_xMarkStream->deleteMark(m_nBlockStart);
            else
            {   // the estimation was wrong (or we didn't get one)
                m_nBlockLen = nRealBlockLength;
                m_xMarkStream->jumpToMark(m_nBlockStart);
                m_xOutStream->writeLong(m_nBlockLen);
                m_xMarkStream->jumpToFurthest();
                m_xMarkStream->deleteMark(m_nBlockStart);
            }
        }
    }
    catch(const staruno::Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
sal_Int32 OStreamSection::available()
{
    sal_Int32 nBytes = 0;
    try
    {   // don't allow any exceptions to leave this block, this may be called during the stack unwinding of an exception
        if (m_xInStream.is() &&  m_xMarkStream.is())
            nBytes = m_xMarkStream->offsetToMark(m_nBlockStart) - sizeof(m_nBlockLen);
    }
    catch(const staruno::Exception&)
    {
    }
    return nBytes;
}
// -----------------------------------------------------------------------------

}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
