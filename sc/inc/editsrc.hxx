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

#pragma once

#include "address.hxx"
#include <editeng/unoedsrc.hxx>
#include <svl/lstner.hxx>

#include <memory>

class ScEditEngineDefaulter;
class SvxEditEngineForwarder;

class ScDocShell;
class ScCellTextData;
class ScHeaderFooterTextData;
class ScAccessibleTextData;
class SdrObject;

class ScEditSource : public SvxEditSource
{
public:
    virtual ScEditEngineDefaulter* GetEditEngine() = 0;
};

/**
 * ScHeaderFooterTextObj keeps the authoritative copy of
 * ScHeaderFooterTextData that this class holds reference to.  It's a
 * reference instead of a copy to avoid broadcasting changes to the
 * authoritative copy.
 */
class ScHeaderFooterEditSource final : public ScEditSource
{
private:
    ScHeaderFooterTextData& mrTextData;

public:
    ScHeaderFooterEditSource(ScHeaderFooterTextData& rData);
    virtual ~ScHeaderFooterEditSource() override;

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    virtual ScEditEngineDefaulter* GetEditEngine() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;
};

/**
 * Data (incl. EditEngine) for cell EditSource is now shared in
 * ScCellTextData.
 *
 * ScCellEditSource with local copy of ScCellTextData is used by
 * ScCellFieldsObj, ScCellFieldObj.
 */
class ScCellEditSource final : public ScEditSource
{
private:
    std::unique_ptr<ScCellTextData> pCellTextData;

public:
    ScCellEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual ~ScCellEditSource() override;

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    virtual ScEditEngineDefaulter* GetEditEngine() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder* GetTextForwarder() override;

    virtual void UpdateData() override;

    void SetDoUpdateData(bool bValue);
    bool IsDirty() const;
};

class ScAnnotationEditSource final : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    std::unique_ptr<ScEditEngineDefaulter>  pEditEngine;
    std::unique_ptr<SvxEditEngineForwarder> pForwarder;
    bool                    bDataValid;

    SdrObject*                  GetCaptionObj();
public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override ;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

//  EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource final : public SvxEditSource
{
private:
    SvxTextForwarder*   pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual             ~ScSimpleEditSource() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override ;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;

};

class ScAccessibilityEditSource final : public SvxEditSource
{
private:
    ::std::unique_ptr < ScAccessibleTextData > mpAccessibleTextData;

public:
                        ScAccessibilityEditSource( ::std::unique_ptr < ScAccessibleTextData > && pAccessibleCellTextData );
    virtual             ~ScAccessibilityEditSource() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual SvxViewForwarder*   GetViewForwarder() override;
    virtual SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) override;
    virtual void                UpdateData() override;
    virtual SfxBroadcaster&     GetBroadcaster() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
