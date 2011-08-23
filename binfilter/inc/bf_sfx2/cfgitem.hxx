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
#ifndef	_SFXCFGITEM_HXX
#define	_SFXCFGITEM_HXX
class SotStorage;
class String;
namespace binfilter {

class SfxConfigManager;
class SfxShell;

#define SFX_ITEMTYPE_ACCEL				 1
#define SFX_ITEMTYPE_MENU				 2
#define SFX_ITEMTYPE_TOOLBOX			 3
#define SFX_ITEMTYPE_STATBAR			 4
#define SFX_ITEMTYPE_DIALOGCONFIG		 5
#define SFX_ITEMTYPE_IMAGELIST			 6
#define SFX_ITEMTYPE_OPTIONS			 7
#define SFX_ITEMTYPE_MAILCONFIG          8
#define SFX_ITEMTYPE_TOOLBOXCONFIG		 9
#define SFX_ITEMTYPE_APPEVENTCONFIG 	10
#define SFX_ITEMTYPE_DOCEVENTCONFIG 	11
#define SFX_ITEMTYPE_PLUGINSTATUSBAR	12
#define SFX_ITEMTYPE_HTMLCONFIG			13
#define SFX_ITEMTYPE_AUTOCORRCONFIG		14
#define SFX_ITEMTYPE_SPELLCONFIG		15
#define SFX_ITEMTYPE_MISCCONFIG			16
#define SFX_ITEMTYPE_APPEARCONFIG		17
#define SFX_ITEMTYPE_SVXSEARCHPARAMS	18
#define SFX_ITEMTYPE_FILTERCONFIG		19

#define SFX_ITEMTYPE_INTERFACE_START    20
#define SFX_ITEMTYPE_INTERFACE_END     300
}//end of namespace binfilter  
#include <tools/solar.h>
namespace binfilter {
class SfxConfigItem
{
    SfxConfigManager*	m_pCfgMgr;
    const USHORT		m_nType;
    BOOL				m_bDefault : 1;
    BOOL				m_bModified : 1;
    BOOL                m_bInitialized : 1;

public:

    static String       GetStreamName( USHORT nType );
                        enum {
                            ERR_OK,
                            WARNING_VERSION,
                            ERR_READ
                        };

                        SfxConfigItem( USHORT nType, SfxConfigManager* pCfgMgr );
                        ~SfxConfigItem();

            BOOL		Initialize();
            BOOL		StoreConfig();
            void		Connect( SfxConfigManager* pMgr );

    virtual int         Load(SotStorage& rStorage) = 0;
    virtual BOOL        Store(SotStorage& rStorage) = 0;
    virtual String      GetStreamName() const = 0;
    virtual void        UseDefault() = 0;
    virtual BOOL        ReInitialize();

            void		SetDefault( BOOL bOn = TRUE );
            void		SetModified( BOOL bSet = TRUE );

            BOOL		IsDefault() const
                        { return m_bDefault; }
            BOOL		IsModified()
                        { return m_bModified; }
            USHORT		GetType() const
                        { return m_nType; }
            void		ReleaseConfigManager()
                        { m_pCfgMgr = 0; }
    SfxConfigManager*	GetConfigManager() const
                        { return m_pCfgMgr; }
};

}//end of namespace binfilter
#endif

