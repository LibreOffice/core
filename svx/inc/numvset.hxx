/*************************************************************************
 *
 *  $RCSfile: numvset.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_NUMVSET_HXX
#define _SVX_NUMVSET_HXX


// include ---------------------------------------------------------------


#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#include <limits.h>
class SvxBrushItem;
class SvxNumRule;
/*-----------------13.02.97 14.02-------------------

--------------------------------------------------*/

struct SvxBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    USHORT          nItemId;
};

/*-----------------09/09/97 01:05pm-----------------

--------------------------------------------------*/
class SvxNumValueSet : public ValueSet
{
    Color           aLineColor;
    USHORT          nPageType;
    BOOL            bHTMLMode;
    Rectangle       aOrgRect;
    VirtualDevice*  pVDev;
    public:
        SvxNumValueSet( Window* pParent, const ResId& rResId, USHORT nType );
        ~SvxNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            SetHTMLMode(BOOL bSet) {bHTMLMode = bSet;}

};

/*-----------------13.02.97 09.32-------------------

--------------------------------------------------*/

class SvxBmpNumValueSet : public SvxNumValueSet
{
    const List& rStrList;
    List        aGrfBrushItems;
    Timer       aFormatTimer;
    BOOL        bGrfNotFound;

    protected:
        DECL_STATIC_LINK(SvxBmpNumValueSet, GraphicArrivedHdl_Impl, SvxBrushItem*);
        DECL_LINK(FormatHdl_Impl, Timer*);

    void            SetGrfNotFound(BOOL bSet) {bGrfNotFound = bSet;}
    BOOL            IsGrfNotFound()const {return bGrfNotFound;}

    const List&     GetStringList(){return rStrList;}
    List&           GetGrfBrushItems() {return aGrfBrushItems;}

    Timer&          GetFormatTimer() {return aFormatTimer;}

    public:
        SvxBmpNumValueSet( Window* pParent, const ResId& rResId, const List& rStrNames);
        ~SvxBmpNumValueSet();

    SvxBmpItemInfo* FindInfo(USHORT nInfo);
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
    BOOL                bPosition;
    UINT16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        SvxNumberingPreview(Window* pParent, const ResId& rResId ) :
            Window(pParent, rResId),
            pActNum(0),nPageWidth(0), pOutlineNames(0), bPosition(FALSE), nActLevel(USHRT_MAX) {}

        void    SetNumRule(const SvxNumRule* pNum)
                    {pActNum = pNum; Invalidate();};
        void    SetPageWidth(long nPgWidth)
                                {nPageWidth = nPgWidth;}
        void    SetOutlineNames(const String* pNames)
                        {pOutlineNames = pNames;}
        void    SetPositionMode()
                        { bPosition = TRUE;}
        void    SetLevel(USHORT nSet) {nActLevel = nSet;}

};


#endif

