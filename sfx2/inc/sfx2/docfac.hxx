/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docfac.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:18:34 $
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
#ifndef _SFX_OBJFAC_HXX
#define _SFX_OBJFAC_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif

// SFX_IMPL_MODULE_LIB
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#include <sfx2/objsh.hxx>
#include <sfx2/sfxdefs.hxx>

class SfxObjectFactoryArr_Impl;
class SfxMedium;
class SfxFilter;
class SfxViewFactory;
struct SfxObjectFactory_Impl;
class SfxFilterContainer;
class SfxBindings;

//====================================================================

typedef void (*SfxVoidFunc)();

#define SDT_SDT_DOCFACTPRIO     10      // Explorer
#define SDT_SW_DOCFACTPRIO      20      // Text-Dokument
#define SDT_SWW_DOCFACTPRIO     21      // Web-Dokument
#define SDT_SC_DOCFACTPRIO      30      // Tabellen-Dokument
#define SDT_SI_DOCFACTPRIO      40      // Impress-Dokument
#define SDT_SD_DOCFACTPRIO      41      // Draw-Dokument
#define SDT_SCH_DOCFACTPRIO     50      // Chart-Dokument
#define SDT_SMA_DOCFACTPRIO     60      // Math-Dokument
#define SDT_SIM_DOCFACTPRIO     70      // Image-Dokument
#define SDT_FRM_DOCFACTPRIO    100      // Rahmen-Dokument
#define SDT_MSG_DOCFACTPRIO    110      // Nachrichten-Dokument
#define SDT_SDB_DOCFACTPRIO    200      // Datenbank-Dokument

//====================================================================

class SFX2_DLLPUBLIC SfxObjectFactory
{
private:
    const char*             pShortName;     // Objekt-Kurzname
    SfxObjectFactory_Impl*  pImpl;          // Zusatzdaten
    SfxObjectShellFlags     nFlags;
    SAL_DLLPRIVATE void Construct();

public:
    SfxObjectFactory( const SvGlobalName &rName, SfxObjectShellFlags nFlags, const char* pShortName );
    ~SfxObjectFactory();

    const SvGlobalName& GetClassId() const;
    SfxObjectShellFlags GetFlags() { return nFlags; }
    const char*     GetShortName() const { return pShortName; }
    String          GetFactoryName() const { return String::CreateFromAscii( pShortName ); }
    String          GetModuleName() const;
    void            SetDocumentTypeNameResource( const ResId& rId );
    String          GetDocumentTypeName() const;
    SfxFilterContainer *GetFilterContainer( BOOL bForceLoad = TRUE) const;

    // Views
    void            RegisterViewFactory(SfxViewFactory &rFactory);
    USHORT          GetViewFactoryCount() const;
    SfxViewFactory& GetViewFactory(USHORT i = 0) const;

    // Filter
    const SfxFilter* GetTemplateFilter() const;
    static String   GetStandardTemplate( const String& rServiceName );
    static void     SetStandardTemplate( const String& rServiceName, const String& rTemplateName );

    void            SetDocumentServiceName( const rtl::OUString& rServiceName );
    const rtl::OUString&    GetDocumentServiceName() const;

    SfxModule*      GetModule() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void SetModule_Impl( SfxModule* );
    SAL_DLLPRIVATE static void UpdateFilterContainers_Impl();
//#endif

private:
    // Kopieren verboten
    SAL_DLLPRIVATE SfxObjectFactory(const SfxObjectFactory&);
    SAL_DLLPRIVATE const SfxObjectFactory& operator=(const SfxObjectFactory &);
};

//=========================================================================

#define SFX_DECL_OBJECTFACTORY()                                            \
private:                                                                    \
    static SfxObjectFactory*    pObjectFactory;                             \
public:                                                                     \
    static SfxObjectFactory&    Factory();                                  \
    virtual SfxObjectFactory&   GetFactory() const { return Factory(); }

#define SFX_IMPL_OBJECTFACTORY(ClassName,GlobName,Flags,ShortName)          \
    SfxObjectFactory*           ClassName::pObjectFactory = 0;              \
    SfxObjectFactory&           ClassName::Factory()                          \
                                { if (!pObjectFactory)                      \
                                    pObjectFactory =                        \
                                        new SfxObjectFactory( GlobName, Flags, ShortName ); \
                                  return *pObjectFactory;                    \
                                }
#endif // #ifndef _SFX_OBJFAC_HXX

