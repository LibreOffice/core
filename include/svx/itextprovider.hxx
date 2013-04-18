/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined SVX_ITEXTPROVIDER_HXX_INCLUDED
#define SVX_ITEXTPROVIDER_HXX_INCLUDED

#include <sal/types.h>

#include <svx/svxdllapi.h>

class SdrText;

namespace svx
{

    /** This interface provides access to text object(s) in an SdrObject.

     */
    class SVX_DLLPUBLIC ITextProvider
    {
    public:
        /** Return the number of texts available for this object. */
        virtual sal_Int32 getTextCount() const = 0;

        /** Return the nth available text. */
        virtual SdrText* getText(sal_Int32 nIndex) const = 0;

    protected:
        ~ITextProvider() {}
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
