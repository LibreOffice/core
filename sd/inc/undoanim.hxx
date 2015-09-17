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

#ifndef INCLUDED_SD_INC_UNDOANIM_HXX
#define INCLUDED_SD_INC_UNDOANIM_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <svx/svdundo.hxx>
#include <boost/scoped_ptr.hpp>

#include "sdundo.hxx"

class SdPage;

namespace sd
{

struct UndoAnimationImpl;

class UndoAnimation : public SdrUndoAction
{
public:
    UndoAnimation( SdDrawDocument* pDoc, SdPage* pThePage );
    virtual ~UndoAnimation();

    virtual void Undo() SAL_OVERRIDE;
    virtual void Redo() SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    boost::scoped_ptr<UndoAnimationImpl>  mpImpl;
};

struct UndoAnimationPathImpl;
class UndoAnimationPath : public SdrUndoAction
{
public:
    UndoAnimationPath( SdDrawDocument* pDoc, SdPage* pThePage, const com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    virtual ~UndoAnimationPath();

    virtual void Undo() SAL_OVERRIDE;
    virtual void Redo() SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    boost::scoped_ptr<UndoAnimationPathImpl> mpImpl;
};

struct UndoTransitionImpl;

class UndoTransition : public SdUndoAction
{
public:
    UndoTransition( SdDrawDocument* pDoc, SdPage* pThePage );
    virtual ~UndoTransition();

    virtual void Undo() SAL_OVERRIDE;
    virtual void Redo() SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

private:
    boost::scoped_ptr<UndoTransitionImpl> mpImpl;
};

}

#endif // INCLUDED_SD_INC_UNDOANIM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
