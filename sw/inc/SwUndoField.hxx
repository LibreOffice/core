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
#ifndef INCLUDED_SW_INC_SWUNDOFIELD_HXX
#define INCLUDED_SW_INC_SWUNDOFIELD_HXX

#include "undobj.hxx"

#include <com/sun/star/uno/Any.h>

class SwDoc;
class SwField;
class SwMsgPoolItem;

class SwUndoField : public SwUndo
{
    SwNodeOffset m_nNodeIndex;
    sal_Int32 m_nOffset;

protected:
    SwDoc * m_pDoc;
    SwPosition GetPosition();

public:
    SwUndoField(const SwPosition & rPos );
    virtual ~SwUndoField() override;
};

class SwUndoFieldFromDoc final : public SwUndoField
{
    std::unique_ptr<SwField> m_pOldField, m_pNewField;
    SwMsgPoolItem * m_pHint;
    bool m_bUpdate;

    void DoImpl();

public:
    SwUndoFieldFromDoc(const SwPosition & rPos, const SwField & aOldField,
                       const SwField & aNewField,
                       SwMsgPoolItem * pHint, bool bUpdate);

    virtual ~SwUndoFieldFromDoc() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

class SwUndoFieldFromAPI final : public SwUndoField
{
    css::uno::Any m_aOldVal, m_aNewVal;
    sal_uInt16 m_nWhich;

    void DoImpl();

public:
    SwUndoFieldFromAPI(const SwPosition & rPos,
                       const css::uno::Any & rOldVal,
                       const css::uno::Any & rNewVal,
                       sal_uInt16 nWhich);
    virtual ~SwUndoFieldFromAPI() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

#endif // INCLUDED_SW_INC_SWUNDOFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
