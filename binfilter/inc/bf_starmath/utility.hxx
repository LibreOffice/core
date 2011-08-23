/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef UTILITY_HXX
#define UTILITY_HXX

#include <bf_sfx2/minarray.hxx>
#include <vcl/font.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/fract.hxx>
class String; 
namespace binfilter {


class SmRect;


#define C2S(cChar) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))

/////////////////////////////////////////////////////////////////

const ByteString    ExportString( const String& rString );
const String        ImportString( const ByteString& rByteString );

/////////////////////////////////////////////////////////////////

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
    {	XubString 	aText,
                    aFontName;
        Size	 	aFontSize;
        FontWeight  eFontWeight;
        FontItalic  eFontItalic;

        Key(const XubString &rText, const Font &rFont);

    };

    SmRectCache();
    ~SmRectCache();

    const SmRect *	Search(const Key &rKey) const;
    const SmRect *	Add(const Key &rKey, const SmRect &rRect);

    VirtualDevice * GetVirDev();

    void Reset();
};


////////////////////////////////////////////////////////////
//
// SmFace
//


class SmFace : public Font
{
    long	nBorderWidth;

    void    Impl_Init();

public:
    SmFace() :
        Font(), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const Font& rFont) :
        Font(rFont), nBorderWidth(-1) { Impl_Init(); }
    SmFace(const String& rName, const Size& rSize) :
        Font(rName, rSize), nBorderWidth(-1) { Impl_Init(); }
    SmFace( FontFamily eFamily, const Size& rSize) :
        Font(eFamily, rSize), nBorderWidth(-1) { Impl_Init(); }

    SmFace(const SmFace &rFace) :
        Font(rFace), nBorderWidth(-1) { Impl_Init(); }

    // overloaded version in order to supply a min value
    // for font size (height). (Also used in ctor's to do so.)
    void 	SetSize(const Size& rSize);

    void	SetBorderWidth(long nWidth)		{ nBorderWidth = nWidth; }
    long	GetBorderWidth() const;
    long	GetDefaultBorderWidth() const	{ return GetSize().Height() / 20 ; }
    void	FreezeBorderWidth()		{ nBorderWidth = GetDefaultBorderWidth(); }

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
    USHORT	nMaxLen;
    String	aText;

public:
    SmInfoText(Window* pParent, WinBits nWinStyle = 0, USHORT nMax = 128);
    SmInfoText(Window* pParent, const ResId& rResId, USHORT nMax = 128);

    void	SetText(const String& rStr);

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
    USHORT	nSize;

    virtual void   *CreateItem(const String& rString) = 0;
    virtual void   *CreateItem(const void *pItem) = 0;
    virtual void	DestroyItem(void *pItem) = 0;

    virtual BOOL	CompareItem(const void *pFirstItem, const void *pSecondItem) const = 0;

    virtual String	GetStringItem(void *pItem) = 0;

    virtual void	LoadItem(SvStream& rStream, void *pItem) = 0;
    virtual void	SaveItem(SvStream& rStream, const void *pItem) const = 0;

    void	   *GetPtr(USHORT nPos) const { return SfxPtrArr::GetObject(nPos); }
    void	  *&GetPtr(USHORT nPos) { return SfxPtrArr::GetObject(nPos); }
    void		InsertPtr(USHORT nPos, void *pItem) { SfxPtrArr::Insert(nPos, pItem); }
    void		RemovePtr(USHORT nPos, USHORT nCount = 1) { SfxPtrArr::Remove(nPos, nCount); }

public:
    SmPickList(USHORT nInitSize = 0, USHORT nMaxSize = 5);
    ~SmPickList();


    void	   *Get(USHORT nPos = 0) const { return GetPtr(nPos); }

    void	   *operator [] (USHORT nPos) const { return GetPtr(nPos); }

    USHORT		GetSize() const { return nSize; }
    USHORT		Count() const { return SfxPtrArr::Count(); }

    void		Clear();

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
    virtual void	DestroyItem(void *pItem);

    virtual BOOL	CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String	GetStringItem(void *pItem);

    virtual void	LoadItem(SvStream& rStream, void *pItem);
    virtual void	SaveItem(SvStream& rStream, const void *pItem) const;

public:
    SmStringPickList()
        : SmPickList(0, 5) {}
    SmStringPickList(USHORT nInitSize, USHORT nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmStringPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    ~SmStringPickList() { Clear(); }

    virtual void	Insert(const String &rString);
    virtual void	Update(const String &rString, const String &rNewString);
    virtual void	Remove(const String &rString);

    inline BOOL		Contains(const String &rString) const;
    inline String	Get(USHORT nPos = 0) const;

    inline SmStringPickList& operator = (const SmStringPickList& rList);
    inline String			 operator [] (USHORT nPos) const;
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

inline BOOL	SmStringPickList::Contains(const String &rString) const
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
    virtual void	DestroyItem(void *pItem);

    virtual BOOL	CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String	GetStringItem(void *pItem);

    virtual void	LoadItem(SvStream& rStream, void *pItem);
    virtual void	SaveItem(SvStream& rStream, const void *pItem) const;

public:
    SmFontPickList()
        : SmPickList(0, 5) {}
    SmFontPickList(USHORT nInitSize, USHORT nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmFontPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    ~SmFontPickList() { Clear(); }


    inline BOOL		Contains(const Font &rFont) const;
    inline Font		Get(USHORT nPos = 0) const;

    inline SmFontPickList& 	operator = (const SmFontPickList& rList);
    inline Font				operator [] (USHORT nPos) const;

};

inline SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline Font	SmFontPickList::operator [] (USHORT nPos) const
{
    return *((Font *)SmPickList::operator[](nPos));
}

inline Font SmFontPickList::Get(USHORT nPos) const
{
    return nPos < Count() ? *((Font *)SmPickList::Get(nPos)) : Font();
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

    void			SetText(const String& rStr);

    virtual void	Insert(const String &rString);
    virtual void	Update(const String &rString, const String &rNewString);
    virtual void	Remove(const String &rString);
};
#endif

////////////////////////////////////////////////////////////
//
//	SmFontPickListBox
//


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
