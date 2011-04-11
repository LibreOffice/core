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

#ifndef SC_INPUTOPT_HXX
#define SC_INPUTOPT_HXX


#include <unotools/configitem.hxx>
#include <tools/solar.h>

class SvStream;


class ScInputOptions
{
private:
    sal_uInt16      nMoveDir;           // enum ScDirection
    sal_Bool        bMoveSelection;
    sal_Bool        bEnterEdit;
    sal_Bool        bExtendFormat;
    sal_Bool        bRangeFinder;
    sal_Bool        bExpandRefs;
    sal_Bool        bMarkHeader;
    sal_Bool        bUseTabCol;
    sal_Bool        bTextWysiwyg;
    sal_Bool        bReplCellsWarn;

public:
                ScInputOptions();
                ScInputOptions( const ScInputOptions& rCpy );
                ~ScInputOptions();

    void        SetDefaults();

    void        SetMoveDir(sal_uInt16 nNew)         { nMoveDir = nNew;       }
    sal_uInt16      GetMoveDir() const              { return nMoveDir;       }
    void        SetMoveSelection(sal_Bool bSet)     { bMoveSelection = bSet; }
    sal_Bool        GetMoveSelection() const        { return bMoveSelection; }
    void        SetEnterEdit(sal_Bool bSet)         { bEnterEdit = bSet;     }
    sal_Bool        GetEnterEdit() const            { return bEnterEdit;     }
    void        SetExtendFormat(sal_Bool bSet)      { bExtendFormat = bSet;  }
    sal_Bool        GetExtendFormat() const         { return bExtendFormat;  }
    void        SetRangeFinder(sal_Bool bSet)       { bRangeFinder = bSet;   }
    sal_Bool        GetRangeFinder() const          { return bRangeFinder;   }
    void        SetExpandRefs(sal_Bool bSet)        { bExpandRefs = bSet;    }
    sal_Bool        GetExpandRefs() const           { return bExpandRefs;    }
    void        SetMarkHeader(sal_Bool bSet)        { bMarkHeader = bSet;    }
    sal_Bool        GetMarkHeader() const           { return bMarkHeader;    }
    void        SetUseTabCol(sal_Bool bSet)         { bUseTabCol = bSet;     }
    sal_Bool        GetUseTabCol() const            { return bUseTabCol;     }
    void        SetTextWysiwyg(sal_Bool bSet)       { bTextWysiwyg = bSet;   }
    sal_Bool        GetTextWysiwyg() const          { return bTextWysiwyg;   }
    void        SetReplaceCellsWarn(sal_Bool bSet)  { bReplCellsWarn = bSet; }
    sal_Bool        GetReplaceCellsWarn() const     { return bReplCellsWarn; }

    const ScInputOptions&   operator=   ( const ScInputOptions& rOpt );
};


//==================================================================
// CfgItem for input options
//==================================================================

class ScInputCfg : public ScInputOptions,
                  public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScInputCfg();

    void            SetOptions( const ScInputOptions& rNew );
    void            OptionsChanged();   // after direct access to SetOptions base class

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
