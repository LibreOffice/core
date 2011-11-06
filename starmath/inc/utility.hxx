/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef UTILITY_HXX
#define UTILITY_HXX

#include <sfx2/minarray.hxx>
#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <tools/fract.hxx>


class SmRect;
class String;

#define C2S(cChar) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(cChar))


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
//
// SmFace
//

sal_Bool    IsItalic( const Font &rFont );
sal_Bool    IsBold( const Font &rFont );

class SmFace : public Font
{
    long    nBorderWidth;

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
    sal_uInt16  nMaxLen;
    String  aText;

public:
    SmInfoText(Window* pParent, WinBits nWinStyle = 0, sal_uInt16 nMax = 128);
    SmInfoText(Window* pParent, const ResId& rResId, sal_uInt16 nMax = 128);

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
    sal_uInt16  nSize;

    virtual void   *CreateItem(const String& rString) = 0;
    virtual void   *CreateItem(const void *pItem) = 0;
    virtual void    DestroyItem(void *pItem) = 0;

    virtual sal_Bool    CompareItem(const void *pFirstItem, const void *pSecondItem) const = 0;

    virtual String  GetStringItem(void *pItem) = 0;

    void       *GetPtr(sal_uInt16 nPos) const { return SfxPtrArr::GetObject(nPos); }
    void      *&GetPtr(sal_uInt16 nPos) { return SfxPtrArr::GetObject(nPos); }
    void        InsertPtr(sal_uInt16 nPos, void *pItem) { SfxPtrArr::Insert(nPos, pItem); }
    void        RemovePtr(sal_uInt16 nPos, sal_uInt16 nCount = 1) { SfxPtrArr::Remove(nPos, nCount); }

public:
    SmPickList(sal_uInt16 nInitSize = 0, sal_uInt16 nMaxSize = 5);
    virtual ~SmPickList();

    SmPickList&   operator = (const SmPickList& rList);

    void       *Get(sal_uInt16 nPos = 0) const { return GetPtr(nPos); }
    using   SfxPtrArr::Insert;
    void        Insert(const void* pItem);
    void        Update(const void* pItem, const void *pNewItem);
    using   SfxPtrArr::Remove;
    void        Remove(const void* pItem);

    using   SfxPtrArr::operator [];
    void       *operator [] (sal_uInt16 nPos) const { return GetPtr(nPos); }

    sal_uInt16      GetSize() const { return nSize; }
    sal_uInt16      Count() const { return SfxPtrArr::Count(); }

    void        Clear();
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

    virtual sal_Bool    CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String  GetStringItem(void *pItem);

public:
    SmStringPickList()
        : SmPickList(0, 5) {}
    SmStringPickList(sal_uInt16 nInitSize, sal_uInt16 nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmStringPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    virtual ~SmStringPickList() { Clear(); }

    virtual void    Insert(const String &rString);
    virtual void    Update(const String &rString, const String &rNewString);
    virtual void    Remove(const String &rString);

    inline sal_Bool     Contains(const String &rString) const;
    inline String   Get(sal_uInt16 nPos = 0) const;

    inline SmStringPickList& operator = (const SmStringPickList& rList);
    inline String            operator [] (sal_uInt16 nPos) const;
};

inline SmStringPickList& SmStringPickList::operator = (const SmStringPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline String SmStringPickList::operator [] (sal_uInt16 nPos) const
{
    return *((String *)SmPickList::operator[](nPos));
}

inline String SmStringPickList::Get(sal_uInt16 nPos) const
{
    return nPos < Count() ? *((String *)SmPickList::Get(nPos)) : String();
}

inline sal_Bool SmStringPickList::Contains(const String &rString) const
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

    virtual sal_Bool    CompareItem(const void *pFirstItem, const void *pSecondItem) const;

    virtual String  GetStringItem(void *pItem);

public:
    SmFontPickList()
        : SmPickList(0, 5) {}
    SmFontPickList(sal_uInt16 nInitSize, sal_uInt16 nMaxSize)
        : SmPickList(nInitSize, nMaxSize) {}
    SmFontPickList(const SmPickList& rOrig )
        : SmPickList(rOrig) {}
    virtual ~SmFontPickList() { Clear(); }

    using   SfxPtrArr::Insert;
    virtual void    Insert(const Font &rFont);
    using   SmPickList::Update;
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    using   SfxPtrArr::Remove;
    virtual void    Remove(const Font &rFont);

    using   SmPickList::Contains;
    inline sal_Bool     Contains(const Font &rFont) const;
    inline Font     Get(sal_uInt16 nPos = 0) const;

    inline SmFontPickList&  operator = (const SmFontPickList& rList);
    using   SfxPtrArr::operator [];
    inline Font             operator [] (sal_uInt16 nPos) const;

    void            ReadFrom(const SmFontDialog& rDialog);
    void            WriteTo(SmFontDialog& rDialog) const;
};

inline SmFontPickList& SmFontPickList::operator = (const SmFontPickList& rList)
{
    *(SmPickList *)this = *(SmPickList *)&rList; return *this;
}

inline Font SmFontPickList::operator [] (sal_uInt16 nPos) const
{
    return *((Font *)SmPickList::operator[](nPos));
}

inline Font SmFontPickList::Get(sal_uInt16 nPos) const
{
    return nPos < Count() ? *((Font *)SmPickList::Get(nPos)) : Font();
}

inline sal_Bool SmFontPickList::Contains(const Font &rFont) const
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
    SmStringPickComboBox(Window* pParent, WinBits nWinStyle = 0, sal_uInt16 nMax = 4);
    SmStringPickComboBox(Window* pParent, const ResId& rResId, sal_uInt16 nMax = 4);

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
    SmFontPickListBox(Window* pParent, const ResId& rResId, sal_uInt16 nMax = 4);

    SmFontPickListBox& operator = (const SmFontPickList& rList);

    using   SfxPtrArr::Insert;
    virtual void    Insert(const Font &rFont);
    using   Window::Update;
    virtual void    Update(const Font &rFont, const Font &rNewFont);
    using   SfxPtrArr::Remove;
    virtual void    Remove(const Font &rFont);
};

#endif

