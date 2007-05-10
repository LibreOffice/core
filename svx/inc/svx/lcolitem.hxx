/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lcolitem.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 14:28:13 $
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
#ifndef _SVX_LCOLITEM_HXX
#define _SVX_LCOLITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

/*
 * dummy-Item fuer ToolBox-Controller:
 *
 */

class SvxLineColorItem : public SvxColorItem
{
public:
    TYPEINFO();

    SvxLineColorItem( const USHORT nId  );
    SvxLineColorItem( const Color& aColor, const USHORT nId  );
    SvxLineColorItem( SvStream& rStrm, const USHORT nId  );
    SvxLineColorItem( const SvxLineColorItem& rCopy );
    ~SvxLineColorItem();

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * pIntlWrapper = 0 )
        const;

    inline SvxLineColorItem& operator=(const SvxLineColorItem& rColor)
        { return (SvxLineColorItem&)SvxColorItem::
            operator=((const SvxColorItem&)rColor); }

};

#endif

