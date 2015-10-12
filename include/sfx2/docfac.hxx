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
#ifndef INCLUDED_SFX2_DOCFAC_HXX
#define INCLUDED_SFX2_DOCFAC_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <tools/rtti.hxx>

// SFX_IMPL_MODULE_LIB
#include <osl/module.hxx>
#include <rtl/ustring.hxx>

#include <sfx2/objsh.hxx>

class SfxMedium;
class SfxFilter;
class SfxViewFactory;
struct SfxObjectFactory_Impl;
class SfxFilterContainer;
class SfxBindings;

typedef void (*SfxVoidFunc)();


class SFX2_DLLPUBLIC SfxObjectFactory
{
private:
    const char*             pShortName;
    SfxObjectFactory_Impl*  pImpl;      // Additional Data
    SfxObjectShellFlags     nFlags;

public:
    SfxObjectFactory( const SvGlobalName &rName, SfxObjectShellFlags nFlags, const char* pShortName );
    ~SfxObjectFactory();

    const SvGlobalName& GetClassId() const;
    SfxObjectShellFlags GetFlags() { return nFlags; }
    const char*     GetShortName() const { return pShortName; }
    OUString        GetFactoryURL() const;  // shortcut for "private:factory/GetShortName()"
    OUString        GetFactoryName() const { return OUString::createFromAscii(pShortName); }
    OUString        GetModuleName() const;
    SfxFilterContainer *GetFilterContainer( bool bForceLoad = true) const;

    // Views
    void            RegisterViewFactory(SfxViewFactory &rFactory);
    sal_uInt16          GetViewFactoryCount() const;
    SfxViewFactory& GetViewFactory(sal_uInt16 i = 0) const;

    /// returns the view factory whose GetAPIViewName or GetLegacyViewName delivers the requested logical name
    SfxViewFactory* GetViewFactoryByViewName( const OUString& i_rViewName ) const;

    // Filter
    const SfxFilter* GetTemplateFilter() const;
    static OUString GetStandardTemplate( const OUString& rServiceName );
    static void     SetStandardTemplate( const OUString& rServiceName, const OUString& rTemplateName );
    static void     SetSystemTemplate( const OUString& rServiceName, const OUString& rTemplateName );

    void            SetDocumentServiceName( const OUString& rServiceName );
    const OUString&    GetDocumentServiceName() const;

    SfxModule*      GetModule() const;

    SAL_DLLPRIVATE void SetModule_Impl( SfxModule* );
    SAL_DLLPRIVATE sal_uInt16 GetViewNo_Impl( const sal_uInt16 i_nViewId, const sal_uInt16 i_nFallback ) const;

private:
    SfxObjectFactory(const SfxObjectFactory&) = delete;
    const SfxObjectFactory& operator=(const SfxObjectFactory &) = delete;
};

#define SFX_DECL_OBJECTFACTORY()                                            \
public:                                                                     \
    static SfxObjectFactory&    Factory();                                  \
    virtual SfxObjectFactory&   GetFactory() const override { return Factory(); }

#define SFX_IMPL_OBJECTFACTORY(ClassName,GlobName,Flags,ShortName)          \
    SfxObjectFactory& ClassName::Factory()                                  \
    {                                                                       \
        static SfxObjectFactory aObjectFactory(GlobName, Flags, ShortName); \
        return aObjectFactory;                                              \
    }
#endif // INCLUDED_SFX2_DOCFAC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
