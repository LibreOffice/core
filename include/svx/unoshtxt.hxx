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

#ifndef INCLUDED_SVX_UNOSHTXT_HXX
#define INCLUDED_SVX_UNOSHTXT_HXX

#include <memory>
#include <editeng/unoedsrc.hxx>
#include <svx/svxdllapi.h>

class SdrObject;
class SdrView;
class OutputDevice;
class SvxTextEditSourceImpl;
class SdrText;

class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxTextEditSource final : public SvxEditSource, public SvxViewForwarder
{
public:
    SvxTextEditSource( SdrObject* pObj, SdrText* pText );

    /** Since the views don't broadcast their dying, make sure that
        this object gets destroyed if the view becomes invalid

        The window is necessary, since our views can display on multiple windows
     */
    SvxTextEditSource( SdrObject& rObj, SdrText* pText, SdrView& rView, const OutputDevice& rViewWindow );
    SvxTextEditSource(const SvxTextEditSource&) = delete;
    SvxTextEditSource& operator=(const SvxTextEditSource&) = delete;
    virtual ~SvxTextEditSource() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*       GetTextForwarder() override;
    virtual SvxViewForwarder*      GetViewForwarder() override;
    virtual SvxEditViewForwarder*  GetEditViewForwarder( bool bCreate = false ) override;
    virtual void                    UpdateData() override;

    virtual void addRange( SvxUnoTextRangeBase* pNewRange ) override;
    virtual void removeRange( SvxUnoTextRangeBase* pOldRange ) override;
    virtual const SvxUnoTextRangeBaseVec& getRanges() const override;

    virtual SfxBroadcaster&         GetBroadcaster() const override;

    void lock();
    void unlock();

    // the SvxViewForwarder interface
    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point&, const MapMode& ) const override;
    virtual Point       PixelToLogic( const Point&, const MapMode& ) const override;

    void UpdateOutliner();

private:
    SVX_DLLPRIVATE SvxTextEditSource( SvxTextEditSourceImpl* pImpl );

    rtl::Reference<SvxTextEditSourceImpl>  mpImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
