/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

/*-----------------09/09/97 01:05pm-----------------

--------------------------------------------------*/
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

/*-----------------13.02.97 09.32-------------------

--------------------------------------------------*/

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
        SvxBmpNumValueSet( Window* pParent, const ResId& rResId/*, const List& rStrNames*/);
        ~SvxBmpNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

};
/*-----------------02.12.97 10:31-------------------

--------------------------------------------------*/

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

