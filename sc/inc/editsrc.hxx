/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    virtual ScEditEngineDefaulter* GetEditEngine();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();
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
    virtual ScEditEngineDefaulter* GetEditEngine();

    virtual SvxEditSource* Clone() const;
    virtual SvxTextForwarder* GetTextForwarder();

    virtual void UpdateData();

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
    sal_Bool                    bDataValid;

    SdrObject*                  GetCaptionObj();
public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


//  EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource : public SvxEditSource
{
private:
    SvxTextForwarder*   pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual             ~ScSimpleEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

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

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*   GetViewForwarder();
    virtual SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = false );
    virtual void                UpdateData();
    virtual SfxBroadcaster&     GetBroadcaster() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
