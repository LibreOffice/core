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
#ifndef INCLUDED_VCL_SOURCE_EDIT_TEXTUND2_HXX
#define INCLUDED_VCL_SOURCE_EDIT_TEXTUND2_HXX

#include "textundo.hxx"
#include <vcl/textdata.hxx>

class TextUndoDelPara : public TextUndo
{
private:
    bool            mbDelObject;
    sal_uInt32      mnPara;
    TextNode*       mpNode; // points at the valid not-destroyed object

public:
                    TextUndoDelPara( TextEngine* pTextEngine, TextNode* pNode, sal_uInt32 nPara );
                    virtual ~TextUndoDelPara();

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual OUString GetComment () const override;
};

class TextUndoConnectParas : public TextUndo
{
private:
    sal_uInt32          mnPara;
    sal_Int32           mnSepPos;

public:
                    TextUndoConnectParas( TextEngine* pTextEngine, sal_uInt32 nPara, sal_Int32 nSepPos );
                    virtual ~TextUndoConnectParas();

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual OUString GetComment () const override;
};

class TextUndoSplitPara : public TextUndo
{
private:
    sal_uInt32          mnPara;
    sal_Int32           mnSepPos;

public:
                    TextUndoSplitPara( TextEngine* pTextEngine, sal_uInt32 nPara, sal_Int32 nSepPos );
                    virtual ~TextUndoSplitPara();

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual OUString GetComment () const override;
};

class TextUndoInsertChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    OUString        maText;

public:
                    TextUndoInsertChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const OUString& rStr );

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual bool    Merge( SfxUndoAction *pNextAction ) override;

    virtual OUString GetComment () const override;
};

class TextUndoRemoveChars : public TextUndo
{
private:
    TextPaM         maTextPaM;
    OUString        maText;

public:
                    TextUndoRemoveChars( TextEngine* pTextEngine, const TextPaM& rTextPaM, const OUString& rStr );

    virtual void    Undo() override;
    virtual void    Redo() override;

    virtual OUString GetComment () const override;
};

class TextUndoSetAttribs: public TextUndo
{
public:
                        TextUndoSetAttribs( TextEngine* pTextEngine, const TextSelection& rESel );
                        virtual ~TextUndoSetAttribs();

    virtual void        Undo() override;
    virtual void        Redo() override;

    virtual OUString GetComment () const override;
};

#endif // INCLUDED_VCL_SOURCE_EDIT_TEXTUND2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
