/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/CommandImageResolver.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>
#include <svtools/miscopt.hxx>
#include <officecfg/Office/Common.hxx>

using css::uno::Sequence;

namespace vcl
{

namespace
{

const char* ImageType_Prefixes[ImageType_COUNT] =
{
    "cmd/sc_",
    "cmd/lc_"
};

OUString lclConvertToCanonicalName(const OUString& rFileName)
{
    bool bRemoveSlash(true);
    sal_Int32 nLength = rFileName.getLength();
    const sal_Unicode* pString = rFileName.getStr();

    OUStringBuffer aBuffer(nLength);

    for (sal_Int32 i = 0; i < nLength; i++)
    {
        const sal_Unicode cCurrentChar = pString[i];
        switch (cCurrentChar)
        {
            // map forbidden characters to escape
            case '/':
                if (!bRemoveSlash)
                    aBuffer.append("%2f");
                break;
            case '\\': aBuffer.append("%5c"); bRemoveSlash = false; break;
            case ':':  aBuffer.append("%3a"); bRemoveSlash = false; break;
            case '*':  aBuffer.append("%2a"); bRemoveSlash = false; break;
            case '?':  aBuffer.append("%3f"); bRemoveSlash = false; break;
            case '<':  aBuffer.append("%3c"); bRemoveSlash = false; break;
            case '>':  aBuffer.append("%3e"); bRemoveSlash = false; break;
            case '|':  aBuffer.append("%7c"); bRemoveSlash = false; break;
            default:
                aBuffer.append(cCurrentChar); bRemoveSlash = false; break;
        }
    }
    return aBuffer.makeStringAndClear();
}

} // end anonymouse namespace

CommandImageResolver::CommandImageResolver()
{
    for (sal_Int32 n = 0; n < ImageType_COUNT; n++)
        m_pImageList[n] = nullptr;
}

CommandImageResolver::~CommandImageResolver()
{
    for (sal_Int32 n = 0; n < ImageType_COUNT; n++)
        delete m_pImageList[n];
}

bool CommandImageResolver::registerCommands(Sequence<OUString>& aCommandSequence)
{
    sal_Int32 nSequenceSize = aCommandSequence.getLength();

    m_aImageCommandNameVector.resize(nSequenceSize);
    m_aImageNameVector.resize(nSequenceSize);

    for (sal_Int32 i = 0; i < nSequenceSize; ++i)
    {
        OUString aCommandName(aCommandSequence[i]);
        OUString aImageName;

        m_aImageCommandNameVector[i] = aCommandName;

        if (aCommandName.indexOf(".uno:") != 0)
        {
            INetURLObject aUrlObject(aCommandName, INetURLObject::ENCODE_ALL);
            aImageName = aUrlObject.GetURLPath();
            aImageName = lclConvertToCanonicalName(aImageName);
        }
        else
        {
            // just remove the schema
            if (aCommandName.getLength() > 5)
                aImageName = aCommandName.copy(5);

            // Search for query part.
            if (aImageName.indexOf('?') != -1)
                aImageName = lclConvertToCanonicalName(aImageName);
        }

        // Image names are not case-dependent. Always use lower case characters to
        // reflect this.
        aImageName = aImageName.toAsciiLowerCase();
        aImageName += ".png";

        m_aImageNameVector[i] = aImageName;
        m_aCommandToImageNameMap[aCommandName] = aImageName;
    }
    return true;
}

bool CommandImageResolver::hasImage(const OUString& rCommandURL)
{
    CommandToImageNameMap::const_iterator pIterator = m_aCommandToImageNameMap.find(rCommandURL);
    return pIterator != m_aCommandToImageNameMap.end();
}

ImageList* CommandImageResolver::getImageList(sal_Int16 nImageType)
{
    const OUString& rIconTheme = officecfg::Office::Common::Misc::SymbolStyle::get();

    if (rIconTheme != m_sIconTheme)
    {
        m_sIconTheme = rIconTheme;
        for (sal_Int32 n = 0; n < ImageType_COUNT; ++n)
        {
            delete m_pImageList[n];
            m_pImageList[n] = nullptr;
        }
    }

    if (!m_pImageList[nImageType])
    {
        OUString sIconPath = OUString::createFromAscii(ImageType_Prefixes[nImageType]);
        m_pImageList[nImageType] = new ImageList(m_aImageNameVector, sIconPath);
    }

    return m_pImageList[nImageType];
}

Image CommandImageResolver::getImageFromCommandURL(sal_Int16 nImageType, const OUString& rCommandURL)
{
    CommandToImageNameMap::const_iterator pIterator = m_aCommandToImageNameMap.find(rCommandURL);
    if (pIterator != m_aCommandToImageNameMap.end())
    {
        ImageList* pImageList = getImageList(nImageType);
        return pImageList->GetImage(pIterator->second);
    }
    return Image();
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
