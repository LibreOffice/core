/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _TEXTUND2_HXX
#define _TEXTUND2_HXX

#include "textundo.hxx"
#include <vcl/textdata.hxx>


class TextUndoDelPara : public TextUndo
{
private:
    sal_Bool            mbDelObject;
    sal_uLong           mnPara;
    TextNode*       mpNode; // Zeigt auf das gueltige, nicht zerstoerte Objekt!

public:
                    TYPEINFO();
                    TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, sal_uLong nPara );
                    ~TextUndoDelPara();

    virtual void    Undo();
    virtual void    Redo();

    virtual rtl::OUString GetComment () const;
};


class TextUndoConnectParas : public TextUndo
{
private:
    sal_uLong           mnPara;
    sal_uInt16          mnSepPos;

public:
                    TYPEINFO();
                    TextUndoConnectParas( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nSepPos );
                    ~TextUndoConnectParas();

    virtual void    Undo();
    virtual void    Redo();

    virtual rtl::OUString GetComment () const;
};


class TextUndoSplitPara : public TextUndo
{
private:
    sal_uLong           mnPara;
    sal_uInt16          mnSepPos;

public:
                    TYPEINFO();
                    TextUndoSplitPara( TextEngine* pTextEngine, sal_uLong nPara, sal_uInt16 nSepPos );
                    ~TextUndoSplitPara();

    virtual void    Undo();
    virtual void    Redo();

    virtual rtl::OUString GetComment () const;
};


class TextUndoInsertChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TYPEINFO();
                    TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

    virtual void    Undo();
    virtual void    Redo();

    virtual sal_Bool    Merge( SfxUndoAction *pNextAction );

    virtual rtl::OUString GetComment () const;
};


class TextUndoRemoveChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    String          maText;

public:
                    TYPEINFO();
                    TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const String& rStr );

    virtual void    Undo();
    virtual void    Redo();

    virtual rtl::OUString GetComment () const;
};


class TextUndoSetAttribs: public TextUndo
{
private:
    TextSelection       maSelection;

public:
                        TYPEINFO();
                        TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rESel );
                        ~TextUndoSetAttribs();

    virtual void        Undo();
    virtual void        Redo();

    virtual rtl::OUString GetComment () const;
};

#endif // _TEXTUND2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
