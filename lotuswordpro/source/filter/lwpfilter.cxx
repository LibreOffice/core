/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Circle object.
 ************************************************************************/
#include "lwpfilter.hxx"
#include "lwpresource.hxx"
#include <xfilter/xfsaxstream.hxx>
#include "lwp9reader.hxx"
#include <lwpsvstream.hxx>
#include <xfilter/xffontfactory.hxx>
#include <xfilter/xfstylemanager.hxx>

#include <osl/file.h>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/attrlist.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>

#include <cppuhelper/supportsservice.hxx>

#include <sfx2/docfile.hxx>

#include <memory>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

 /**
 * @descr   decompressed small file
 * @param   pCompressed - real file stream
 * @param   pDecompressed - file decompressed, create inside, caller should delete it
 * @return  success - sal_True, fail - sal_False
 */
#include "bento.hxx"
using namespace OpenStormBento;
#include "explode.hxx"
static bool Decompress(SvStream *pCompressed, SvStream * & pOutDecompressed)
{
    pCompressed->Seek(0);
    std::unique_ptr<SvMemoryStream> aDecompressed(new SvMemoryStream(4096, 4096));
    unsigned char buffer[512];
    pCompressed->ReadBytes(buffer, 16);
    aDecompressed->WriteBytes(buffer, 16);

    std::unique_ptr<LwpSvStream> aLwpStream(new LwpSvStream(pCompressed));
    std::unique_ptr<OpenStormBento::LtcBenContainer> pBentoContainer;
    {
        sal_uLong ulRet = BenOpenContainer(aLwpStream.get(), &pBentoContainer);
        if (ulRet != BenErr_OK)
            return false;
    }

    std::unique_ptr<LtcUtBenValueStream> aWordProData(pBentoContainer->FindValueStreamWithPropertyName("WordProData"));

    if (!aWordProData)
        return false;

    // decompressing
    Decompression decompress(aWordProData.get(), aDecompressed.get());
    if (0!= decompress.explode())
        return false;

    sal_uInt32 nPos = aWordProData->GetSize();
    nPos += 0x10;

    pCompressed->Seek(nPos);
    while (sal_uInt32 iRead = pCompressed->ReadBytes(buffer, 512))
        aDecompressed->WriteBytes(buffer, iRead);

    // disable stream growing past its current size
    aDecompressed->SetResizeOffset(0);

    //transfer ownership of aDecompressed's ptr
    pOutDecompressed = aDecompressed.release();
    return true;
}

 /**
 * @descr   Get LwpSvStream, if small file, both compressed/decompressed stream
 *                  Otherwise, only normal stream
 * @param   pStream - real file stream
 * @param    LwpSvStream * , created inside, deleted outside
 * @param      sal_Bool, sal_True -
 */
static bool GetLwpSvStream(SvStream *pStream, LwpSvStream * & pLwpSvStream)
{
    SvStream * pDecompressed = nullptr;

    pStream->Seek(0x10);
    sal_uInt32 nTag(0);
    pStream->ReadUInt32( nTag );
    if (nTag != 0x3750574c) // "LWP7"
    {
        // small file, needs decompression
        if (!Decompress(pStream, pDecompressed))
        {
            pLwpSvStream = nullptr;
            return true;
        }
        pStream->Seek(0);
        pDecompressed->Seek(0);
    }

    pLwpSvStream = nullptr;
    bool bCompressed = false;
    if (pDecompressed)
    {
        LwpSvStream *pOriginalLwpSvStream = new LwpSvStream(pStream);
        pLwpSvStream  = new LwpSvStream(pDecompressed, pOriginalLwpSvStream);
        bCompressed = true;
    }
    else
    {
        pLwpSvStream  = new LwpSvStream(pStream);
    }
    return bCompressed;
}
int ReadWordproFile(SvStream &rStream, uno::Reference<css::xml::sax::XDocumentHandler> const & xHandler)
{
    int nRet = 0;
    try
    {
        LwpSvStream *pRawLwpSvStream = nullptr;
        std::unique_ptr<LwpSvStream> aLwpSvStream;
        std::unique_ptr<LwpSvStream> aCompressedLwpSvStream;
        std::unique_ptr<SvStream> aDecompressed;
        if (GetLwpSvStream(&rStream, pRawLwpSvStream) && pRawLwpSvStream)
        {
            SvStream *pDecompressed = pRawLwpSvStream->GetStream();
            if (pDecompressed)
            {
                aDecompressed.reset(pDecompressed);
                aCompressedLwpSvStream.reset(pRawLwpSvStream->GetCompressedStream());
            }
        }

        if (!pRawLwpSvStream)
        {
            // nothing returned, fail when uncompressing
            return 1;
        }

        aLwpSvStream.reset(pRawLwpSvStream);

        std::unique_ptr<IXFStream> pStrm(new XFSaxStream(xHandler));
        Lwp9Reader reader(aLwpSvStream.get(), pStrm.get());
        //Reset all static objects,because this function may be called many times.
        XFGlobalReset();
        const bool bOk = reader.Read();
        if (!bOk)
            nRet = 1;
    }
    catch (...)
    {
        return 1;
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
