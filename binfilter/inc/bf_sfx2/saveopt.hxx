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
#ifndef _SFX_SAVEOPT_HXX
#define _SFX_SAVEOPT_HXX


#include <vcl/wintypes.hxx>
#include <vcl/field.hxx>
#include <tools/string.hxx>
#include <bf_svtools/poolitem.hxx>
// -----------------------------------------------------------------------
#include "bf_sfx2/cfgitem.hxx"
namespace binfilter {

struct SfxOptions_Impl;

// -----------------------------------------------------------------------
class SfxOptions : public SfxConfigItem
{
#if _SOLAR__PRIVATE
    struct SfxOptions_Impl
    {
        SfxMapUnit	eUserMetric;
        BOOL		bSaveDocWins:1,
                    bSaveDocView:1,
                    bSaveRelINet:1,
                    bSaveRelFSys:1,
                    bAutoHelpAgent:1,
                    bSaveGrfCompr:1,
                    bSaveGrfOrig:1,
                    bWelcomeScreen:1;
        String		aRet;
    };
#endif

    SfxOptions_Impl*pImp;
    String			aDocMgrConfig;
    USHORT			nAutoSaveTime;
    USHORT			nUndoActions;
    FieldUnit  		eMetric;
    BOOL			bBackup:1,
                    bAutoSave:1,
                    bDocInfSave:1,
                    bIndepGrfFmt:1,
                    bAutoSavePrompt:1,
                    bSaveWorkingSet:1,
                    bHelpBalloons:1,
                    bHelpTips:1;

private:
    void			SetMetric_Impl( FieldUnit e, BOOL bLoading );

protected:
    virtual int 	Load(SvStream&);
    virtual BOOL	Store(SvStream&);
    virtual	void	UseDefault();

public:
                    SfxOptions();
                    ~SfxOptions();

    void			SetBackup( BOOL b ) 		{ bBackup = b;SetDefault(FALSE);}
    BOOL			IsBackup() const			{ return bBackup;		  }
    void			SetAutoSave( BOOL b )		{ bAutoSave = b;	SetDefault(FALSE);	  }
    BOOL			IsAutoSave() const			{ return bAutoSave; 	  }
    void			SetAutoSavePrompt( BOOL b ) { bAutoSavePrompt = b;SetDefault(FALSE);  }
    BOOL			IsAutoSavePrompt() const	{ return bAutoSavePrompt; }
    void			SetAutoSaveTime( USHORT n ) { nAutoSaveTime = n;SetDefault(FALSE);  }
    USHORT			GetAutoSaveTime() const 	{ return nAutoSaveTime;   }
    void			SetDocInfoSave(BOOL b)		{ bDocInfSave = b;	 SetDefault(FALSE); }
    BOOL			IsDocInfoSave()	const		{ return bDocInfSave;     }

    void			SetSaveOriginalGraphics(BOOL b);
    BOOL			IsSaveOriginalGraphics() const;
    void			SetSaveGraphicsCompressed(BOOL b);
    BOOL			IsSaveGraphicsCompressed() const;

    void			SetIndepGrfFmt(BOOL b)		{ bIndepGrfFmt = b;	  SetDefault(FALSE);}
    BOOL			IsIndepGrfFmt()	const		{ return bIndepGrfFmt;    }
    void			SetSaveWorkingSet( BOOL b )	{ bSaveWorkingSet = b;SetDefault(FALSE);}
    BOOL			IsSaveWorkingSet() const	{ return bSaveWorkingSet;		  }
    void			SetSaveDocWins( BOOL b );
    BOOL			IsSaveDocWins() const;
    void			SetSaveDocView( BOOL b );
    BOOL			IsSaveDocView() const;
    void			SetAutoHelpAgent( BOOL b );
    BOOL			IsAutoHelpAgent() const;
    void			SetWelcomeScreen( BOOL b );
    BOOL			IsWelcomeScreen() const;

    void			SetMetric(FieldUnit e);
    FieldUnit		GetMetric() const			{ return eMetric; }
    SfxMapUnit		GetUserMetric() const;

    void			SetUndoCount(USHORT n)		{ nUndoActions = n; SetDefault(FALSE);}
    USHORT 			GetUndoCount() const		{ return nUndoActions; }

    void			SetHelpBalloons( BOOL b )	{ bHelpBalloons = b; SetDefault(FALSE); }
    BOOL			IsHelpBalloons() const		{ return bHelpBalloons; }
    void			SetHelpTips( BOOL b )		{ bHelpTips = b; SetDefault(FALSE); }
    BOOL			IsHelpTips() const			{ return bHelpTips; }

    const String&	GetDocumentManagerConfig() const
                    { return aDocMgrConfig; }
    void            SetDocumentManagerConfig( const String &rConfig )
                    { aDocMgrConfig = rConfig; SetDefault(FALSE); }

    void			SetSaveRelINet( BOOL b );
    BOOL			IsSaveRelINet() const;
    void			SetSaveRelFSys( BOOL b );
    BOOL			IsSaveRelFSys() const;
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
