/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_NUMVSET_HXX
#define _SVX_NUMVSET_HXX


// include ---------------------------------------------------------------


#include <svtools/valueset.hxx>
#include <limits.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include "svx/svxdllapi.h"

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

class SvxBrushItem;
class SvxNumRule;
struct SvxBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    sal_uInt16          nItemId;
};

class SVX_DLLPUBLIC SvxNumValueSet : public ValueSet
{
    Color           aLineColor;
    sal_uInt16          nPageType;
    sal_Bool            bHTMLMode;
    Rectangle       aOrgRect;
    VirtualDevice*  pVDev;

    com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> xFormatter;
    com::sun::star::lang::Locale aLocale;

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyValue> > aNumSettings;

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess> > aOutlineSettings;
    public:
        SvxNumValueSet( Window* pParent, const ResId& rResId, sal_uInt16 nType );
        ~SvxNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            SetHTMLMode(sal_Bool bSet) {bHTMLMode = bSet;}
    void            SetNumberingSettings(
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyValue> >& aNum,
        com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& xFormatter,
        const com::sun::star::lang::Locale& rLocale );

    void            SetOutlineNumberingSettings(
            com::sun::star::uno::Sequence<
                com::sun::star::uno::Reference<
                    com::sun::star::container::XIndexAccess> >& rOutline,
            com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& xFormatter,
            const com::sun::star::lang::Locale& rLocale);
};

class SVX_DLLPUBLIC SvxBmpNumValueSet : public SvxNumValueSet
{
    String      sBullets;
    Timer       aFormatTimer;
    sal_Bool        bGrfNotFound;

    protected:
        DECL_LINK(FormatHdl_Impl, Timer*);

    void            SetGrfNotFound(sal_Bool bSet) {bGrfNotFound = bSet;}
    sal_Bool            IsGrfNotFound()const {return bGrfNotFound;}

    Timer&          GetFormatTimer() {return aFormatTimer;}

    public:
        SvxBmpNumValueSet( Window* pParent, const ResId& rResId);
        ~SvxBmpNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

};

class SvxNumberingPreview : public Window
{
    const SvxNumRule*   pActNum;
    Font                aStdFont;
    long                nPageWidth;
    const String*       pOutlineNames;
    sal_Bool                bPosition;
    sal_uInt16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        SvxNumberingPreview(Window* pParent, const ResId& rResId ) :
            Window(pParent, rResId),
            pActNum(0),nPageWidth(0), pOutlineNames(0), bPosition(sal_False), nActLevel(USHRT_MAX)
            { SetBorderStyle( WINDOW_BORDER_MONO ); }

        void    SetNumRule(const SvxNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const String* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = sal_True;}
        void    SetLevel(sal_uInt16 nSet) {nActLevel = nSet;}

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
