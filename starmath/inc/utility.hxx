/*************************************************************************
 *
 *  $RCSfile: utility.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-02 15:07:16 $
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
#ifndef UTILITY_HXX
#define UTILITY_HXX

#ifndef _SFXVARARR_HXX //autogen
#include <sfx2/minarray.hxx>
#endif
#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif


class SmRect;
class String;

#define C2S(cChar) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))

const ByteString ExportString(const String& rString);


inline long SmPtsTo100th_mm(long nNumPts)
    // returns the length (in 100th of mm) that corresponds to the length
    // 'nNumPts' (in units points).
    // 72.27 [pt] = 1 [inch] = 2,54 [cm] = 2540 [100th of mm].
    // result is being rounded to the nearest integer.
{
    DBG_ASSERT(nNumPts >= 0, "Sm : Ooops...");
    // broken into multiple and fraction of 'nNumPts' to reduce chance
    // of overflow
    // (7227 / 2) is added in order to round to the nearest integer
    return 35 * nNumPts + (nNumPts * 1055L + (7227 / 2)) / 7227L;
}


inline long SmPtsTo100th_mm(const Fraction &rNumPts)
    // as above but with argument 'rNumPts' as 'Fraction'
{
    Fraction  aTmp (254000L, 7227L);
    return aTmp *= rNumPts;
}


inline Fraction Sm100th_mmToPts(long nNum100th_mm)
    // returns the length (in points) that corresponds to the length
    // 'nNum100th_mm' (in 100th of mm).
{
    DBG_ASSERT(nNum100th_mm >= 0, "Sm : Ooops...");
    Fraction  aTmp (7227L, 254000L);
    return aTmp *= Fraction(nNum100th_mm);
}


inline long SmRoundFraction(const Fraction &rFrac)
{
    DBG_ASSERT(rFrac > Fraction(), "Sm : Ooops...");
    return (rFrac.GetNumerator() + rFrac.GetDenominator() / 2) / rFrac.GetDenominator();
}


class SmViewShell;
SmViewShell * SmGetActiveView();


////////////////////////////////////////////////////////////

class SmFace;

SvStream& operator << (SvStream& rStream, const SmFace& rFont);
SvStream& operator >> (SvStream& rStream, SmFace& rFont);

void ReadSM20Font(SvStream& rStream, Font& rFont);


////////////////////////////////////////////////////////////
//
// SmRectCache
//

class SmRectCache
{
    VirtualDevice *pVirDev;

    SmRectCache(const SmRectCache &);
    SmRectCache & operator = (const SmRectCache &);

public:
    struct Key
    {   XubString   aText,
                    aFontName;
        Size        aFontSize;
        FontWeight  eFontWeight;
        FontItalic  eFontItalic;

        Key(const XubString &rText, const Font &rFont);

        BOOL operator <  (const Key &rKey) const;
        BOOL operator == (const Key &rKey) const;
    };

    SmRectCache();
    ~SmRectCache();

    const SmRect *  Search(const Key &rKey) const;
    const SmRect *  Add(const Key &rKey, const SmRect &rRect);

    VirtualDevice * GetVirDev();

    void Reset();
};


////////////////////////////////////////////////////////////
//
// SmFace
//


class SmFace : public Font
{
    long    nBorderWidth;

public:
    SmFace() :
        Font(), nBorderWidth(-1) {}
    SmFace(const Font& rFont) :
        Font(rFont), nBorderWidth(-1) { SetSize(GetSize()); }
    SmFace(const String& rName, const Size& rSize) :
        Font(rName, rSize), nBorderWidth(-1) { SetSize(GetSize()); }
    SmFace( FontFamily eFamily, const Size& rSize) :
        Font(eFamily, rSize), nBorderWidth(-1) { SetSize(GetSize()); }

    SmFace(const SmFace &rFace) :
        Font(rFace), nBorderWidth(-1) { SetSize(GetSize()); }

    // overloaded version in order to supply a min value
    // for font size (height). (Also used in ctor's to do so.)
    void    SetSize(const Size& rSize);

    void    SetBorderWidth(long nWidth)     { nBorderWidth = nWidth; }
    long    GetBorderWidth() const;
    long    GetDefaultBorderWidth() const   { return GetSize().Height() / 20 ; }
    void    FreezeBorderWidth()     { nBorderWidth = GetDefaultBorderWidth(); }

    SmFace & operator = (const SmFace &rFace);
};

SmFace & operator *= (SmFace &rFace, const Fraction &rFrac);


#ifdef NEVER
////////////////////////////////////////////////////////////
//
// SmInfoText
//

class SmInfoText : public FixedText
{
protected:
    USHORT  nMaxLen;
    String  aText;

public:
    SmInfoText(Window* pParent, WinBits nWinStyle = 0, USHORT nMax = 128);
    SmInfoText(Window* pParent, const ResId& rResId, USHORT nMax = 128);

    void    SetText(const String& rStr);

    XubString GetText() const { return (aText); }
};
#endif


////////////////////////////////////////////////////////////
//
// SmPickList
//

class SmPickList : public SfxPtrArr
{
protected:
    USHORT  nSize;

    virtual void   *CreateItem(const String& rString) = 0;
    virtual void   *CreateItem(const void *pItem) = 0;
    virtual void    DestroyItem(void *pItem) = 0;

    virtual BOOL    CompareItem(const void *pFirstItem, const void *pSecondItem) const = 0;

    virtual String  GetStringItem(void *pItem) = 0;

    virtual void    LoadItem(SvStream& rStream, void *pItem) = 0;
    virtual void    SaveItem(SvStream& rStream, const void *pItem) const = 0;

    void       *GetPtr(USHORT nPos) const { return SfxPtrArr::GetObject(nPos); }
    void      *&GetPtr(USHORT nPos) { return SfxPtrArr::GetObject(nPos); }
    void        InsertPtr(USHORT nPos, void *pItem) { SfxPtrArr::Insert(nPos, pItem); }
    void        RemovePtr(USHORT nPos, USHORT nCount = 1) { SfxPtrArr::Remove(nPos, nCount); }

public:
    SmPickList(USHORT nInitSize = 0, USHORT nMaxSize = 5);
    ~SmPickList();

    SmPickList&   operator = (const SmPickList& rList);

    void       *Get(USHORT nPos = 0) const { return GetPtr(nPos); }
    void        Insert(const void* pItem);
    void        Update(const void* pItem, const void *pNewItem);
    void        Remove(const void* pItem);

    void       *operator [] (USHORT nPos) const { return GetPtr(nPos); }

    void        SetSize(USHORT nNewSize);
    USHORT      GetSize() const { return nSize; }
    USHORT      Count() const { return SfxPtrArr::Count(); }

    BOOL        Contains(const void *pItem) const;
    void        Clear();

    friend SvStream&   operator << (SvStream& rStream, const SmPickList& rPickList);
    friend SvStream&   operator >> (SvStream& rStream, SmPickList& rPickList);
};


////////////////////////////////////////////////////////////
//
// SmStringPickList
//
#ifdef NEVER
class SmStringPickList : public SmPickList
{
protected:
    virtual void   *CreateItem(const String& rString);
    virtual void   *CreateItem(const void *pItem);
    virtual void    DestroyItem(void *pItem);

    virtual BOOL    CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String  GetStringItem(void *pItem);

    virtual void    LoadItem(SvStream& rStream, void *pItem);
    virtual void    SaveItem(SvStream& rStream, const void *pItem) const;

public:
    SmStringPickList()
        : SmPickList(0, 5) {}
    SmStringPickList(USHORT nInitSize, USHORT nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmStringPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    ~SmStringPickList() { Clear(); }

    virtual void    Insert(const String &rString);
    virtual void    Update(const String &rString, const String &rNewString);
    virtual void    Remove(const String &rString);

    inline BOOL     Contains(const String &rString) const;
    inline String   Get(USHORT nPos = 0) const;

    inline SmStringPickList& operator = (const SmStringPickList& rList);
    inline String            operator [] (USHORT nPos) const;
};

inline SmStringPickList& SmStringPickList::operator = (const SmStringPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline String SmStringPickList::operator [] (USHORT nPos) const
{
    return *((String *)SmPickList::operator[](nPos));
}

inline String SmStringPickList::Get(USHORT nPos) const
{
    return nPos < Count() ? *((String *)SmPickList::Get(nPos)) : String();
}

inline BOOL SmStringPickList::Contains(const String &rString) const
{
    return SmPickList::Contains((void *)&rString);
}
#endif

////////////////////////////////////////////////////////////
//
// SmFontPickList
//

class SmFontDialog;

class SmFontPickList : public SmPickList
{
protected:
    virtual void   *CreateItem(const String& rString);
    virtual void   *CreateItem(const void *pItem);
    virtual void    DestroyItem(void *pItem);

    virtual BOOL    CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String  GetStringItem(void *pItem);

    virtual void    LoadItem(SvStream& rStream, void *pItem);
    virtual void    SaveItem(SvStream& rStream, const void *pItem) const;

public:
    SmFontPickList()
        : SmPickList(0, 5) {}
    SmFontPickList(USHORT nInitSize, USHORT nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmFontPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    ~SmFontPickList() { Clear(); }

    virtual void    Insert(const Font &rFont);
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    virtual void    Remove(const Font &rFont);

    inline BOOL     Contains(const Font &rFont) const;
    inline Font     Get(USHORT nPos = 0) const;

    inline SmFontPickList&  operator = (const SmFontPickList& rList);
    inline Font             operator [] (USHORT nPos) const;

    void            ReadFrom(const SmFontDialog& rDialog);
    void            WriteTo(SmFontDialog& rDialog) const;
};

inline SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline Font SmFontPickList::operator [] (USHORT nPos) const
{
    return *((Font *)SmPickList::operator[](nPos));
}

inline Font SmFontPickList::Get(USHORT nPos) const
{
    return nPos < Count() ? *((Font *)SmPickList::Get(nPos)) : Font();
}

inline BOOL SmFontPickList::Contains(const Font &rFont) const
{
    return SmPickList::Contains((void *)&rFont);
}


////////////////////////////////////////////////////////////
//
// SmStringPickComboBox
//
#ifdef NEVER
class SmStringPickComboBox : public SmStringPickList, public ComboBox
{
protected:
    virtual void LoseFocus();

    DECL_LINK(SelectHdl, ComboBox *);

public:
    SmStringPickComboBox(Window* pParent, WinBits nWinStyle = 0, USHORT nMax = 4);
    SmStringPickComboBox(Window* pParent, const ResId& rResId, USHORT nMax = 4);

    SmStringPickComboBox& operator = (const SmStringPickList& rList);

    void            SetText(const String& rStr);

    virtual void    Insert(const String &rString);
    virtual void    Update(const String &rString, const String &rNewString);
    virtual void    Remove(const String &rString);
};
#endif

////////////////////////////////////////////////////////////
//
//  SmFontPickListBox
//

class SmFontPickListBox : public SmFontPickList, public ListBox
{
protected:
    DECL_LINK(SelectHdl, ListBox *);

public:
    SmFontPickListBox(Window* pParent, WinBits nWinStyle = 0, USHORT nMax = 4);
    SmFontPickListBox(Window* pParent, const ResId& rResId, USHORT nMax = 4);

    SmFontPickListBox& operator = (const SmFontPickList& rList);

    virtual void    Insert(const Font &rFont);
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    virtual void    Remove(const Font &rFont);
};

#endif

