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
#ifndef INCLUDED_SVX_POSTATTR_HXX
#define INCLUDED_SVX_POSTATTR_HXX

#include <svl/stritem.hxx>
#include <svx/svxdllapi.h>

/** The author shorthand symbol of a note
 */

class SVX_DLLPUBLIC SvxPostItAuthorItem final : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxPostItAuthorItem)
    SvxPostItAuthorItem( TypedWhichId<SvxPostItAuthorItem> nWhich  );

    SvxPostItAuthorItem( const OUString& rAuthor, TypedWhichId<SvxPostItAuthorItem> nWhich  );
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxPostItAuthorItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};


/** The date of a note
 */

class SVX_DLLPUBLIC SvxPostItDateItem final : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxPostItDateItem)
    SvxPostItDateItem( TypedWhichId<SvxPostItDateItem> nWhich  );

    SvxPostItDateItem( const OUString& rDate, TypedWhichId<SvxPostItDateItem> nWhich  );
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxPostItDateItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};

/** The text of a note
 */

class SVX_DLLPUBLIC SvxPostItTextItem final : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxPostItTextItem)
    SvxPostItTextItem( TypedWhichId<SvxPostItTextItem> nWhich  );

    SvxPostItTextItem( const OUString& rText, TypedWhichId<SvxPostItTextItem> nWhich  );
    // "pure virtual methods" from SfxPoolItem
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxPostItTextItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};


/** The internal id of a note
 */

class SVX_DLLPUBLIC SvxPostItIdItem final : public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxPostItIdItem)
    SvxPostItIdItem( TypedWhichId<SvxPostItIdItem>  nWhich );

    virtual SvxPostItIdItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
