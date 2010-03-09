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
#include "precompiled_configmgr.hxx"

#include <string.h>
#include "bufferedfile.hxx"

#include "filehelper.hxx"

#include <algorithm>


namespace configmgr
{

BufferedOutputFile::BufferedOutputFile( rtl::OUString const& aFileURL, sal_uInt32 nBufferSizeHint )
: m_pFile(new osl::File(aFileURL))
, m_buffer()
{
    m_buffer.reserve( nBufferSizeHint ? nBufferSizeHint : 512 );
}

BufferedOutputFile::~BufferedOutputFile ()
{
    delete m_pFile;
}


BufferedOutputFile::RC BufferedOutputFile::open( sal_uInt32 uFlags )
{
    if (!m_pFile) return E_BADF;

    return m_pFile->open(uFlags);
}

BufferedOutputFile::RC BufferedOutputFile::close()
{
    if (!m_pFile) return E_BADF;

    RC rc = this->sync();
    RC rc2 = m_pFile->close();

    delete m_pFile, m_pFile = 0;
    if (rc == E_None)
        rc = rc2;
    return rc;
}


//BufferedOutputFile::RC BufferedOutputFile::getPos( sal_uInt64& uPos )

BufferedOutputFile::RC BufferedOutputFile::write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
{
    if (!m_pFile) return E_BADF;

    if (uBytesToWrite > m_buffer.max_size()-m_buffer.size())
    {
        // write big chunks natively
        RC rc = this->sync();
        if (rc == E_None)
        {
            OSL_ASSERT(m_buffer.empty());
            rc = m_pFile->write(pBuffer, uBytesToWrite, rBytesWritten);
        }
        return rc;
    }

    // FIXME: handle out-out-memory here
    const sal_uInt8 * data = static_cast<const sal_uInt8 *>(pBuffer);
    m_buffer.insert(m_buffer.end(), data, data + uBytesToWrite);
    rBytesWritten = uBytesToWrite;

    return E_None;
}

BufferedOutputFile::RC BufferedOutputFile::sync()
{
    if (!m_pFile) return E_BADF;

    sal_uInt64 size = m_buffer.size();
    sal_uInt64 written = 0;

    RC rc = m_pFile->write(&m_buffer.front(),size,written);

    if (rc != E_None)
        return rc;

    // we don't support special files where multiple write passes are needed
    if (written < size)
    {
        // but we try our best to stay consistent
        m_buffer.erase(m_buffer.begin(),
                       m_buffer.end() + sal::static_int_cast<sal_uInt32>( written ));

        return E_IO;
    }

    m_buffer.clear();

    return E_None;
}

} // namespace configmgr

