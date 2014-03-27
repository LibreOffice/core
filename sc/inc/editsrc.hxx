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

#ifndef SC_EDITSRC_HXX
#define SC_EDITSRC_HXX

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
class ScHeaderFooterEditSource : public ScEditSource
{
private:
    ScHeaderFooterTextData& mrTextData;

public:
    ScHeaderFooterEditSource(ScHeaderFooterTextData& rData);
    virtual ~ScHeaderFooterEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    virtual ScEditEngineDefaulter* GetEditEngine() SAL_OVERRIDE;

    virtual SvxEditSource*      Clone() const SAL_OVERRIDE;
    virtual SvxTextForwarder*   GetTextForwarder() SAL_OVERRIDE;
    virtual void                UpdateData() SAL_OVERRIDE;
};

/**
 * Data (incl. EditEngine) for cell EditSource is now shared in
 * ScCellTextData.
 *
 * ScCellEditSource with local copy of ScCellTextData is used by
 * ScCellFieldsObj, ScCellFieldObj.
 */
class ScCellEditSource : public ScEditSource
{
private:
    ScCellTextData* pCellTextData;

public:
    ScCellEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual ~ScCellEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    virtual ScEditEngineDefaulter* GetEditEngine() SAL_OVERRIDE;

    virtual SvxEditSource* Clone() const SAL_OVERRIDE;
    virtual SvxTextForwarder* GetTextForwarder() SAL_OVERRIDE;

    virtual void UpdateData() SAL_OVERRIDE;

    void SetDoUpdateData(bool bValue);
    bool IsDirty() const;
};

class ScAnnotationEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    bool                    bDataValid;

    SdrObject*                  GetCaptionObj();
public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource();

    virtual SvxEditSource*      Clone() const SAL_OVERRIDE ;
    virtual SvxTextForwarder*   GetTextForwarder() SAL_OVERRIDE;
    virtual void                UpdateData() SAL_OVERRIDE;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
};


//  EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource : public SvxEditSource
{
private:
    SvxTextForwarder*   pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual             ~ScSimpleEditSource();

    virtual SvxEditSource*      Clone() const SAL_OVERRIDE ;
    virtual SvxTextForwarder*   GetTextForwarder() SAL_OVERRIDE;
    virtual void                UpdateData() SAL_OVERRIDE;

};

class ScAccessibilityEditSource : public SvxEditSource
{
private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr < ScAccessibleTextData > mpAccessibleTextData;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
                        ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual             ~ScAccessibilityEditSource();

    virtual SvxEditSource*      Clone() const SAL_OVERRIDE;
    virtual SvxTextForwarder*   GetTextForwarder() SAL_OVERRIDE;
    virtual SvxViewForwarder*   GetViewForwarder() SAL_OVERRIDE;
    virtual SvxEditViewForwarder*   GetEditViewForwarder( bool bCreate = false ) SAL_OVERRIDE;
    virtual void                UpdateData() SAL_OVERRIDE;
    virtual SfxBroadcaster&     GetBroadcaster() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
