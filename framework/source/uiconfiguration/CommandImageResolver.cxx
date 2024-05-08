/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CommandImageResolver.hxx"
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/urlobj.hxx>

using css::uno::Sequence;

namespace vcl
{

namespace
{

constexpr o3tl::enumarray<ImageType, OUString> ImageType_Prefixes
{
    u"cmd/sc_"_ustr,
    u"cmd/lc_"_ustr,
    u"cmd/32/"_ustr
};

OUString lclConvertToCanonicalName(const OUString& rFileName)
{
    bool bRemoveSlash(true);
    sal_Int32 nLength = rFileName.getLength();
    const sal_Unicode* pString = rFileName.getStr();

    OUStringBuffer aBuffer(nLength*2);

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

} // end anonymous namespace

CommandImageResolver::CommandImageResolver()
{
}

CommandImageResolver::~CommandImageResolver()
{
}

void CommandImageResolver::registerCommands(const Sequence<OUString>& aCommandSequence)
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
            INetURLObject aUrlObject(aCommandName, INetURLObject::EncodeMechanism::All);
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
        aImageName = aImageName.toAsciiLowerCase() + ".png";

        m_aImageNameVector[i] = aImageName;
        m_aCommandToImageNameMap[aCommandName] = aImageName;
    }
}

bool CommandImageResolver::hasImage(const OUString& rCommandURL)
{
    CommandToImageNameMap::const_iterator pIterator = m_aCommandToImageNameMap.find(rCommandURL);
    return pIterator != m_aCommandToImageNameMap.end();
}

ImageList* CommandImageResolver::getImageList(ImageType nImageType)
{
    const OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    if (sIconTheme != m_sIconTheme)
    {
        m_sIconTheme = sIconTheme;
        for (auto& rp : m_pImageList)
            rp.reset();
    }

    if (!m_pImageList[nImageType])
    {
        OUString sIconPath = ImageType_Prefixes[nImageType];
        m_pImageList[nImageType].reset( new ImageList(m_aImageNameVector, sIconPath) );
    }

    return m_pImageList[nImageType].get();
}

Image CommandImageResolver::getImageFromCommandURL(ImageType nImageType, const OUString& rCommandURL)
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
