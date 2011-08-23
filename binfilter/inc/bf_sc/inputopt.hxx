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


#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SvStream;

namespace binfilter {

class ScInputOptions
{
private:
    USHORT		nMoveDir;			// enum ScDirection
    BOOL		bMoveSelection;
    BOOL		bEnterEdit;
    BOOL		bExtendFormat;
    BOOL		bRangeFinder;
    BOOL		bExpandRefs;
    BOOL		bMarkHeader;
    BOOL		bUseTabCol;
    BOOL		bTextWysiwyg;
    BOOL        bReplCellsWarn;

public:
                ScInputOptions();
                ScInputOptions( const ScInputOptions& rCpy );
                ~ScInputOptions();

    void		SetDefaults();

    void        SetMoveDir(USHORT nNew)         { nMoveDir = nNew;       }
    USHORT      GetMoveDir() const              { return nMoveDir;       }
    void        SetMoveSelection(BOOL bSet)     { bMoveSelection = bSet; }
    BOOL        GetMoveSelection() const        { return bMoveSelection; }
    void        SetEnterEdit(BOOL bSet)         { bEnterEdit = bSet;     }
    BOOL        GetEnterEdit() const            { return bEnterEdit;     }
    void        SetExtendFormat(BOOL bSet)      { bExtendFormat = bSet;  }
    BOOL        GetExtendFormat() const         { return bExtendFormat;  }
    void        SetRangeFinder(BOOL bSet)       { bRangeFinder = bSet;   }
    BOOL        GetRangeFinder() const          { return bRangeFinder;   }
    void        SetExpandRefs(BOOL bSet)        { bExpandRefs = bSet;    }
    BOOL        GetExpandRefs() const           { return bExpandRefs;    }
    void        SetMarkHeader(BOOL bSet)        { bMarkHeader = bSet;    }
    BOOL        GetMarkHeader() const           { return bMarkHeader;    }
    void        SetUseTabCol(BOOL bSet)         { bUseTabCol = bSet;     }
    BOOL        GetUseTabCol() const            { return bUseTabCol;     }
    void        SetTextWysiwyg(BOOL bSet)       { bTextWysiwyg = bSet;   }
    BOOL        GetTextWysiwyg() const          { return bTextWysiwyg;   }
    void        SetReplaceCellsWarn(BOOL bSet)  { bReplCellsWarn = bSet; }
    BOOL        GetReplaceCellsWarn() const     { return bReplCellsWarn; }

    const ScInputOptions&	operator=	( const ScInputOptions& rOpt );
/*N*/ 	friend SvStream& 		operator>>	( SvStream& rStream, ScInputOptions& rOpt );
/*N*/ 	friend SvStream&		operator<<	( SvStream& rStream, const ScInputOptions& rOpt );
};


//==================================================================
// CfgItem fuer Eingabe-Optionen
//==================================================================

class ScInputCfg : public ScInputOptions,
    public ::utl::ConfigItem
{
    ::com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScInputCfg();

    virtual void                Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void                Commit();

    void			SetOptions( const ScInputOptions& rNew ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	void			SetOptions( const ScInputOptions& rNew );
};


} //namespace binfilter
#endif

