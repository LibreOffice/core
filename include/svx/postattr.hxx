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

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svx/svxdllapi.h>

/** The author shorthand symbol of a note
 */

class SVX_DLLPUBLIC SvxPostItAuthorItem: public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SvxPostItAuthorItem( sal_uInt16 nWhich  );

    SvxPostItAuthorItem( const OUString& rAuthor, sal_uInt16 nWhich  );
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
};


/** The date of a note
 */

class SVX_DLLPUBLIC SvxPostItDateItem: public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SvxPostItDateItem( sal_uInt16 nWhich  );

    SvxPostItDateItem( const OUString& rDate, sal_uInt16 nWhich  );
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
};

/** The text of a note
 */

class SVX_DLLPUBLIC SvxPostItTextItem: public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SvxPostItTextItem( sal_uInt16 nWhich  );

    SvxPostItTextItem( const OUString& rText, sal_uInt16 nWhich  );
    // "pure virtual methods" from SfxPoolItem
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
};


/** The internal id of a note
 */

class SVX_DLLPUBLIC SvxPostItIdItem: public SfxStringItem
{
public:
    static SfxPoolItem* CreateDefault();

    SvxPostItIdItem( sal_uInt16 nWhich );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
