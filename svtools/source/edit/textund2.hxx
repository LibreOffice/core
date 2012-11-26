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


#ifndef _TEXTUND2_HXX
#define _TEXTUND2_HXX

#include <textundo.hxx>


class TextUndoDelPara : public TextUndo
{
private:
    sal_Bool            mbDelObject;
    sal_uLong           mnPara;
    TextNode*       mpNode; // Zeigt auf das gueltige, nicht zerstoerte Objekt!

public:
                    TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, sal_uLong nPara );
                    ~TextUndoDelPara();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoConnectParas : public TextUndo
{
private:
    sal_uLong           mnPara;
    sal_uInt16          mnSepPos;

public:
                    TextUndoConnectParas( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nSepPos );
                    ~TextUndoConnectParas();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoSplitPara : public TextUndo
{
private:
    sal_uLong           mnPara;
    sal_uInt16          mnSepPos;

public:
                    TextUndoSplitPara( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nSepPos );
                    ~TextUndoSplitPara();

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoInsertChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

//  const TextPaM&  GetTextPaM() { return aTextPaM; }
//  String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();

    virtual sal_Bool    Merge( SfxUndoAction *pNextAction );
};


class TextUndoRemoveChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

//  const TextPaM&      GetTextPaM() { return aTextPaM; }
//  String&         GetStr() { return aText; }

    virtual void    Undo();
    virtual void    Redo();
};


class TextUndoSetAttribs: public TextUndo
{
private:
    TextSelection       maSelection;
//  SfxItemSet          aNewAttribs;
//  TextInfoArray       aPrevAttribs;
//  sal_uInt8               nSpecial;
//  sal_Bool                bSetIsRemove;
//  sal_uInt16              nRemoveWhich;
//
//  void                ImpSetSelection( TextView* pView );


public:
                        TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rESel );
                        ~TextUndoSetAttribs();

//  TextInfoArray&      GetTextInfos()  { return aPrevAttribs; }
//  SfxItemSet&         GetNewAttribs()     { return aNewAttribs; }
//  void                SetSpecial( sal_uInt8 n )           { nSpecial = n; }
//  void                SetRemoveAttribs( sal_Bool b )      { bSetIsRemove = b; }
//  void                SetRemoveWhich( sal_uInt16 n )      { nRemoveWhich = n; }

    virtual void        Undo();
    virtual void        Redo();
};

#endif // _TEXTUND2_HXX
