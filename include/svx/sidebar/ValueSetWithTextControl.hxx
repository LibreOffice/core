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
#ifndef _SVX_SIDEBAR_VALUESETWITHTEXT_CONTROL_HXX_
#define _SVX_SIDEBAR_VALUESETWITHTEXT_CONTROL_HXX_

#include "svx/svxdllapi.h"

#include <svtools/valueset.hxx>
#include <limits.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>

#include <vcl/image.hxx>

#include <vector>

namespace com{namespace sun{ namespace star{
    namespace container{
        class XIndexAccess;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace text{
        class XNumberingFormatter;
    }
}}}

namespace svx { namespace sidebar {

/** Specialization of class <ValueSet>.
    This specialization allows is a one-columned ValueSet which allow
    items containing an image and a text or a text and a second text.

    Especially, used for sidebar related controls.
*/
class SVX_DLLPUBLIC ValueSetWithTextControl : public ValueSet
{
public:
    // control type of specialized <ValueSet>:
    // - image + text
    // - text + text
    enum tControlType
    {
        IMAGE_TEXT,
        TEXT_TEXT
    };

    ValueSetWithTextControl(
        const tControlType eControlType,
        Window* pParent,
        const ResId& rResId);

    virtual ~ValueSetWithTextControl(void);

    // add item for control type IMAGE_TEXT
    // if control type does not match IMAGE_TEXT no item is added.
    // @param pSelectedItemImage
    // selection item image is optional. if not provided, it is the same as the image item
    // @param pItemHelpText
    // help text is optional. if not provided, it is the same as the item text
    void AddItem(
        const Image& rItemImage,
        const Image* pSelectedItemImage,
        const OUString& rItemText,
        const OUString* pItemHelpText );

    // replace item images for control type IMAGE_TEXT
    void ReplaceItemImages(
        const sal_uInt16 nItemId,
        const Image& rItemImage,
        const Image* pSelectedItemImage );

    // add item for control type TEXT_TEXT
    // if control type does not match TEXT_TEXT no item is added.
    // @param pItemHelpText
    // help text is optional. if not provided, it is the same as the item text
    void AddItem(
        const OUString& rItemText,
        const OUString& rItemText2,
        const OUString* pItemHelpText );

    virtual void UserDraw( const UserDrawEvent& rUDEvt );

private:
    struct ValueSetWithTextItem
    {
        Image maItemImage;
        Image maSelectedItemImage;
        OUString maItemText;
        OUString maItemText2;
    };

    typedef ::std::vector< ValueSetWithTextItem > tItemList;

    const tControlType meControlType;
    tItemList maItems;
};

class SVX_DLLPUBLIC SvxNumValueSet2 : public ValueSet
{
    Color           aLineColor;
    Rectangle       aOrgRect;
    VirtualDevice*  pVDev;

    com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> xFormatter;
    com::sun::star::lang::Locale aLocale;

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyValue> > aNumSettings;


    public:
        SvxNumValueSet2( Window* pParent, const ResId& rResId);
        ~SvxNumValueSet2();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );


    void            SetNumberingSettings(
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyValue> >& aNum,
        com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& xFormatter,
        const com::sun::star::lang::Locale& rLocale );
};

class SVX_DLLPUBLIC SvxNumValueSet3 : public ValueSet
{
    public:
        SvxNumValueSet3( Window* pParent, const ResId& rResId);
        ~SvxNumValueSet3();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

};

} } // end of namespace svx::sidebar

#endif
