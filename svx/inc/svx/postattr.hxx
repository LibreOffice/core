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
#ifndef _SVX_POSTATTR_HXX
#define _SVX_POSTATTR_HXX

#include <svl/stritem.hxx>
#include "svx/svxdllapi.h"

// class SvxPostItAuthorItem ---------------------------------------------



/*
The author shorthand symbol of a note
*/

class SVX_DLLPUBLIC SvxPostItAuthorItem: public SfxStringItem
{
public:
    TYPEINFO();

    SvxPostItAuthorItem( sal_uInt16 nWhich  );

    SvxPostItAuthorItem( const String& rAuthor, sal_uInt16 nWhich  );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    inline SvxPostItAuthorItem& operator=( const SvxPostItAuthorItem& rAuthor )
    {
        SetValue( rAuthor.GetValue() );
        return *this;
    }
};


// class SvxPostItDateItem -----------------------------------------------



/*
The date of a note
*/

class SVX_DLLPUBLIC SvxPostItDateItem: public SfxStringItem
{
public:
    TYPEINFO();

    SvxPostItDateItem( sal_uInt16 nWhich  );

    SvxPostItDateItem( const String& rDate, sal_uInt16 nWhich  );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    inline SvxPostItDateItem& operator=( const SvxPostItDateItem& rDate )
    {
        SetValue( rDate.GetValue() );
        return *this;
    }
};


// class SvxPostItTextItem -----------------------------------------------



/*
The text of a note
*/

class SVX_DLLPUBLIC SvxPostItTextItem: public SfxStringItem
{
public:
    TYPEINFO();

    SvxPostItTextItem( sal_uInt16 nWhich  );

    SvxPostItTextItem( const String& rText, sal_uInt16 nWhich  );
    // "pure virtual methods" from SfxPoolItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    inline SvxPostItTextItem& operator=( const SvxPostItTextItem& rText )
    {
        SetValue( rText.GetValue() );
        return *this;
    }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
