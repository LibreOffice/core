/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formatclipboard.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SDFORMATCLIPBOARD_HXX
#define _SDFORMATCLIPBOARD_HXX

#include "View.hxx"
// header for class SfxItemSet
#include <svl/itemset.hxx>

//-----------------------------------------------------------------------------
/** This class acts as data container and execution class for the format paintbrush feature in draw and impress.
*/

class SdFormatClipboard
{
public:
    SdFormatClipboard();
    virtual ~SdFormatClipboard();

    bool HasContent() const;
    bool CanCopyThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier ) const;
    bool HasContentForThisType( UINT32 nObjectInventor, UINT16 nObjectIdentifier ) const;

    void Copy( ::sd::View& rDrawView, bool bPersistentCopy=false );
    void Paste( ::sd::View& rDrawView
        , bool bNoCharacterFormats=false, bool bNoParagraphFormats=false );
    void Erase();

private:
    SfxItemSet* m_pItemSet;
    bool   m_bPersistentCopy;
    UINT32 m_nType_Inventor;
    UINT16 m_nType_Identifier;
};

#endif
