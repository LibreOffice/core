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

#ifndef INCLUDED_SVX_SVDOUTL_HXX
#define INCLUDED_SVX_SVDOUTL_HXX

#include <editeng/outliner.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdobj.hxx>

class SdrTextObj;
class SdrPage;

class SVX_DLLPUBLIC SdrOutliner : public Outliner
{
    tools::WeakReference<SdrTextObj> mpTextObj;
    const SdrPage* mpVisualizedPage;

public:
    SdrOutliner( SfxItemPool* pItemPool, OutlinerMode nMode );
    virtual ~SdrOutliner() override ;

    void  SetTextObj( const SdrTextObj* pObj );
    void SetTextObjNoInit( const SdrTextObj* pObj );
    const SdrTextObj* GetTextObj() const;

    void setVisualizedPage(const SdrPage* pPage) { if(pPage != mpVisualizedPage) mpVisualizedPage = pPage; }
    const SdrPage* getVisualizedPage() const { return mpVisualizedPage; }

    virtual OUString CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rpTxtColor, boost::optional<Color>& rpFldColor) override;

    bool hasEditViewCallbacks() const;
};

#endif // INCLUDED_SVX_SVDOUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
