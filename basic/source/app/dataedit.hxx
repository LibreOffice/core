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

#ifndef DATAEDIT_HXX
#define DATAEDIT_HXX

#include <vcl/menu.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <svtools/textdata.hxx>

class String;
class Font;

// Find, Load and Save must be implemented,
// the others must exist in MemberType
#define DATA_FUNC_DEF( MemberName, MemberType )                                 \
public:                                                                         \
    MemberType MemberName;                                                      \
    sal_Bool Find( const String& rStr );                                            \
    sal_Bool Load( const String& rStr );                                            \
    sal_Bool Save( const String& rStr );                                            \
                                                                                \
    void GrabFocus(){ MemberName.GrabFocus(); }                                 \
    void Show(){ MemberName.Show(); }                                           \
    void SetPosPixel( const Point& rNewPos ){ MemberName.SetPosPixel(rNewPos); }\
    void SetSizePixel( const Size& rNewSize ){ MemberName.SetSizePixel(rNewSize); } \
    Size GetSizePixel(){ return MemberName.GetSizePixel(); }                    \
    Point GetPosPixel(){ return MemberName.GetPosPixel(); }                     \
    void Update(){ MemberName.Update(); }                                       \
    void SetFont( const Font& rNewFont ){ MemberName.SetFont(rNewFont); }       \
                                                                                \
    void Delete();                                                              \
    void Cut();                                                                 \
    void Copy();                                                                \
    void Paste();                                                               \
    void Undo();                                                                \
    void Redo();                                                                \
    String GetText() const;                                                     \
    void SetText( const String& rStr );                                         \
    sal_Bool HasText() const;                                                       \
    String GetSelected();                                                       \
    TextSelection GetSelection() const;                                         \
    void SetSelection( const TextSelection& rSelection );                       \
    sal_uInt16 GetLineNr() const;                                                   \
    void ReplaceSelected( const String& rStr );                                 \
    sal_Bool IsModified();                                                          \
    void SetModifyHdl( Link l );


class DataEdit
{
public:
    virtual ~DataEdit(){}

    virtual void Delete()=0;
    virtual void Cut()=0;
    virtual void Copy()=0;
    virtual void Paste()=0;

    virtual void Undo()=0;
    virtual void Redo()=0;

    virtual sal_Bool Find( const String& )=0;         // Find and select text
    virtual sal_Bool Load( const String& )=0;         // Load text from file
    virtual sal_Bool Save( const String& )=0;         // Save text to file
    virtual String GetSelected()=0;
    virtual void GrabFocus()=0;
    virtual TextSelection GetSelection() const=0;
    virtual void SetSelection( const TextSelection& rSelection )=0;
    virtual sal_uInt16 GetLineNr() const=0;
    virtual String GetText() const=0;
    virtual void SetText( const String& rStr )=0;
    virtual sal_Bool HasText() const=0;               // to avoid GetText.Len()
    virtual void ReplaceSelected( const String& rStr )=0;
    virtual sal_Bool IsModified()=0;
    virtual void SetModifyHdl( Link )=0;
    virtual void Show()=0;
    virtual void SetPosPixel( const Point& rNewPos )=0;
    virtual void SetSizePixel( const Size& rNewSize )=0;
    virtual Size GetSizePixel()=0;
    virtual Point GetPosPixel()=0;
    virtual void Update()=0;
    virtual void SetFont( const Font& rNewFont )=0;

    virtual void BuildKontextMenu( PopupMenu *&pMenu )
    {
        (void) pMenu; /* avoid warning about unused parameter */
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
