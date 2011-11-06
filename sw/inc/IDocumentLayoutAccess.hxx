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



#ifndef IDOCUMENTLAYOUTACCESS_HXX_INCLUDED
#define IDOCUMENTLAYOUTACCESS_HXX_INCLUDED

#include <swtypes.hxx>

class ViewShell;
class SwRootFrm;
class SwFrmFmt;
class SfxItemSet;
class SwLayouter;
class SwFmtAnchor;

/** Provides access to the layout of a document.
*/
class IDocumentLayoutAccess
{
public:

    /** Returns the layout set at the document.
    */
    virtual const ViewShell* GetCurrentViewShell() const = 0;
    virtual       ViewShell* GetCurrentViewShell() = 0; //swmod 071107//swmod 071225
    virtual const SwRootFrm* GetCurrentLayout() const = 0;
    virtual       SwRootFrm* GetCurrentLayout() = 0;    //swmod 080218
    virtual bool HasLayout() const = 0;

    /** !!!The old layout must be deleted!!!
    */
    virtual void SetCurrentViewShell( ViewShell* pNew ) = 0;    //swmod 071107//swmod 071225

    /**
    */
    virtual SwFrmFmt* MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet ) = 0;

    /**
    */
    virtual SwLayouter* GetLayouter() = 0;
    virtual const SwLayouter* GetLayouter() const = 0;
    virtual void SetLayouter( SwLayouter* pNew ) = 0;

    /**
    */
    virtual void DelLayoutFmt( SwFrmFmt *pFmt ) = 0;

    /**
    */
    virtual SwFrmFmt* CopyLayoutFmt( const SwFrmFmt& rSrc, const SwFmtAnchor& rNewAnchor,
                                     bool bSetTxtFlyAtt, bool bMakeFrms ) = 0;

protected:

    virtual ~IDocumentLayoutAccess() {};
 };

 #endif // IDOCUMENTLAYOUTACCESS_HXX_INCLUDED
