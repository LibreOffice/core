/*************************************************************************
 *
 *  $RCSfile: utility.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:27 $
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

#pragma hdrstop

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif

#include "starmath.hrc"

#include "utility.hxx"
#include "dialog.hxx"
#include "view.hxx"
#include "smdll.hxx"


SmViewShell * SmGetActiveView()
    // return pointer to active SmViewShell, if this is not possible
    // return 0 instead.
{
    SfxViewShell *pView = SfxViewShell::Current();
    return PTR_CAST(SmViewShell, pView);
}


////////////////////////////////////////


SvStream& operator << (SvStream& rStream, const SmFace& rFont)
{
    rStream.WriteByteString(ExportString(rFont.GetName()));
    rStream << (ULONG)rFont.GetFamily();
    rStream << (ULONG)rFont.GetCharSet();
    rStream << (ULONG)rFont.GetWeight();
    rStream << (ULONG)rFont.GetItalic();

    return rStream;
}

SvStream& operator >> (SvStream& rStream, SmFace& rFont)
{
    ULONG   nData;
    String  aString;

    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
    rStream.ReadByteString(aString);
    rFont.SetName(aString);
    rStream >> nData;
    rFont.SetFamily((FontFamily)nData);
    rStream >> nData;
    rFont.SetCharSet((CharSet)nData);
    rStream >> nData;
    rFont.SetWeight((FontWeight)nData);
    rStream >> nData;
    rFont.SetItalic((FontItalic)nData);

    return rStream;
}

void ReadSM20Font(SvStream& rStream, Font& rFont)
{
    BOOL    bData;
    ULONG   nData;
    String  aString;

    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
    rStream.ReadByteString(aString);
    rFont.SetName(aString);
    rStream >> nData;
    rFont.SetFamily((FontFamily)nData);
    rStream >> nData;
    rFont.SetCharSet((CharSet)nData);
    rStream >> nData;
    rFont.SetWeight((FontWeight)nData);
    rStream >> bData;
    if (bData)
        rFont.SetItalic(ITALIC_NORMAL);
    else
        rFont.SetItalic(ITALIC_NONE);
}


////////////////////////////////////////////////////////////
//
// SmRectCache
//


SmRectCache::Key::Key(const XubString &rText, const Font &rFont)
{
}


BOOL SmRectCache::Key::operator <  (const Key &rKey) const
{
#ifdef never
    BOOL  bRes = FALSE;

    if (aText < rKey.aText)
        bRes = TRUE;
    else if ()

    return      aText       <  rKey.aText
            ||  aText       == rKey.aText        &&  aFontName   < rKey.aFontName
            ||  aFontName   == rKey.aFontname    &&  aFontSize   < rKey.aFontSize
            ||  aFontSize   == rKey.aFontSize    &&  eFontWeight < rKey.eFontWeight
            ||  eFontWeight == rKey.eFontWeight  &&  eFontItalic < rKey.eFontItalic;
#endif
    return FALSE;
}


BOOL SmRectCache::Key::operator == (const Key &rKey) const
{
    return      aText       == rKey.aText
            &&  aFontName   == rKey.aFontName
            &&  aFontSize   == rKey.aFontSize
            &&  eFontWeight == rKey.eFontWeight
            &&  eFontItalic == rKey.eFontItalic;
}


SmRectCache::SmRectCache()
{
    pVirDev = 0;
}


SmRectCache::~SmRectCache()
{
    delete pVirDev;
}


const SmRect * SmRectCache::Search(const Key &rKey) const
{
    return 0;
}


const SmRect * SmRectCache::Add(const Key &rKey, const SmRect &rRect)
{
    return (const SmRect *)-1;
}


VirtualDevice * SmRectCache::GetVirDev()
{
    if (!pVirDev)
    {
        SmViewShell *pView = SmGetActiveView();
        if (pView)
            pVirDev = new VirtualDevice( pView->GetGraphicWindow() );
        else
            pVirDev = new VirtualDevice;
        pVirDev->SetMapMode( MapMode(MAP_100TH_MM) );
    }
    DBG_ASSERT(pVirDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
               "Sm : falscher MapMode");

    return pVirDev;
}


void SmRectCache::Reset()
{
}


////////////////////////////////////////////////////////////

ByteString ConvertUnknownCharacter(sal_Unicode ch)
{
    ByteString aString("<?");
    aString += "UCS2(";
    aString += ByteString::CreateFromInt32(ch);
    aString += ")>";

    return aString;
}

const ByteString ExportString(const String& rString)
{
    ByteString  aString;

    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();

    for (xub_StrLen i = 0; i < rString.Len(); i++)
    {
        sal_Unicode ch = rString.GetChar(i);

        sal_Char cChar = ByteString::ConvertFromUnicode( ch, eEnc );
        if ((ch != '\r') && (ch != '\n') && (ch != '\t'))
        {
//          ch = String::Convert(rString[i], eSet, CHARSET_ANSI);

            if (ch == 0)
                aString += ConvertUnknownCharacter(ch);
            else
                aString += cChar;
        }
        else
            aString += ch;
    }

    aString.ConvertLineEnd(LINEEND_CRLF);

    return (aString);
}

////////////////////////////////////////////////////////////


/**************************************************************************/

SmPickList::SmPickList(USHORT nInitSize, USHORT nMaxSize) :
    SfxPtrArr((BYTE) nInitSize, 1)
{
    nSize = nMaxSize;
}


SmPickList::~SmPickList()
{
    Clear();
}


SmPickList& SmPickList::operator=(const SmPickList& rList)
{
    USHORT  nPos;

    Clear();
    nSize = rList.nSize;
    for (nPos = 0; nPos < rList.Count(); nPos++)
        InsertPtr(nPos, CreateItem(rList.Get(nPos)));

    return *this;
}


void SmPickList::Insert(const void *pItem)
{
    Remove(pItem);
    InsertPtr(0, CreateItem(pItem));

    if (Count() > nSize)
    {
        DestroyItem(GetPtr(nSize));
        RemovePtr(nSize, 1);
    }
}


void SmPickList::Update(const void *pItem, const void *pNewItem)
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
        {
            DestroyItem(GetPtr(nPos));
            GetPtr(nPos) = CreateItem(pNewItem);
            break;
        }
}

void SmPickList::Remove(const void *pItem)
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
        {
            DestroyItem(GetPtr(nPos));
            RemovePtr(nPos, 1);
            break;
        }
}

void SmPickList::SetSize(USHORT nNewSize)
{
    nSize = nNewSize;

    while (Count() > nSize)
    {
        DestroyItem(GetPtr(Count() - 1));
        RemovePtr(Count() - 1, 1);
    }
}


BOOL SmPickList::Contains(const void *pItem) const
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        if (CompareItem(GetPtr(nPos), pItem))
            return TRUE;

    return FALSE;
}


void SmPickList::Clear()
{
    USHORT  nPos;

    for (nPos = 0; nPos < Count(); nPos++)
        DestroyItem(GetPtr(nPos));

    RemovePtr(0, Count());
}


SvStream& operator << (SvStream& rStream, const SmPickList& rPickList)
{
    USHORT      nPos;

    rStream << 'p';
    rStream << rPickList.nSize;
    rStream << rPickList.Count();

    for (nPos = 0; nPos < rPickList.Count(); nPos++)
        rPickList.SaveItem(rStream, rPickList.GetObject(nPos));

    return rStream;
}

SvStream& operator >> (SvStream& rStream, SmPickList& rPickList)
{
    char    cTag;
    USHORT  nCount, nPos;
    void   *pItem;

    rPickList.Clear();

    rStream >> cTag;
    if (cTag == 'p')
    {
        rStream >> rPickList.nSize;
        rStream >> nCount;

        for (nPos = 0; nPos < nCount; nPos++)
        {
            pItem = rPickList.CreateItem(String());
            rPickList.LoadItem(rStream, pItem);
            rPickList.InsertPtr(nPos, pItem);
        }
    }

    return rStream;
}


/**************************************************************************/
/**************************************************************************/

void * SmFontPickList::CreateItem(const String& rString)
{
    return new Font();
}

void * SmFontPickList::CreateItem(const void *pItem)
{
    return new Font(*((Font *) pItem));
}

void SmFontPickList::DestroyItem(void *pItem)
{
    delete (Font *)pItem;
}

BOOL SmFontPickList::CompareItem(const void *pFirstItem, const void *pSecondItem) const
{
    Font    *pFirstFont, *pSecondFont;

    pFirstFont  = (Font *)pFirstItem;
    pSecondFont = (Font *)pSecondItem;

    if (pFirstFont->GetName() == pSecondFont->GetName())
        if ((pFirstFont->GetFamily()  == pSecondFont->GetFamily())  &&
            (pFirstFont->GetCharSet() == pSecondFont->GetCharSet()) &&
            (pFirstFont->GetWeight()  == pSecondFont->GetWeight())  &&
            (pFirstFont->GetItalic()  == pSecondFont->GetItalic()))
            return (TRUE);

    return FALSE;
}

String SmFontPickList::GetStringItem(void *pItem)
{
    Font   *pFont;
    String  aString;

    pFont = (Font *)pItem;

    aString = pFont->GetName();

    if (pFont->GetItalic() != ITALIC_NONE)
        aString += String(SmResId(RID_FONTITALIC));
    if (pFont->GetWeight() == WEIGHT_BOLD)
        aString += String(SmResId(RID_FONTBOLD));

    return (aString);
}

void SmFontPickList::LoadItem(SvStream& rStream, void *pItem)
{
    rStream >> *((Font *)pItem);
}

void SmFontPickList::SaveItem(SvStream& rStream, const void *pItem) const
{
    rStream << *(const Font *) pItem;
}

void SmFontPickList::Insert(const Font &rFont)
{
    SmPickList::Insert((void *)&rFont);
}

void SmFontPickList::Update(const Font &rFont, const Font &rNewFont)
{
    SmPickList::Update((void *)&rFont, (void *)&rNewFont);
}

void SmFontPickList::Remove(const Font &rFont)
{
    SmPickList::Remove((void *)&rFont);
}


void SmFontPickList::ReadFrom(const SmFontDialog& rDialog)
{
    Insert(rDialog.GetFont());
}

void SmFontPickList::WriteTo(SmFontDialog& rDialog) const
{
    rDialog.SetFont(Get());
}


/**************************************************************************/


/**************************************************************************/

IMPL_LINK( SmFontPickListBox, SelectHdl, ListBox *, pListBox )
{
    USHORT  nPos;
    String  aString;

    nPos = GetSelectEntryPos();

    if (nPos != 0)
    {
        SmFontPickList::Insert(Get(nPos));
        aString = GetEntry(nPos);
        RemoveEntry(nPos);
        InsertEntry(aString, 0);
    }

    SelectEntryPos(0);

    return 0;
}


SmFontPickListBox::SmFontPickListBox(Window* pParent, WinBits nWinStyle, USHORT nMax) :
    SmFontPickList(nMax, nMax),
    ListBox(pParent, nWinStyle)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}


SmFontPickListBox::SmFontPickListBox(Window* pParent, const ResId& rResId, USHORT nMax) :
    SmFontPickList(nMax, nMax),
    ListBox(pParent, rResId)
{
    SetSelectHdl(LINK(this, SmFontPickListBox, SelectHdl));
}


SmFontPickListBox& SmFontPickListBox::operator=(const SmFontPickList& rList)
{
    USHORT nPos;

    *(SmFontPickList *)this = rList;

    for (nPos = 0; nPos < Count(); nPos++)
        InsertEntry(GetStringItem(GetPtr(nPos)), nPos);

    if (Count() > 0)
        SelectEntry(GetStringItem(GetPtr(0)));

    return *this;
}

void SmFontPickListBox::Insert(const Font &rFont)
{
    SmFontPickList::Insert(rFont);

    RemoveEntry(GetStringItem(GetPtr(0)));
    InsertEntry(GetStringItem(GetPtr(0)), 0);
    SelectEntry(GetStringItem(GetPtr(0)));

    while (GetEntryCount() > nSize)
        RemoveEntry(GetEntryCount() - 1);

    return;
}


void SmFontPickListBox::Update(const Font &rFont, const Font &rNewFont)
{
    SmFontPickList::Update(rFont, rNewFont);

    // ********************** hier fehlt noch was

    return;
}


void SmFontPickListBox::Remove(const Font &rFont)
{
    SmFontPickList::Remove(rFont);

    // ********************** hier fehlt noch was

    return;
}

////////////////////////////////////////


void SmFace::SetSize(const Size& rSize)
{
    Size  aSize (rSize);

    // check the requested size against minimum value
    static int __READONLY_DATA  nMinVal = SmPtsTo100th_mm(2);

    if (aSize.Height() < nMinVal)
        aSize.Height() = nMinVal;

    //! we don't force a maximum value here because this may prevent eg the
    //! parentheses in "left ( ... right )" from matching up with large
    //! bodies (eg stack{...} with many entries).
    //! Of course this is holds only if characters are used and not polygons.

    Font::SetSize(aSize);
}


long SmFace::GetBorderWidth() const
{
    if (nBorderWidth < 0)
        return GetDefaultBorderWidth();
    else
        return nBorderWidth;
}

SmFace & SmFace::operator = (const SmFace &rFace)
{
    Font::operator = (rFace);
    nBorderWidth = -1;
    return *this;
}


SmFace & operator *= (SmFace &rFace, const Fraction &rFrac)
    // scales the width and height of 'rFace' by 'rFrac' and returns a
    // reference to 'rFace'.
    // It's main use is to make scaling fonts look easier.
{   const Size &rFaceSize = rFace.GetSize();

    rFace.SetSize(Size(Fraction(rFaceSize.Width())  *= rFrac,
                       Fraction(rFaceSize.Height()) *= rFrac));
    return rFace;
}



