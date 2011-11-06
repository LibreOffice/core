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



#ifndef _SD_UNDO_ANIM_HXX
#define _SD_UNDO_ANIM_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <svx/svdundo.hxx>

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

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    UndoAnimationImpl*  mpImpl;
};

struct UndoAnimationPathImpl;
class UndoAnimationPath : public SdrUndoAction
{
public:
    UndoAnimationPath( SdDrawDocument* pDoc, SdPage* pThePage, const com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );
    virtual ~UndoAnimationPath();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    UndoAnimationPathImpl* mpImpl;
};

struct UndoTransitionImpl;

class UndoTransition : public SdUndoAction
{
public:
    UndoTransition( SdDrawDocument* pDoc, SdPage* pThePage );
    virtual ~UndoTransition();

    virtual void Undo();
    virtual void Redo();

    virtual String GetComment() const;

private:
    UndoTransitionImpl* mpImpl;
};

}

#endif      // _SD_UNDO_ANIM_HXX

