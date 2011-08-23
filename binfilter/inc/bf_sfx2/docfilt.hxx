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
#ifndef _SFX_DOCFILT_HACK_HXX
#define _SFX_DOCFILT_HACK_HXX

#ifndef _COM_SUN_STAR_PLUGIN_PLUGINDESCRIPTION_HPP_
#include <com/sun/star/plugin/PluginDescription.hpp>
#endif

#ifndef _WLDCRD_HXX //autogen
#include <tools/wldcrd.hxx>
#endif
namespace binfilter {

#define SFX_FILTER_IMPORT            0x00000001L
#define SFX_FILTER_EXPORT            0x00000002L
#define SFX_FILTER_TEMPLATE          0x00000004L
#define SFX_FILTER_INTERNAL          0x00000008L
#define SFX_FILTER_TEMPLATEPATH      0x00000010L
#define SFX_FILTER_OWN 		         0x00000020L
#define SFX_FILTER_ALIEN	         0x00000040L
#define SFX_FILTER_USESOPTIONS       0x00000080L
#define SFX_FILTER_NOTINFILEDLG      0x00001000L
#define SFX_FILTER_NOTINCHOOSER      0x00002000L

#define SFX_FILTER_DEFAULT           0x00000100L
#define SFX_FILTER_EXECUTABLE        0x00000200L
#define SFX_FILTER_SUPPORTSSELECTION 0x00000400L
#define SFX_FILTER_MAPTOAPPPLUG      0x00000800L
#define SFX_FILTER_ASYNC             0x00004000L
// Legt Objekt nur an, kein Laden
#define SFX_FILTER_CREATOR           0x00008000L
#define SFX_FILTER_OPENREADONLY      0x00010000L
#define SFX_FILTER_MUSTINSTALL		 0x00020000L
#define SFX_FILTER_CONSULTSERVICE	 0x00040000L

#define SFX_FILTER_STARONEFILTER	 0x00080000L
#define SFX_FILTER_PACKED	 		 0x00100000L
#define SFX_FILTER_SILENTEXPORT      0x00200000L

#define SFX_FILTER_BROWSERPREFERED   0x00400000L
#define SFX_FILTER_PREFERED          0x10000000L

#define SFX_FILTER_VERSION_NONE      0
#define SFX_FILTER_NOTINSTALLED		 SFX_FILTER_MUSTINSTALL | SFX_FILTER_CONSULTSERVICE

#include <bf_sfx2/sfxdefs.hxx>

//========================================================================
class SfxFilterContainer;

class SfxFilter
{
friend class SfxFilterContainer;

    String			aName;
    WildCard		aWildCard;
    ULONG			lFormat;
    String			aMacType;
    String			aTypeName;
    String			aUserData;
    SfxFilterFlags	nFormatType;
    USHORT          nDocIcon;
    const SfxFilterContainer* pContainer;
    String          aMimeType;
    String          aFilterName;
    String          aPattern;
    ULONG           nVersion;
    BOOL            bPlugDataSearched;
    ::com::sun::star::plugin::PluginDescription* pPlugData;
    String          aUIName;
    String          aDefaultTemplate;

public:
                    SfxFilter( const String &rName,
                               const String &rWildCard,
                               SfxFilterFlags nFormatType,
                               ULONG lFormat,		// Clipboardformat
                               const String &rMacType,	// Dateityp (MAC)
                               const String &rTypeName, // Dateityp als String (OS/2)
                               USHORT nDocIcon,         // Dokumenticon OS/2
                               const String &rMimeType,
                               const SfxFilterContainer* pFactoryP,
                               const String &rUserData );// Daten der Anwendung
                    ~SfxFilter();

    const SfxFilterContainer* GetFilterContainer( ) const { return pContainer; }

    FASTBOOL		IsAllowedAsTemplate() const
                    { return nFormatType & SFX_FILTER_TEMPLATE; }
    FASTBOOL		IsOwnFormat() const
                    { return nFormatType & SFX_FILTER_OWN; }
    FASTBOOL		IsOwnTemplateFormat() const
                    { return nFormatType & SFX_FILTER_TEMPLATEPATH; }
    FASTBOOL		IsAlienFormat() const
                    { return nFormatType & SFX_FILTER_ALIEN; }
    FASTBOOL		CanImport() const
                    { return nFormatType & SFX_FILTER_IMPORT; }
    FASTBOOL		CanExport() const
                    { return nFormatType & SFX_FILTER_EXPORT; }
    FASTBOOL		IsInternal() const
                    { return nFormatType & SFX_FILTER_INTERNAL; }
    SfxFilterFlags  GetFilterFlags() const	{ return nFormatType; }
    const String&   GetFilterName() const { return aFilterName; }
    const String&   GetMimeType() const { return aMimeType; }
    const String&   GetName() const { return  aName; }
    String          GetFilterNameWithPrefix() const;
    const WildCard& GetWildcard() const { return aWildCard; }
    const String&	GetMacType() const { return aMacType; }
    const String&	GetRealTypeName() const { return aTypeName; }
    ULONG			GetFormat() const { return lFormat; }
    const String&	GetTypeName() const;
    const String&   GetUIName() const { return aUIName; }
    USHORT          GetDocIconId() const { return nDocIcon; }
    const String&	GetUserData() const { return aUserData; }
    const String&   GetDefaultTemplate() const { return aDefaultTemplate; }
    void            SetDefaultTemplate( const String& rStr )
                        { aDefaultTemplate = rStr; }
    BOOL			UsesStorage() const { return GetFormat() != 0; }
    void            SetURLPattern( const String& rStr )
                    {
                        aPattern = rStr;
                        aPattern.ToLowerAscii();
                    }
    String          GetURLPattern() const { return aPattern; }
    void            SetUIName( const String& rName ) { aUIName = rName; }
    void            SetVersion( ULONG nVersionP ) { nVersion = nVersionP; }
    ULONG           GetVersion() const { return nVersion; }
    const ::com::sun::star::plugin::PluginDescription* GetPlugData() const {
        return ((SfxFilter*)this)->GetPlugData();	}

#if _SOLAR__PRIVATE
    void            InitMembers_Impl();
#endif
};

}//end of namespace binfilter
#endif

