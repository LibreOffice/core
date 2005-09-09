/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numprevw.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:53:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _NUMPREVW_HXX
#define _NUMPREVW_HXX


#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

class SwNumRule;
/*-----------------02.12.97 10:31-------------------

--------------------------------------------------*/

class NumberingPreview : public Window
{
    const SwNumRule*    pActNum;
    Font                aStdFont;
    long                nPageWidth;
    const String*       pOutlineNames;
    BOOL                bPosition;
    UINT16              nActLevel;

    protected:
        virtual void        Paint( const Rectangle& rRect );

    public:
        NumberingPreview(Window* pParent, const ResId& rResId ) :
            Window(pParent, rResId),
            pActNum(0),nPageWidth(0), pOutlineNames(0), bPosition(FALSE), nActLevel(USHRT_MAX) {}
        ~NumberingPreview();

        void    SetNumRule(const SwNumRule* pNum)
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


