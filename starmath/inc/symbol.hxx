/*************************************************************************
 *
 *  $RCSfile: symbol.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:25 $
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
#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _DYNARY_HXX //autogen
#include <tools/dynary.hxx>
#endif
#ifndef UTILITY_HXX
#include "utility.hxx"
#endif


#define SS_ATTR_ACCESS      0x80

#define SYMBOLSET_NONE  0xFFFF
#define SYMBOL_NONE     0xFFFF

////////////////////////////////////////////////////////////////////////////////

class SmSym
{
    friend class SmSymSetManager;

    SmFace               Face;
    String               Name;
    SmSym               *pHashNext;
    SmSymSetManager     *pSymSetManager;
    sal_Unicode          Character;
    BYTE                 Attribut;

public:
    SmSym();
    SmSym(const SmSym& rSymbol);
    SmSym(const String& rName, const Font& rFont, sal_Unicode aChar);

    SmSym&      operator = (const SmSym& rSymbol);

    void        SetSymbolName(const String& rName);

    const Font&     GetFace() const { return Face; }
    sal_Unicode     GetCharacter() const { return Character; }
    const String&   GetName() const { return Name; }

    Font&           GetFace() { return Face; }
    sal_Unicode&    GetCharacter() { return Character; }
    String&         GetName() { return Name; }

    friend SvStream& operator << (SvStream& rStream, const SmSym& rSymbol);
    friend SvStream& operator >> (SvStream& rStream, SmSym& rSymbol);
};

DECLARE_LIST(SmListSym, SmSym *);

/**************************************************************************/

class SmSymSet
{
    friend class SmSymSetManager;

    SmListSym            SymbolList;
    String               Name;
    SmSymSetManager     *pSymSetManager;

public:
                SmSymSet();
                SmSymSet(const SmSymSet& rSymbolSet);
                SmSymSet(const String& rName);
                ~SmSymSet();

    SmSymSet&   operator = (const SmSymSet& rSymbolSet);

    const String&   GetName() const { return Name; }
    void            SetName(String& rName);
    USHORT          GetCount() const { return (USHORT) SymbolList.Count(); }

    const SmSym&    GetSymbol(USHORT SymbolNo) const
    {
        DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");
        return *SymbolList.GetObject(SymbolNo);
    }

    USHORT      AddSymbol(SmSym* pSymbol);
    void        DeleteSymbol(USHORT SymbolNo);
    void        RenameSymbol(USHORT SymbolNo, String& rName);
    void        ReplaceSymbol(USHORT SymbolNo, SmSym& rSymbol);
    SmSym *     RemoveSymbol(USHORT SymbolNo);
    USHORT      GetSymbolPos(const String& rName);

    friend SvStream& operator << (SvStream& rStream, const SmSymSet& rSymbolSet);
    friend SvStream& operator >> (SvStream& rStream, SmSymSet& rSymbolSet);
};

DECLARE_DYNARRAY(SmArraySymSet, SmSymSet *)

/**************************************************************************/

class SmSymbolDialog;

class SmSymSetManager : public SfxListener
{
    SmArraySymSet  SymbolSets;
    String          aStreamName;
    SmSym          **HashEntries;
    UINT32         NoSymbolSets;
    UINT32         NoHashEntries;
    BOOL           Modified;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

    UINT32      GetHashIndex(const String& rSymbolName);
    void        EnterHashTable(SmSymSet& rSymbolSet);
    void        FillHashTable();

public:
    void        Init();
    void        Exit();

                SmSymSetManager(UINT32 HashTableSize = 137);
                SmSymSetManager(const SmSymSetManager& rSymbolSetManager);
                ~SmSymSetManager();

    SmSymSetManager&   operator = (const SmSymSetManager& rSymbolSetManager);

    USHORT      GetCount() const { return NoSymbolSets; }
    SmSymSet   *GetSymbolSet(USHORT SymbolSetNo) const { return SymbolSets.Get(SymbolSetNo);}

    USHORT      AddSymbolSet(SmSymSet* pSymbolSet);
    void        ChangeSymbolSet(SmSymSet* pSymbolSet);
    void        DeleteSymbolSet(USHORT SymbolSetNo);
    USHORT      GetSymbolSetPos(const String& rSymbolSetName) const;

    SmSym       *   GetSymbol(const String& rSymbolName);
    const SmSym *   GetSymbol(const String& rSymbolName) const
    {
        return ((SmSymSetManager *) this)->GetSymbol(rSymbolName);
    }

    void        AppendExtraSymbolSet(SmSymSet* pSymbolSet) {}
    void        ResetAccessedSymbols() {}
    SmSymSet    GetAccessedSymbols() { return SmSymSet(); }

    BOOL        IsModified() const { return (Modified); }
    void        SetModified(BOOL Modify) { Modified = Modify; }

    void        Load(const String& rURL);
    void        Save();

    friend SvStream& operator << (SvStream& rStream, SmSymSetManager& rSymbolSetManager);
    friend SvStream& operator >> (SvStream& rStream, SmSymSetManager& rSymbolSetManager);
};

#endif

