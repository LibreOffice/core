/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:25:56 $
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

#ifndef _EDITOBJ_HXX
#define _EDITOBJ_HXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

DBG_NAMEEX( EE_EditTextObject )

class SfxItemPool;
class SfxStyleSheetPool;
class SvxFieldItem;
class EECharAttribArray;

#define EDTOBJ_SETTINGS_ULITEMSUMMATION     0x00000001
#define EDTOBJ_SETTINGS_ULITEMFIRSTPARA     0x00000002

class SVX_DLLPUBLIC EditTextObject
{
private:
    USHORT              nWhich;
    SVX_DLLPRIVATE EditTextObject&      operator=( const EditTextObject& );

protected:
                        EditTextObject( USHORT nWhich );
                        EditTextObject( const EditTextObject& r );

    virtual void        StoreData( SvStream& rOStream ) const;
    virtual void        CreateData( SvStream& rIStream );

public:
    virtual             ~EditTextObject();

    USHORT              Which() const { return nWhich; }

    virtual USHORT      GetUserType() const;    // Fuer OutlinerMode, der kann das aber nicht kompatibel speichern
    virtual void        SetUserType( USHORT n );

    virtual ULONG       GetObjectSettings() const;
    virtual void        SetObjectSettings( ULONG n );

    virtual BOOL        IsVertical() const;
    virtual void        SetVertical( BOOL bVertical );

    virtual USHORT      GetScriptType() const;

    virtual USHORT      GetVersion() const; // Solange der Outliner keine Recordlaenge speichert

    virtual EditTextObject* Clone() const = 0;

    BOOL                    Store( SvStream& rOStream ) const;
    static EditTextObject*  Create( SvStream& rIStream,
                                SfxItemPool* pGlobalTextObjectPool = 0 );
    void                    Skip( SvStream& rIStream );

    // Zur 5.1 hat sich die Bedeutung des LRSpaceItems fuer den Outliner geaendert...
    virtual void            SetLRSpaceItemFlags( BOOL bOutlineMode );
    virtual void            AdjustImportedLRSpaceItems( BOOL bTurnOfBullets );
/* cl removed because not needed anymore since binfilter
    virtual void            PrepareStore( SfxStyleSheetPool* pStyleSheetPool );
    virtual void            FinishStore();
    virtual void            FinishLoad( SfxStyleSheetPool* pStyleSheetPool );
*/
    virtual USHORT      GetParagraphCount() const;

    virtual XubString   GetText( USHORT nParagraph ) const;
    virtual void        Insert( const EditTextObject& rObj, USHORT nPara );
    virtual void        RemoveParagraph( USHORT nPara );
    virtual EditTextObject* CreateTextObject( USHORT nPara, USHORT nParas = 1 ) const;

    virtual BOOL        HasPortionInfo() const;
    virtual void        ClearPortionInfo();

    virtual BOOL        HasOnlineSpellErrors() const;

    virtual BOOL        HasCharAttribs( USHORT nWhich = 0 ) const;
    virtual void        GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const;

    virtual BOOL        RemoveCharAttribs( USHORT nWhich = 0 );
    virtual BOOL        RemoveParaAttribs( USHORT nWhich = 0 );

    virtual void        MergeParaAttribs( const SfxItemSet& rAttribs, USHORT nStart = EE_CHAR_START, USHORT nEnd = EE_CHAR_END );

    virtual BOOL        IsFieldObject() const;
    virtual const SvxFieldItem* GetField() const;
    virtual BOOL        HasField( TypeId aType = NULL ) const;

    virtual SfxItemSet  GetParaAttribs( USHORT nPara ) const;
    virtual void        SetParaAttribs( USHORT nPara, const SfxItemSet& rAttribs );

    virtual BOOL        HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const;
    virtual void        GetStyleSheet( USHORT nPara, XubString& rName, SfxStyleFamily& eFamily ) const;
    virtual void        SetStyleSheet( USHORT nPara, const XubString& rName, const SfxStyleFamily& eFamily );
    virtual BOOL        ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                            const XubString& rNewName, SfxStyleFamily eNewFamily );
    virtual void        ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    bool                operator==( const EditTextObject& rCompare ) const;
};

#endif  // _EDITOBJ_HXX
