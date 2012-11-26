/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFX_OBJFAC_HXX
#define _SFX_OBJFAC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <sal/types.h>
#include <tools/solar.h>

// SFX_IMPL_MODULE_LIB
#include <vos/module.hxx>
#include <rtl/ustring.hxx>

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
    String          GetFactoryURL() const;  // shortcut for "private:factory/GetShortName()"
    String          GetFactoryName() const { return String::CreateFromAscii( pShortName ); }
    String          GetModuleName() const;
    void            SetDocumentTypeNameResource( const ResId& rId );
    String          GetDocumentTypeName() const;
    SfxFilterContainer *GetFilterContainer( sal_Bool bForceLoad = sal_True) const;

    // Views
    void            RegisterViewFactory(SfxViewFactory &rFactory);
    sal_uInt16          GetViewFactoryCount() const;
    SfxViewFactory& GetViewFactory(sal_uInt16 i = 0) const;

    /// returns the view factory whose GetAPIViewName or GetLegacyViewName delivers the requested logical name
    SfxViewFactory* GetViewFactoryByViewName( const String& i_rViewName ) const;

    // Filter
    const SfxFilter* GetTemplateFilter() const;
    static String   GetStandardTemplate( const String& rServiceName );
    static void     SetStandardTemplate( const String& rServiceName, const String& rTemplateName );
    static void     SetSystemTemplate( const String& rServiceName, const String& rTemplateName );

    void            SetDocumentServiceName( const rtl::OUString& rServiceName );
    const rtl::OUString&    GetDocumentServiceName() const;

    SfxModule*      GetModule() const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void SetModule_Impl( SfxModule* );
    SAL_DLLPRIVATE static void UpdateFilterContainers_Impl();
    SAL_DLLPRIVATE sal_uInt16 GetViewNo_Impl( const sal_uInt16 i_nViewId, const sal_uInt16 i_nFallback ) const;
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

