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

#ifndef _SVX_XSFLCLIT_HXX
#define _SVX_XSFLCLIT_HXX

#include <svx/xcolit.hxx>

//------------------------------
// class XSecondaryFillColorItem
//------------------------------
class SVX_DLLPUBLIC XSecondaryFillColorItem : public XColorItem
{
public:
            TYPEINFO();
            SVX_DLLPRIVATE XSecondaryFillColorItem() {}
            SVX_DLLPRIVATE XSecondaryFillColorItem(sal_Int32 nIndex, const Color& rTheColor);
            XSecondaryFillColorItem(const String& rName, const Color& rTheColor);
            SVX_DLLPRIVATE XSecondaryFillColorItem(SvStream& rIn);

    SVX_DLLPRIVATE virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion( sal_uInt16 nFileFormatVersion ) const;

};

#endif

