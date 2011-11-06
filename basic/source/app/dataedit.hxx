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
