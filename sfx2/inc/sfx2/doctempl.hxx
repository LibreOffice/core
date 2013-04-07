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
#ifndef _SFXDOCTEMPL_HXX
#define _SFXDOCTEMPL_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/ref.hxx>
#include <tools/string.hxx>

// CLASS -----------------------------------------------------------------

class SfxObjectShell;

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

class SfxDocTemplate_Impl;

#ifndef SFX_DECL_DOCTEMPLATES_DEFINED
#define SFX_DECL_DOCTEMPLATES_DEFINED
SV_DECL_REF(SfxDocTemplate_Impl)
#endif

// class SfxDocumentTemplates --------------------------------------------

class SFX2_DLLPUBLIC SfxDocumentTemplates
{
private:
    SfxDocTemplate_ImplRef  pImp;

    SAL_DLLPRIVATE sal_Bool CopyOrMove( sal_uInt16 nTargetRegion, sal_uInt16 nTargetIdx,
                                    sal_uInt16 nSourceRegion, sal_uInt16 nSourceIdx, sal_Bool bMove );
public:
                        SfxDocumentTemplates();
                        SfxDocumentTemplates(const SfxDocumentTemplates &);
                        ~SfxDocumentTemplates();

    sal_Bool                IsConstructed() { return pImp != NULL; }
    void                Construct();

    const SfxDocumentTemplates &operator=(const SfxDocumentTemplates &);

    sal_Bool                           Rescan( );
    void                ReInitFromComponent();

    sal_Bool                IsRegionLoaded( sal_uInt16 nIdx ) const;
    sal_uInt16              GetRegionCount() const;
    const String&       GetRegionName(sal_uInt16 nIdx) const;                   //dv!
    String              GetFullRegionName(sal_uInt16 nIdx) const;

    sal_uInt16              GetCount(sal_uInt16 nRegion) const;
    const String&       GetName(sal_uInt16 nRegion, sal_uInt16 nIdx) const;         //dv!
    String              GetFileName(sal_uInt16 nRegion, sal_uInt16 nIdx) const;
    String              GetPath(sal_uInt16 nRegion, sal_uInt16 nIdx) const;

    // Allows to retrieve the target template URL from the UCB
    ::rtl::OUString     GetTemplateTargetURLFromComponent( const ::rtl::OUString& aGroupName,
                                                         const ::rtl::OUString& aTitle );

    // Convert a resource string - a template name - to its localised pair
    // if it exists in templatelocnames.src
    static ::rtl::OUString ConvertResourceString(int nSourceResIds,
                            int nDestResIds,
                            int nCount,
                            const ::rtl::OUString& rString);

    sal_Bool            Copy(sal_uInt16 nTargetRegion,
                         sal_uInt16 nTargetIdx,
                         sal_uInt16 nSourceRegion,
                         sal_uInt16 nSourceIdx);
    sal_Bool            Move(sal_uInt16 nTargetRegion,
                         sal_uInt16 nTargetIdx,
                         sal_uInt16 nSourceRegion,
                         sal_uInt16 nSourceIdx);
    sal_Bool            Delete(sal_uInt16 nRegion, sal_uInt16 nIdx);
    sal_Bool            InsertDir(const String &rText, sal_uInt16 nRegion);
    sal_Bool            SetName(const String &rName, sal_uInt16 nRegion, sal_uInt16 nIdx);

    sal_Bool InsertTemplate (sal_uInt16 nSourceRegion, sal_uInt16 nIdx, const OUString &rName, const OUString &rPath);

    /** Change the name of an entry or a directory

        \param rName
            The new name to set
        \param nRegion
            The id of the region to rename or containing the template to rename
        \param nIdx
            The id of the template to rename or USHRT_MAX to rename the region.

        \return
            sal_True if the action could be performed, sal_False otherwise

    */
    sal_Bool            SetName(const rtl::OUString &rName, sal_uInt16 nRegion, sal_uInt16 nIdx);

    sal_Bool            CopyTo(sal_uInt16 nRegion, sal_uInt16 nIdx, const String &rName) const;
    sal_Bool            CopyFrom(sal_uInt16 nRegion, sal_uInt16 nIdx, String &rName);

    SfxObjectShellRef CreateObjectShell(sal_uInt16 nRegion, sal_uInt16 nIdx);
    sal_Bool            DeleteObjectShell(sal_uInt16, sal_uInt16);

    sal_Bool            GetFull( const String& rRegion, const String& rName, String& rPath );
    sal_Bool            GetLogicNames( const String& rPath, String& rRegion, String& rName ) const;

    /** updates the configuration where the document templates structure is stored.

        <p>The info about the document templates (which files, which groups etc.) is stored in the
        configuration. This means that just by copying files into OOo's template directories, this
        change is not reflected in the SfxDocumentTemplates - 'cause the configuration is not synchronous with
        the file system. This can be enforced with this method.</p>

    @param _bSmart
        The update of the configuration is rather expensive - nothing you want to do regulary if you don't really
        need it. So you have the possibility to do a smart update - it first checks if the update if necessary.
        In case the update is needed, the additional check made it somewhat more expensive. In case it's not
        necessary (which should be the usual case), the check alone is (much) less expensive than the real update.
        <br/>
        So set <arg>_bSmart</arg> to <TRUE/> to do a check for necessity first.
    */
    void            Update( sal_Bool _bSmart = sal_True );

    // allows to detect whether it is allowed to delete ( at least partially )
    // a group or a template, or to edit a template
    sal_Bool        HasUserContents( sal_uInt16 nRegion, sal_uInt16 nIdx ) const;
};

#endif // #ifndef _SFXDOCTEMPL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
