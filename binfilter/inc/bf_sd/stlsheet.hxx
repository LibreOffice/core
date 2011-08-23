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

#ifndef _SD_STLSHEET_HXX
#define _SD_STLSHEET_HXX


#ifndef _SFXSTYLE_HXX //autogen
#include <bf_svtools/style.hxx>
#endif
namespace binfilter {
class SdStyleSheet : public SfxStyleSheet
{
protected:
    virtual void Load (SvStream& rIn, USHORT nVersion);
    virtual void Store(SvStream& rOut);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType);

public:
                        TYPEINFO();
                        SdStyleSheet(const String& rName,
                                     SfxStyleSheetBasePool& rPool,
                                     SfxStyleFamily eFamily,
                                     USHORT nMask);
    virtual 			~SdStyleSheet();

    virtual BOOL		SetParent (const String& rParentName);
    virtual SfxItemSet& GetItemSet();
    virtual BOOL		IsUsed() const;



    SdStyleSheet* GetRealStyleSheet() const;
};

} //namespace binfilter
#endif	   // _SD_STLSHEET_HXX



