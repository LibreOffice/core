/*************************************************************************
 *
 *  $RCSfile: bufferedfile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:17:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
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

#include <string.h>
#include "bufferedfile.hxx"

#include "filehelper.hxx"

#include <algorithm>


namespace configmgr
{

BufferedInputFile::BufferedInputFile( rtl::OUString const& aFileURL)
: m_aFileURL(aFileURL)
, m_pBuffer(0)
, m_nPointer(0)
, m_nSize(0)
{
}

/** D'tor
 */

BufferedInputFile::~BufferedInputFile()
{
    delete [] m_pBuffer;
}

BufferedInputFile::RC BufferedInputFile::open( sal_uInt32 uFlags )
{
    OSL_ENSURE(!m_pBuffer,"File already open");

    osl::File theFile(m_aFileURL);
    RC rc = theFile.open(uFlags);
    if (rc != E_None)
        return rc;

    sal_uInt64 theSize;
    rc = theFile.getSize(theSize);
    if (rc != E_None)
        return rc;

    // still prevent leaks in case of misuse
    delete [] m_pBuffer, m_pBuffer = 0;

    m_pBuffer = new sal_Int8[theSize];

    sal_uInt64 nReallyRead = 0;
    rc = theFile.read(m_pBuffer, theSize, nReallyRead);
    if (rc != E_None)
    {
        delete [] m_pBuffer, m_pBuffer = 0;
        return rc;
    }

    OSL_ENSURE(nReallyRead == m_nSize, "Error, read file can't get it's complete file data");
    m_nSize = nReallyRead;
    m_nPointer = 0;

    return E_None;
}

BufferedInputFile::RC BufferedInputFile::close()
{
    if (m_pBuffer == 0)
        return E_BADF;

    delete [] m_pBuffer, m_pBuffer = 0;
    m_nSize = 0;
    return E_None;
}

BufferedInputFile::RC BufferedInputFile::setPos( sal_uInt32 uHow, sal_uInt64 uPos )
{
    if (m_pBuffer == 0)
        return E_BADF;

    switch (uHow)
    {
    case osl_Pos_Absolut:
        m_nPointer = uPos;
        break;
    case osl_Pos_Current:
        m_nPointer += uPos;
        break;
    case osl_Pos_End:
        m_nPointer = m_nSize + uPos;
        break;
    default:
        return E_INVAL;
    }
    return E_None;
}

BufferedInputFile::RC BufferedInputFile::getPos( sal_uInt64& uPos )
{
    if (m_pBuffer == 0)
        return E_BADF;

    uPos = m_nPointer;
    return E_None;
}

BufferedInputFile::RC BufferedInputFile::available( sal_uInt64& nAvail) const
{
    if (m_pBuffer == 0)
        return E_BADF;

    if (m_nPointer < m_nSize)
        nAvail = m_nSize-m_nPointer;
    else
        nAvail = 0;

    return E_None;
}

BufferedInputFile::RC BufferedInputFile::read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
{
    if (m_pBuffer == 0)
        return E_BADF;

    if (m_nPointer < m_nSize)
    {
        // requested size may be greater than the real file size
        rBytesRead = std::min(m_nSize - m_nPointer, uBytesRequested);

        memcpy(pBuffer, m_pBuffer + m_nPointer, rBytesRead);
        m_nPointer += rBytesRead;
    }
    else
    {
        // EOF
        rBytesRead = 0;
    }
    return E_None;
}

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
        m_buffer.erase(m_buffer.begin(),m_buffer.begin()+written);

        return E_IO;
    }

    m_buffer.clear();

    return E_None;
}

} // namespace configmgr

