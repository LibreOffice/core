/*************************************************************************
 *
 *  $RCSfile: hlnkitem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:56 $
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
#ifndef _SVX_HLNKITEM_HXX
#define _SVX_HLNKITEM_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

#define HYPERDLG_EVENT_MOUSEOVER_OBJECT     0x0001
#define HYPERDLG_EVENT_MOUSECLICK_OBJECT    0x0002
#define HYPERDLG_EVENT_MOUSEOUT_OBJECT      0x0004

enum SvxLinkInsertMode
{
    HLINK_DEFAULT,
    HLINK_FIELD,
    HLINK_BUTTON,
    HLINK_HTMLMODE = 0x0080
};

class SvxHyperlinkItem : public SfxPoolItem
{
    String sName;
    String sURL;
    String sTarget;
    SvxLinkInsertMode eType;

    String sIntName;
    SvxMacroTableDtor*  pMacroTable;

    USHORT nMacroEvents;

public:
    TYPEINFO();

    SvxHyperlinkItem( USHORT nWhich = SID_HYPERLINK_GETLINK ):
                SfxPoolItem(nWhich), pMacroTable(NULL)  { eType = HLINK_DEFAULT; nMacroEvents=0; };
    SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem );
    SvxHyperlinkItem( USHORT nWhich, String& rName, String& rURL,
                                    String& rTarget, String& rIntName,
                                    SvxLinkInsertMode eTyp = HLINK_FIELD,
                                    USHORT nEvents = 0,
                                    SvxMacroTableDtor *pMacroTbl =NULL );
    virtual ~SvxHyperlinkItem () { delete pMacroTable; }

    inline SvxHyperlinkItem& operator=( const SvxHyperlinkItem &rItem );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

    const   String& GetName() const { return sName; }
    void    SetName(const String& rName) { sName = rName; }

    const   String& GetURL() const { return sURL; }
    void    SetURL(const String& rURL) { sURL = rURL; }

    const   String& GetIntName () const { return sIntName; }
    void    SetIntName(const String& rIntName) { sIntName = rIntName; }

    const   String& GetTargetFrame() const { return sTarget; }
    void    SetTargetFrame(const String& rTarget) { sTarget = rTarget; }

    SvxLinkInsertMode GetInsertMode() const { return eType; }
    void    SetInsertMode( SvxLinkInsertMode eNew ) { eType = eNew; }

    void SetMacro( USHORT nEvent, const SvxMacro& rMacro );

    void SetMacroTable( const SvxMacroTableDtor& rTbl );
    const SvxMacroTableDtor* GetMacroTbl() const    { return pMacroTable; }

    void SetMacroEvents (const USHORT nEvents) { nMacroEvents = nEvents; }
    const   USHORT GetMacroEvents() const { return nMacroEvents; }

    virtual SvStream&           Store( SvStream &, USHORT nItemVersion ) const;
    virtual SfxPoolItem*        Create( SvStream &, USHORT nVer ) const;

};

#endif


