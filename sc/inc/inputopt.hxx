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

#ifndef INCLUDED_SC_INC_INPUTOPT_HXX
#define INCLUDED_SC_INC_INPUTOPT_HXX

#include <scdllapi.h>
#include <unotools/configitem.hxx>

class SC_DLLPUBLIC ScInputOptions
{
private:
    sal_uInt16  nMoveDir;           // enum ScDirection
    bool        bMoveSelection;
    bool        bEnterEdit;
    bool        bExtendFormat;
    bool        bRangeFinder;
    bool        bExpandRefs;
    bool        mbSortRefUpdate;
    bool        bMarkHeader;
    bool        bUseTabCol;
    bool        bTextWysiwyg;
    bool        bReplCellsWarn;
    bool        bLegacyCellSelection;

public:
                ScInputOptions();
                ScInputOptions( const ScInputOptions& rCpy );
                ~ScInputOptions();

    void        SetDefaults();

    void        SetMoveDir(sal_uInt16 nNew)         { nMoveDir = nNew;       }
    sal_uInt16      GetMoveDir() const              { return nMoveDir;       }
    void        SetMoveSelection(bool bSet)     { bMoveSelection = bSet; }
    bool        GetMoveSelection() const        { return bMoveSelection; }
    void        SetEnterEdit(bool bSet)         { bEnterEdit = bSet;     }
    bool        GetEnterEdit() const            { return bEnterEdit;     }
    void        SetExtendFormat(bool bSet)      { bExtendFormat = bSet;  }
    bool        GetExtendFormat() const         { return bExtendFormat;  }
    void        SetRangeFinder(bool bSet)       { bRangeFinder = bSet;   }
    bool        GetRangeFinder() const          { return bRangeFinder;   }
    void        SetExpandRefs(bool bSet)        { bExpandRefs = bSet;    }
    bool        GetExpandRefs() const           { return bExpandRefs;    }
    void        SetSortRefUpdate(bool bSet)     { mbSortRefUpdate = bSet; }
    bool        GetSortRefUpdate() const        { return mbSortRefUpdate; }
    void        SetMarkHeader(bool bSet)        { bMarkHeader = bSet;    }
    bool        GetMarkHeader() const           { return bMarkHeader;    }
    void        SetUseTabCol(bool bSet)         { bUseTabCol = bSet;     }
    bool        GetUseTabCol() const            { return bUseTabCol;     }
    void        SetTextWysiwyg(bool bSet)       { bTextWysiwyg = bSet;   }
    bool        GetTextWysiwyg() const          { return bTextWysiwyg;   }
    void        SetReplaceCellsWarn(bool bSet)  { bReplCellsWarn = bSet; }
    bool        GetReplaceCellsWarn() const     { return bReplCellsWarn; }
    void        SetLegacyCellSelection(bool bSet)   { bLegacyCellSelection = bSet; }
    bool        GetLegacyCellSelection() const      { return bLegacyCellSelection; }

    const ScInputOptions&   operator=   ( const ScInputOptions& rOpt );
};

// CfgItem for input options

class ScInputCfg : public ScInputOptions,
                  public utl::ConfigItem
{
    static css::uno::Sequence<OUString> GetPropertyNames();

    virtual void    ImplCommit() override;

public:
            ScInputCfg();

    void            SetOptions( const ScInputOptions& rNew );
    void            OptionsChanged();   // after direct access to SetOptions base class

    virtual void    Notify( const css::uno::Sequence<OUString>& aPropertyNames ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
