/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_COMMANDICONRESOLVER_HXX
#define INCLUDED_VCL_COMMANDICONRESOLVER_HXX

#include <vcl/dllapi.h>
#include <vcl/image.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <unordered_map>
#include <vector>

namespace vcl
{

enum ImageType
{
    ImageType_Color = 0,
    ImageType_Color_Large,
    ImageType_COUNT
};

class VCL_DLLPUBLIC CommandImageResolver
{
private:
    typedef std::unordered_map<OUString, OUString, OUStringHash, std::equal_to<OUString>> CommandToImageNameMap;

    CommandToImageNameMap m_aCommandToImageNameMap;
    std::vector<OUString> m_aImageCommandNameVector;
    std::vector<OUString> m_aImageNameVector;

    ImageList* m_pImageList[ImageType_COUNT];
    OUString m_sIconTheme;

    ImageList* getImageList(sal_Int16 nImageType);

public:
    CommandImageResolver();
    virtual ~CommandImageResolver();

    bool registerCommands(css::uno::Sequence<OUString>& aCommandSequence);
    Image getImageFromCommandURL(sal_Int16 nImageType, const OUString& rCommandURL);

    std::vector<OUString>& getCommandNames()
    {
        return m_aImageCommandNameVector;
    }

    bool hasImage(const OUString& rCommandURL);
};

} // end namespace vcl

#endif // INCLUDED_VCL_COMMANDICONRESOLVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
