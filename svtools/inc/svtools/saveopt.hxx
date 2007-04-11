/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saveopt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:33:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#define INCLUDED_SVTOOLS_SAVEOPT_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

struct SvtLoadSaveOptions_Impl;
class SVL_DLLPUBLIC SvtSaveOptions: public svt::detail::Options
{
    SvtLoadSaveOptions_Impl*    pImp;

public:

    enum SaveGraphicsMode
    {
        SaveGraphicsNormal,
        SaveGraphicsOriginal,
        SaveGraphicsCompressed
    };

    enum EOption
    {
        E_AUTOSAVETIME,
        E_SAVEGRAPHICS,
        E_USEUSERDATA,
        E_BACKUP,
        E_AUTOSAVE,
        E_AUTOSAVEPROMPT,
        E_DOCINFSAVE,
        E_SAVEWORKINGSET,
        E_SAVEDOCWINS,
        E_SAVEDOCVIEW,
        E_SAVERELINET,
        E_SAVERELFSYS,
        E_SAVEUNPACKED,
        E_DOPRETTYPRINTING,
        E_WARNALIENFORMAT
    };
                            SvtSaveOptions();
                            virtual ~SvtSaveOptions();

    void                    SetAutoSaveTime( sal_Int32 n );
    sal_Int32               GetAutoSaveTime() const;

    void                    SetUseUserData( sal_Bool b );
    sal_Bool                IsUseUserData() const;

    void                    SetBackup( sal_Bool b );
    sal_Bool                IsBackup() const;

    void                    SetAutoSave( sal_Bool b );
    sal_Bool                IsAutoSave() const;

    void                    SetAutoSavePrompt( sal_Bool b );
    sal_Bool                IsAutoSavePrompt() const;

    void                    SetDocInfoSave(sal_Bool b);
    sal_Bool                IsDocInfoSave() const;

    void                    SetSaveGraphicsMode( SaveGraphicsMode eMode );
    SaveGraphicsMode        GetSaveGraphicsMode() const;

    void                    SetSaveWorkingSet( sal_Bool b );
    sal_Bool                IsSaveWorkingSet() const;

    void                    SetSaveDocWins( sal_Bool b );
    sal_Bool                IsSaveDocWins() const;

    void                    SetSaveDocView( sal_Bool b );
    sal_Bool                IsSaveDocView() const;

    void                    SetSaveRelINet( sal_Bool b );
    sal_Bool                IsSaveRelINet() const;

    void                    SetSaveRelFSys( sal_Bool b );
    sal_Bool                IsSaveRelFSys() const;

    void                    SetSaveUnpacked( sal_Bool b );
    sal_Bool                IsSaveUnpacked() const;

    void                    SetLoadUserSettings(sal_Bool b);
    sal_Bool                IsLoadUserSettings() const;

    void                    SetPrettyPrinting( sal_Bool _bEnable );
    sal_Bool                IsPrettyPrinting( ) const;

    void                    SetWarnAlienFormat( sal_Bool _bEnable );
    sal_Bool                IsWarnAlienFormat( ) const;

    sal_Bool                IsReadOnly( EOption eOption ) const;
};

#endif

