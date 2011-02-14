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

#ifndef _SVX_XEXCH_HXX
#define _SVX_XEXCH_HXX


#include <tools/stream.hxx>
#include "svx/svxdllapi.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                Clipboard-Format fuer XOutDev-Fuellattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class XFillAttrSetItem;
class SfxItemPool;



class SVX_DLLPUBLIC XFillExchangeData
{
private:
    XFillAttrSetItem*   pXFillAttrSetItem;
    SfxItemPool*        pPool;

public:
                        TYPEINFO();

                        XFillExchangeData();
                        XFillExchangeData(const XFillAttrSetItem rXFillAttrSetItem);
                        virtual ~XFillExchangeData();

                        // Zuweisungsoperator
                        XFillExchangeData& operator=( const XFillExchangeData& rXFillExchangeData );

    SVX_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStm, const XFillExchangeData& rXFillExchangeData );
    SVX_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStm, XFillExchangeData& rXFillExchangeData );

    static sal_uLong        RegisterClipboardFormatName();
    XFillAttrSetItem*   GetXFillAttrSetItem() { return pXFillAttrSetItem; }
};

#endif                  // _SVX_XEXCH_HXX
