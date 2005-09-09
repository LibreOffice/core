/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formatclipboard.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:29:18 $
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

#ifndef _SDFORMATCLIPBOARD_HXX
#define _SDFORMATCLIPBOARD_HXX

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

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
